//========================================================================================================================================================================================================================
// DEMO 047 : Ray Tracer
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.5);
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
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};


struct sphere_t
{
    glm::vec3 center;                           // sphere center
    float radius;                               // sphere radius
    glm::vec3 albedo;                           // surface albedo color
    float transparency;                         // surface transparency
    glm::vec3 emission;                         // surface emission color
    float energy;                               // emission energy
    float reflectivity;                         // surface reflectivity
    float ior;                                  // surface index of refraction
    float pad0;                                 // padding0
    float pad1;                                 // padding1
};

sphere_t spheres[] = 
{
    { .center = glm::vec3( 0.0f, -10004.0f, -20.0f), .radius = 10000.0f, .albedo = glm::vec3(0.80f, 0.20f, 0.20f), .transparency = 0.0f, .emission = glm::vec3(0.01, 0.20, 0.04), .energy = 0.71f, .reflectivity = 0.57f, .ior = 1.17f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3( 0.0f, -10004.0f, -20.0f), .radius = 10000.0f, .albedo = glm::vec3(0.80f, 0.20f, 0.20f), .transparency = 0.1f, .emission = glm::vec3(0.01, 0.20, 0.04), .energy = 0.21f, .reflectivity = 0.57f, .ior = 1.17f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3( 0.0f, -10004.0f, -20.0f), .radius = 10000.0f, .albedo = glm::vec3(0.80f, 0.20f, 0.20f), .transparency = 0.2f, .emission = glm::vec3(0.01, 0.20, 0.04), .energy = 0.23f, .reflectivity = 0.57f, .ior = 1.17f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3( 0.0f, -10004.0f, -20.0f), .radius = 10000.0f, .albedo = glm::vec3(0.80f, 0.20f, 0.20f), .transparency = 0.3f, .emission = glm::vec3(0.01, 0.20, 0.04), .energy = 0.00f, .reflectivity = 0.57f, .ior = 1.17f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3( 0.0f, -10004.0f, -20.0f), .radius = 10000.0f, .albedo = glm::vec3(0.80f, 0.20f, 0.20f), .transparency = 0.5f, .emission = glm::vec3(0.01, 0.20, 0.04), .energy = 0.02f, .reflectivity = 0.57f, .ior = 1.17f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3( 0.0f, -10004.0f, -20.0f), .radius = 10000.0f, .albedo = glm::vec3(0.80f, 0.20f, 0.20f), .transparency = 0.6f, .emission = glm::vec3(0.01, 0.20, 0.04), .energy = 0.41f, .reflectivity = 0.57f, .ior = 1.17f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3( 0.0f, -10004.0f, -20.0f), .radius = 10000.0f, .albedo = glm::vec3(0.80f, 0.20f, 0.20f), .transparency = 0.7f, .emission = glm::vec3(0.01, 0.20, 0.04), .energy = 0.21f, .reflectivity = 0.57f, .ior = 1.17f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3( 0.0f, -10004.0f, -20.0f), .radius = 10000.0f, .albedo = glm::vec3(0.80f, 0.20f, 0.20f), .transparency = 0.8f, .emission = glm::vec3(0.01, 0.20, 0.04), .energy = 0.11f, .reflectivity = 0.57f, .ior = 1.17f, .pad0 = 0.0f, .pad1 = 0.0f }
};

/*
    spheres.push_back(sphere_t<real_t>(vec3( 0.0, -10004.0, -20.0), 10000.0, , ,          0.1,         0.22, 1.11));

    spheres.push_back(sphere_t<real_t>(vec3( 0.0,      0.0, -20.0),     4.0, vec3(1.00, 1.32, 0.36), vec3(0.07, 0.12, 0.11),          0.3,         0.57, 1.17)); 
    spheres.push_back(sphere_t<real_t>(vec3( 5.0,     -1.0, -15.0),     2.0, vec3(0.90, 0.76, 0.46), vec3(0.05, 0.06, 0.09),          0.5,         0.11, 1.31)); 
    spheres.push_back(sphere_t<real_t>(vec3( 5.0,      0.0, -25.0),     3.0, vec3(0.65, 0.77, 0.97), vec3(0.03, 0.11, 0.17),          0.7,         0.13, 1.15)); 
    spheres.push_back(sphere_t<real_t>(vec3(-5.5,      0.0, -15.0),     3.0, vec3(0.90, 0.90, 0.90), vec3(0.21, 0.10, 0.06),          0.9,         0.07, 1.07));
    spheres.push_back(sphere_t<real_t>(vec3( 0.0,     20.0, -30.0),     3.0, vec3(0.00, 0.00, 0.00), vec3(3.00, 2.00, 3.00),          1.0,         0.21, 1.19));
*/

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char* argv[])
{
    const int res_x = 1920;
    const int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Ray Tracer", 4, 4, 3, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // shaders
    //===================================================================================================================================================================================================================
    glsl_program_t ray_tracer(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/trace.cs"));
    ray_tracer.enable();

    uniform_t uni_rt_camera_ws = ray_tracer["camera_ws"];
    uniform_t uni_rt_camera_matrix = ray_tracer["camera_matrix"];

    ray_tracer["inv_res"] = glm::vec2(1.0f / res_x, 1.0f / res_y);
    ray_tracer["focal_scale"] = window.camera.focal_scale();
    ray_tracer["depth"] = 8;
    ray_tracer["sphere_count"] = 8;


    glsl_program_t quad_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/render.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/render.fs"));

    quad_renderer["raytrace_image"] = 0;


    //===================================================================================================================================================================================================================
    // uniform buffer for spheres
    //===================================================================================================================================================================================================================
    GLuint ubo_id;
    glGenBuffers(1, &ubo_id);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(spheres), spheres, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_id);

    //===================================================================================================================================================================================================================
    // fake VAO for full-screen quad rendering
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    //===================================================================================================================================================================================================================
    // raytrace texture will occupy texture unit 0 and image unit 0
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint raytrace_image;
    glGenTextures(1, &raytrace_image);
    glBindTexture(GL_TEXTURE_2D, raytrace_image);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, res_x, res_y);
    glBindImageTexture(0, raytrace_image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    //===================================================================================================================================================================================================================
    // Global GL settings : DEPTH not needed, hence disabled
    //===================================================================================================================================================================================================================
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();

        //===============================================================================================================================================================================================================
        // ray trace the scene
        //===============================================================================================================================================================================================================
        glm::mat4 cmatrix4x4 = window.camera.camera_matrix();
        glm::mat3 camera_matrix = glm::mat3(cmatrix4x4);
        glm::vec3 camera_ws = glm::vec3(cmatrix4x4[3]);

        ray_tracer.enable();
        uni_rt_camera_ws = camera_ws;
        uni_rt_camera_matrix = camera_matrix;

        glDispatchCompute(res_x / 8, res_y / 8, 1);                                             // fill initial ray buffer
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        //===============================================================================================================================================================================================================
        // copy image to screen buffer
        //===============================================================================================================================================================================================================
        quad_renderer.enable();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        window.end_frame();
    }

    glDeleteTextures(1, &raytrace_image);
    glDeleteVertexArrays(1, &vao_id);

    glfw::terminate();
    return 0;
}