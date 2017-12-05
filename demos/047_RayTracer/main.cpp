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
    glsl_program_t ray_initializer(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/init.cs"));
    ray_initializer.enable();

    uniform_t uni_ri_camera_ws = ray_initializer["camera_ws"];
    uniform_t uni_ri_camera_matrix = ray_initializer["camera_matrix"];
    ray_initializer["inv_res"] = glm::vec2(1.0f / res_x, 1.0f / res_y);
    ray_initializer["focal_scale"] = window.camera.focal_scale();

    glsl_program_t ray_tracer(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/trace.cs"));

    glsl_program_t quad_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/render.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/render.fs"));

    quad_renderer["raytrace_image"] = 0;


    //===================================================================================================================================================================================================================
    // ping-pong ray ssbo buffers and atomic counter buffers
    //===================================================================================================================================================================================================================
    GLuint ray_buffer[2], acbo_id;
    GLsizeiptr ssbo_size = 64 * res_x * res_y;

    glGenBuffers(2, ray_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ray_buffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_size, 0, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ray_buffer[0]);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ray_buffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_size, 0, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ray_buffer[1]);

    const GLuint zeros[] = {0, 0};
    glGenBuffers(1, &acbo_id);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, acbo_id);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(zeros), zeros, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, acbo_id);

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
        // 0. initialization step
        //===============================================================================================================================================================================================================
        glm::mat4 cmatrix4x4 = window.camera.camera_matrix();
        glm::mat3 camera_matrix = glm::mat3(cmatrix4x4);
        glm::vec3 camera_ws = glm::vec3(cmatrix4x4[3]);

        ray_initializer.enable();
        uni_ri_camera_ws = camera_ws;
        uni_ri_camera_matrix = camera_matrix;

        glDispatchCompute(res_x / 8, res_y / 8, 1);                                             // fill initial ray buffer
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        //===============================================================================================================================================================================================================
        // 1. ray trace step
        //===============================================================================================================================================================================================================
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(zeros), zeros, GL_DYNAMIC_COPY);          // reset atomic counters

        ray_tracer.enable();

        glDispatchCompute(res_x / 8, res_y / 8, 1);                                             // run one raytrace step
        glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
        
        glm::ivec2* ptr = (glm::ivec2*) glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);    // read atomic counter values
        glm::ivec2 counters = *ptr;
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

        debug_msg("Counters: {%u, %u}", counters.x, counters.y);
        debug_msg("Expected: {%u, %u}", res_x * res_y, 2 * res_x * res_y);

        //===============================================================================================================================================================================================================
        // 2. render step -- copy image to screen buffer
        //===============================================================================================================================================================================================================
        quad_renderer.enable();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        window.end_frame();
    }

    GLuint buffers[] = { ray_buffer[0], ray_buffer[1], acbo_id };
    glDeleteBuffers(3, buffers);

    glDeleteTextures(1, &raytrace_image);
    glDeleteVertexArrays(1, &vao_id);

    glfw::terminate();
    return 0;
}