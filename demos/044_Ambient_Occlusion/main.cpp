//========================================================================================================================================================================================================================
// DEMO 044 : Ambient Occlusion
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "shadowmap.hpp"

const unsigned int TEXTURE_SIZE = 1024;
const unsigned int res_x = 1920;
const unsigned int res_y = 1080;


struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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

struct landscape
{
    GLuint vao_id, vbo_id, nbo_id, tbo_id, ibo_id;
    GLuint triangles;


    typedef float (*generator_func) (const glm::vec2& argument);

    void generate_vao(generator_func func, glm::ivec2 size, const glm::vec3& shift, const glm::mat3& rotation_matrix)
    {

        unsigned int VERTEX_COUNT = (size.x + 1) * (size.y + 1);
        glm::vec3* vertices = (glm::vec3*) malloc((sizeof(glm::vec3) + sizeof(glm::vec2)) * VERTEX_COUNT);
        glm::vec2* uvs = (glm::vec2*) malloc (sizeof(glm::vec2) * VERTEX_COUNT);
        glm::vec3* normals = (glm::vec3*) calloc(VERTEX_COUNT, sizeof(glm::vec3));

        triangles = 2 * size.x * size.y;
        unsigned int INDEX_COUNT = 6 * size.x * size.y;
        glm::ivec3* indices = (glm::ivec3*) malloc(sizeof(glm::ivec3) * INDEX_COUNT);

        //===============================================================================================================================================================================================================
        // vertices and texture coordinates
        //===============================================================================================================================================================================================================

        glm::vec2 delta = glm::vec2(2.0f / size.x, 2.0f / size.y);
        glm::vec2 argument;
        argument.y = -1.0f;

        unsigned int index = 0;
        for (int v = 0; v <= size.x; ++v)
        {
            argument.x = -1.0f;
            for (int u = 0; u <= size.y; ++u)
            {
//              vertices[index] = shift + rotation_matrix * glm::vec3(argument, func(argument));
                vertices[index] = glm::vec3(argument, func(argument));
                uvs[index++] = (argument + glm::vec2(1.0f, 1.0f)) * 0.5f;
                argument.x += delta.x;
            }
            argument.y += delta.y;  
        }

        //===============================================================================================================================================================================================================
        // indices and normals
        //===============================================================================================================================================================================================================
        unsigned int mesh_index = 0;
        index = 0;
        for (int v = 0; v < size.y; ++v)
        {
            for (int u = 0; u < size.x; ++u)
            {
                indices[index] = glm::ivec3(mesh_index, mesh_index + 1, mesh_index + size.x + 2);

                debug_msg("indices[%u] = %s", index, glm::to_string(indices[index]).c_str());

                glm::vec3 normal0 = glm::cross(vertices[indices[index].y] - vertices[indices[index].x], vertices[indices[index].z] - vertices[indices[index].x]);
                normals[indices[index].x] += normal0;
                normals[indices[index].y] += normal0;
                normals[indices[index].z] += normal0;
                ++index;
                
                indices[index] = glm::ivec3(mesh_index, mesh_index + size.x + 2, mesh_index + size.x + 1);
                debug_msg("indices[%u] = %s", index, glm::to_string(indices[index]).c_str());

                glm::vec3 normal1 = glm::cross(vertices[indices[index].y] - vertices[indices[index].x], vertices[indices[index].z] - vertices[indices[index].x]);
                normals[indices[index].x] += normal1;
                normals[indices[index].y] += normal1;
                normals[indices[index].z] += normal1;
                ++index;
                ++mesh_index;
            };
            ++mesh_index;
        };

        for (index = 0; index < VERTEX_COUNT; ++index) normals[index] = glm::normalize(normals[index]);

        GLuint q = size.x + 1;
        GLuint p = 0;
        unsigned int idx = 0;
        for (int i = 0; i < size.y; ++i)
        {
            for (int j = 0; j < size.x; ++j)
            {
                debug_msg("indices[%u] = %s", idx++, glm::to_string(glm::ivec3(p, p + 1, q + 1)).c_str());
                debug_msg("indices[%u] = %s", idx++, glm::to_string(glm::ivec3(p, q + 1, q)).c_str());
                ++p; ++q;
            }
            ++p; ++q;
        }

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);

        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, VERTEX_COUNT * sizeof(glm::vec3), glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glGenBuffers(1, &nbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, nbo_id);
        glBufferData(GL_ARRAY_BUFFER, VERTEX_COUNT * sizeof(glm::vec3), glm::value_ptr(normals[0]), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glGenBuffers(1, &tbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, tbo_id);
        glBufferData(GL_ARRAY_BUFFER, VERTEX_COUNT * sizeof(glm::vec2), glm::value_ptr(uvs[0]), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
        glGenBuffers(1, &ibo_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_COUNT * sizeof(glm::ivec3), glm::value_ptr(indices[0]), GL_STATIC_DRAW);

        free(vertices);
        free(normals);
        free(uvs);
        free(indices);
    }

    void render ()
    {
        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLES, 3 * triangles, GL_UNSIGNED_INT, 0);
    }
};

