//========================================================================================================================================================================================================================
// DEMO 077: Subsurface scattering
//========================================================================================================================================================================================================================
#include <random>
#include <cstdlib>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "fbo.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
          camera(4.0f, 0.5f, glm::lookAt(glm::vec3(4.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
    {
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================

    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

struct depth_fbo_t
{
    GLuint fbo_id;
    GLuint tex_id;
    GLuint tex_res;

    depth_fbo_t(GLuint tex_unit, GLuint tex_res) : tex_res(tex_res)
    {
        glGenFramebuffers(1, &fbo_id);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

        glActiveTexture(GL_TEXTURE0 + tex_unit);
        glGenTextures(1, &tex_id);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, tex_res, tex_res);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_id, 0);
        check_status();
    }

    void viewport()
    {
        glViewport(0, 0, tex_res, tex_res);
    }

    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
    }
};

struct simple_model_t
{
    uint32_t V;
    uint32_t F;

    std::vector<glm::vec3> positions;
    std::vector<glm::uvec3> faces;

    glm::vec3 center;
    float radius;

    GLuint vao_id, vbo_id, ibo_id;

    simple_model_t(const char* file_name)
    {
        debug_msg("Loading %s model", file_name);

        FILE* file = fopen(file_name, "rb");
        char buf[4096];

        glm::dvec3 c = glm::dvec3(0.0);

        while(fgets (buf, sizeof(buf), file))
        {
            char token = buf[0];
            //===========================================================================================================================================================================================================
            // skip any line that does not begin with 'v' or 'f'
            //===========================================================================================================================================================================================================
            if ((token != 'v') && (token != 'f')) continue;

            //===========================================================================================================================================================================================================
            // is it a new face?
            //===========================================================================================================================================================================================================
            if ('v' == token)
            {
                glm::dvec3 position;
                if (3 != sscanf(&buf[2], "%lf %lf %lf", &position[0], &position[1], &position[2])) continue;
                c += position;
                positions.push_back(glm::vec3(position));
                continue;
            }

            //===========================================================================================================================================================================================================
            // if not, then it is a new vertex position
            //===========================================================================================================================================================================================================
            glm::uvec3 triangle;
            if (3 != sscanf(&buf[2], "%i %i %i", &triangle[0], &triangle[1], &triangle[2])) continue;
            faces.push_back(triangle - glm::uvec3(1));
        }
        fclose(file);

        V = positions.size();
        F = faces.size();

        debug_msg("File %s parsed : vertices = %u, faces = %u", file_name, V, F);

        c *= (1.0 / V);
        center = glm::vec3(c);
        radius = 0.0f;
        for(uint32_t i = 0; i < V; ++i)
        {
            glm::vec3 dV = positions[i] - center;
            float r2 = glm::dot(dV, dV);
            if (r2 > radius)
                radius = r2;
        }

        radius = glm::sqrt(radius);

        debug_msg("Model mass center :: %s. Radius = %f", glm::to_string(center).c_str(), radius);

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);

        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * V, positions.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

        glGenBuffers(1, &ibo_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::uvec3) * F, faces.data(), GL_STATIC_DRAW);
    }

    ~simple_model_t()
    {
        glDeleteBuffers(1, &vbo_id);
        glDeleteBuffers(1, &ibo_id);
        glDeleteVertexArrays(1, &vao_id);
    }

    void render()
    {
        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLES, 3 * F, GL_UNSIGNED_INT, 0);
    }
};

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("VAO Loader", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // init shaders
    //===================================================================================================================================================================================================================
    glsl_program_t depth_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/depth.vs"),
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/depth.fs"));

    uniform_t uni_dp_camera_matrix = depth_pass["CameraMatrix"];
    uniform_t uni_dp_model_matrix  = depth_pass["ModelMatrix"];

    glsl_program_t light_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/shape.vs"),
                              glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/shape.gs"),
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/shape.fs"));

    light_pass.enable();
    uniform_t uni_lp_camera_matrix   = light_pass["CameraMatrix"];
    uniform_t uni_lp_light_matrix    = light_pass["LightMatrix"];
    uniform_t uni_lp_model_matrix    = light_pass["ModelMatrix"];
    uniform_t uni_lp_camera_position = light_pass["CameraPosition"];
    uniform_t uni_lp_light_position  = light_pass["LightPosition"];
    uniform_t uni_lp_depth_offset    = light_pass["DepthOffs"];

    glm::vec2 depth_offset[32];

    for(int i = 0; i != 32; ++i)
    {
        float u = std::rand() / float(RAND_MAX);
        float v = std::rand() / float(RAND_MAX);
        depth_offset[i].x = float(glm::sqrt(glm::abs(v)) * glm::cos(constants::two_pi * u));
        depth_offset[i].y = float(glm::sqrt(glm::abs(v)) * glm::sin(constants::two_pi * u));
    }

    uni_lp_depth_offset = depth_offset;
    light_pass["DepthMap"] = 0;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonOffset(1.0f, 1.0f);

    depth_fbo_t depth_fbo(0, 2048);

    simple_model_t dragon("../../../resources/manifolds/dragon.obj");

    //===================================================================================================================================================================================================================
    // program main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();

        float t0 = 0.28913f * window.frame_ts;
        float t1 = 0.16735f * window.frame_ts;

        float cs0 = glm::cos(t0);
        float sn0 = glm::sin(t0);
        float cs1 = glm::cos(t1);
        float sn1 = glm::sin(t1);

        glm::vec3 light_ws = dragon.center + 1.5f * dragon.radius * glm::vec3(cs0 * cs1, cs0 * sn1, sn0);
        glm::mat4 light_view_matrix = glm::lookAt(light_ws, dragon.center, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 light_projection_matrix = glm::infinitePerspective(constants::half_pi, 1.0f, 0.25f);
        glm::mat4 light_projection_view_matrix = light_projection_matrix * light_view_matrix;

        glm::vec3 camera_ws = window.camera.position();
        glm::mat4 view_matrix = window.camera.view_matrix;
        glm::mat4 projection_matrix = window.camera.projection_matrix;
        glm::mat4 projection_view_matrix = projection_matrix * view_matrix;

        glm::mat4 model_matrix = glm::mat4(1.0f);

        //===============================================================================================================================================================================================================
        // depth pass
        //===============================================================================================================================================================================================================
        depth_fbo.bind();
        depth_fbo.viewport();

        depth_pass.enable();
        uni_dp_camera_matrix = light_projection_view_matrix;
        uni_dp_model_matrix = model_matrix;

        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_POLYGON_OFFSET_FILL);

        dragon.render();

        //===============================================================================================================================================================================================================
        // render pass
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window.res_x, window.res_y);
        glClearColor(0.04f, 0.01f, 0.09f, 0.0f);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_POLYGON_OFFSET_FILL);

        light_pass.enable();
        uni_lp_camera_matrix   = projection_view_matrix;
        uni_lp_light_matrix    = light_projection_view_matrix;
        uni_lp_model_matrix    = model_matrix;
        uni_lp_camera_position = camera_ws;
        uni_lp_light_position  = light_ws;

        dragon.render();

        //===============================================================================================================================================================================================================
        // show back buffer
        //===============================================================================================================================================================================================================
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}
