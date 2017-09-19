//========================================================================================================================================================================================================================
// DEMO 074 : Grass Rendering
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "imgui_window.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "image.hpp"
#include "plato.hpp"
#include "polyhedron.hpp"
#include "glsl_noise.hpp"

const float z_near = 0.5;
const float inv_grass_scale = 64.0f;

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    bool pause = false;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), z_near);
    }

    //===================================================================================================================================================================================================================
    // mouse handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE))
            pause = !pause;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

glm::vec3 tri(const glm::vec3& x)
{
    return glm::abs(glm::fract(x) - glm::vec3(0.5f));
}

float potential(const glm::vec3& p)
{    
    glm::vec3 q = p;
    glm::vec3 oq = tri(1.1f * q + tri(1.1f * glm::vec3(q.z, q.x, q.y)));
    float ground = q.z + 3.5f + glm::dot(oq, glm::vec3(0.067));
    q += (oq - glm::vec3(0.25f)) * 0.3f;
    q = glm::cos(0.444f * q + glm::sin(1.112f * glm::vec3(q.z, q.x, q.y)));
    float canyon = 0.95f * (glm::length(p) - 1.05f);
    float sphere = 11.0f - glm::length(p);
    return glm::min(glm::min(ground, canyon), sphere);
}

glm::vec3 gradient(const glm::vec3& p)
{
    const float delta = 0.075f;

    const glm::vec3 dX = glm::vec3(delta, 0.0f, 0.0f);
    const glm::vec3 dY = glm::vec3(0.0f, delta, 0.0f);
    const glm::vec3 dZ = glm::vec3(0.0f, 0.0f, delta);

    glm::vec3 dF = glm::vec3
    (
        potential(p + dX) - potential(p - dX),
        potential(p + dY) - potential(p - dY),
        potential(p + dZ) - potential(p - dZ)
    );

    return glm::normalize(dF);
}

glm::vec3 move(glm::vec3& position, glm::vec3& velocity, float dt)
{
    glm::vec3 v0 = gradient(position);
    glm::vec3 v1 = velocity;

    glm::vec3 v = glm::normalize(v1 + v0);
    velocity = v;
    position = position + dt * v;
}


//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Grass Rendering", 4, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // raymarch shader and uniform variables initialization
    //===================================================================================================================================================================================================================
    glsl_program_t ray_marcher(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ray_marcher.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/canyon.fs"));
    ray_marcher.enable();
    uniform_t uni_rm_camera_matrix = ray_marcher["camera_matrix"];
    uniform_t uni_rm_camera_ws = ray_marcher["camera_ws"];
    uniform_t uni_rm_light_ws = ray_marcher["light_ws"];
    uniform_t uni_rm_time = ray_marcher["time"];

    glm::vec2 focal_scale = glm::vec2(1.0f / window.camera.projection_matrix[0][0], 1.0f / window.camera.projection_matrix[1][1]);
    ray_marcher["focal_scale"] = focal_scale;
    ray_marcher["stone_tex"] = 0;
    ray_marcher["grass_tex"] = 1;
    ray_marcher["clay_tex"] = 2;
    ray_marcher["z_near"] = z_near;

    //===================================================================================================================================================================================================================
    // grass generating shader
    //===================================================================================================================================================================================================================
    glsl_program_t grass_generator(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/grass_gen.vs"),
                                   glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/grass_gen.gs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/grass_gen.fs"));

    grass_generator.enable();
    uniform_t uni_gg_pv_matrix = grass_generator["projection_view_matrix"];
    uniform_t uni_gg_light_ws  = grass_generator["light_ws"];
    uniform_t uni_gg_camera_ws = grass_generator["camera_ws"];
    uniform_t uni_gg_origin    = grass_generator["origin"];
    uniform_t uni_gg_time      = grass_generator["time"];
    grass_generator["blade_tex"] = 3;
    grass_generator["grid_scale"] = 1.0f / inv_grass_scale;

    //===================================================================================================================================================================================================================
    // load textures
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint stone_tex_id = image::png::texture2d("../../../resources/tex2d/moss.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    glActiveTexture(GL_TEXTURE1);
    GLuint grass_tex_id = image::png::texture2d("../../../resources/tex2d/grass_dirt.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    glActiveTexture(GL_TEXTURE2);
    GLuint clay_tex_id = image::png::texture2d("../../../resources/tex2d/clay.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    glActiveTexture(GL_TEXTURE3);
    GLuint grass_blades_tex_id = image::png::texture2d_array("../../../resources/tex2d/nature/grass/grass_blade_%u.png", 8);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    glm::vec3 light_ws = glm::sphericalRand(11.0f);
    glm::vec3 light_velocity = glm::sphericalRand(1.0f);
    float p = potential(light_ws);

    while(p < 0.5f)
    {
        move(light_ws, light_velocity, 0.125f);
        p = potential(light_ws);
    }

    glEnable(GL_DEPTH_TEST);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        if (!window.pause)
        {
            move(light_ws, light_velocity, window.frame_dt);
            float p = potential(light_ws);

            while(p < 0.5f)
            {
                move(light_ws, light_velocity, 0.125f);
                p = potential(light_ws);
            }
        }
        
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::mat4 cmatrix4x4 = glm::inverse(window.camera.view_matrix);
        glm::mat3 camera_matrix = glm::mat3(cmatrix4x4);
        glm::vec3 camera_ws = glm::vec3(cmatrix4x4[3]);

        //===============================================================================================================================================================================================================
        // render raymarch scene
        //===============================================================================================================================================================================================================
        glDepthFunc(GL_ALWAYS);
        ray_marcher.enable();

        uni_rm_time = time;
        uni_rm_camera_matrix = camera_matrix;
        uni_rm_camera_ws = camera_ws;
        uni_rm_light_ws = light_ws;

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // render grass
        //===============================================================================================================================================================================================================
        glDepthFunc(GL_LESS);
        grass_generator.enable();

        uni_gg_time = time;
        uni_gg_pv_matrix = projection_view_matrix;
        uni_gg_light_ws  = light_ws;
        uni_gg_camera_ws = camera_ws;

        const int half_res = 1024;
        const int full_res = 2 * half_res + 1;
        uni_gg_origin = glm::ivec2(inv_grass_scale * camera_ws.x, inv_grass_scale * camera_ws.y) - glm::ivec2(half_res);
        glDrawArraysInstanced(GL_POINTS, 0, full_res, full_res);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glDeleteTextures(1, &stone_tex_id);
    glDeleteTextures(1, &grass_tex_id);
    glDeleteTextures(1, &grass_blades_tex_id);
    glDeleteVertexArrays(1, &vao_id);

    glfw::terminate();
    return 0;
}