float func (const glm::vec2& argument)
{
    float x = argument.x;
    float y = argument.y;
    return 1.5f * glm::sin(5.0f * x) + 
           1.4f * glm::sin(7.0f * y) +
           0.3f * glm::sin(21.0f * x + 22.0f * y) +
           0.1f * glm::sin(30.0f * x - 17.0f * y) +
           0.05f * glm::sin(73.1235f * x + 53.73457f * y) +
           0.02f * glm::sin(42.0234f * x - 63.90423f * y);
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Ambient occlusion", 8, 3, 3, 1920, 1080, true);

    float fov = constants::two_pi / 6.0f;
    float aspect_ratio = float(res_x) / float(res_y);
    float znear = 1.0f;
    glm::vec2 scale_xy = -glm::vec2(glm::tan(fov / 2.0f));
    scale_xy.x *= aspect_ratio;

    //===================================================================================================================================================================================================================
    // shader program for geometry pass
    //===================================================================================================================================================================================================================
    glsl_program_t geometry(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/geometry_pass.vs"),
                            glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/geometry_pass.fs"));
    geometry.enable();
    uniform_t geometry_pass_mvp_matrix = geometry["mvp_matrix"];

    //===================================================================================================================================================================================================================
    // shader program for screen-space ambient occlusion pass
    //===================================================================================================================================================================================================================
    glsl_program_t ssao(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao.vs"),
                        glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao.fs"));
    ssao.enable();
    ssao["scale_xy"] = scale_xy;
    ssao["projection_matrix"] = window.camera.projection_matrix;

    const unsigned int MAX_KERNEL_SIZE = 64;
    glm::vec3 rnd[MAX_KERNEL_SIZE];
    for (int i = 0; i < MAX_KERNEL_SIZE; ++i)
    {
        rnd[i] = glm::sphericalRand(1.0f);
        debug_msg("spherical_rand[%d] = %s", i, glm::to_string(rnd[i]).c_str());
    };
    glUniform3fv(ssao["spherical_rand"], MAX_KERNEL_SIZE, glm::value_ptr(rnd[0]));


    //===================================================================================================================================================================================================================
    // shader program for ssao image blur 
    //===================================================================================================================================================================================================================
    glsl_program_t blur(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/blur.vs"),
                        glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blur.fs"));

    //===================================================================================================================================================================================================================
    // lighting shader program 
    //===================================================================================================================================================================================================================
    glsl_program_t simple_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/simple_light.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/simple_light.fs"));

    simple_light.enable();

    uniform_t simple_light_view_matrix  = simple_light["view_matrix"]; 
    uniform_t simple_light_model_matrix = simple_light["model_matrix"];
    simple_light["projection_matrix"] = window.camera.projection_matrix;

    //===================================================================================================================================================================================================================
    // creating landscape mesh
    //===================================================================================================================================================================================================================

    glm::vec3 shift = glm::vec3(0.0f);
    glm::mat3 rotation_matrix = glm::mat3(1.0f);
    glm::mat4 model_matrix = glm::scale(glm::vec3(200.0f, 200.0f, 20.0f));


    landscape waves;
    waves.generate_vao(func, glm::ivec2(32, 32), shift, rotation_matrix);
    debug_msg("Landscape generated");


    depth_map zbuffer(res_x, res_y);
    color_map ssao_buffer(res_x, res_y);
    color_map blur_buffer(res_x, res_y);

    //===================================================================================================================================================================================================================
    // generate quad VAO
    //===================================================================================================================================================================================================================
    GLuint quad_vao_id, quad_vbo_id;
    
    glm::vec2 quad_data [] = 
    {
        glm::vec2(-1.0f, -1.0f),
        glm::vec2( 1.0f, -1.0f),
        glm::vec2( 1.0f,  1.0f),
        glm::vec2(-1.0f,  1.0f)
    };

    glGenVertexArrays(1, &quad_vao_id);
    glBindVertexArray(quad_vao_id);

    glGenBuffers(1, &quad_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), glm::value_ptr(quad_data[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    debug_msg("projection_matrix = %s", glm::to_string(window.camera.projection_matrix).c_str());

    while(!window.should_close())
    {
        window.new_frame();
        glm::mat4 mvp_matrix;

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);                                                                                   // dark blue background

        //===============================================================================================================================================================================================================
        // geometry pass
        //===============================================================================================================================================================================================================
        geometry.enable();
        zbuffer.bind();
    
        glClear(GL_DEPTH_BUFFER_BIT);   

        mvp_matrix = window.camera.projection_matrix * window.camera.view_matrix * model_matrix;
        geometry_pass_mvp_matrix = mvp_matrix;
        waves.render();

        glBindVertexArray(quad_vao_id);

        //===============================================================================================================================================================================================================
        // ssao pass
        //===============================================================================================================================================================================================================
        ssao.enable();
        ssao_buffer.bind();
        glClear(GL_COLOR_BUFFER_BIT);
        zbuffer.bind_texture(GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        //===============================================================================================================================================================================================================
        // blur pass
        //===============================================================================================================================================================================================================
        blur.enable();
        blur_buffer.bind();
        glClear(GL_COLOR_BUFFER_BIT);
        ssao_buffer.bind_texture(GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        //===============================================================================================================================================================================================================
        // lighting pass
        //===============================================================================================================================================================================================================
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.01f, 0.0f, 0.05f, 0.0f);                                                                                 // dark blue background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        simple_light.enable();
        blur_buffer.bind_texture(GL_TEXTURE0);

        simple_light_view_matrix = window.camera.view_matrix;
        simple_light_model_matrix = model_matrix;
        waves.render();           

        window.end_frame();
    }
    
    //===================================================================================================================================================================================================================
    // close OpenGL window and terminate GLFW
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}