//========================================================================================================================================================================================================================
// DEMO 050 : Ray Marching
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "glsl_noise.hpp"
#include "image.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    glm::vec3 mouse_pos;
    bool hell = true;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */),
          mouse_pos(0.0f, 0.0f, -1.0f)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.5f);
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

        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE))
            hell = !hell;
    }

    void on_mouse_move() override
    {
        mouse_pos.x = mouse.x;
        mouse_pos.y = mouse.y;
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }

    void on_mouse_button(int button, int action, int mods) override
    {
        mouse_pos.z = (action == GLFW_PRESS) ? 1.0f : -1.0f;
    }
};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Ray Marching", 4, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Shader and uniform variables initialization
    //===================================================================================================================================================================================================================
    glsl_program_t ray_marcher(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ray_marcher.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/mountain.fs"));
//                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/canyon.fs"));
    ray_marcher.enable();
    uniform_t uniform_camera_matrix = ray_marcher["camera_matrix"];
    uniform_t uniform_time = ray_marcher["time"];
    uniform_t uniform_hell = ray_marcher["hell"];

    glm::vec2 focal_scale = glm::vec2(1.0f / window.camera.projection_matrix[0][0], 1.0f / window.camera.projection_matrix[1][1]);
    ray_marcher["focal_scale"] = focal_scale;
    ray_marcher["value_texture"] = 0;
    ray_marcher["stone_texture"] = 1;

    //===================================================================================================================================================================================================================
    // Load noise textures - for very fast 2d noise calculation
    //===================================================================================================================================================================================================================
    //    GLint samples;
    //    glGetIntegerv(GL_SAMPLES, &samples);
    //    glm::vec2 sample_position;
    
    //    debug_msg("The number of samples per pixel :: %d", samples);
    //
    //    for(GLuint index = 0; index < samples; ++index)
    //    {
    //        glGetMultisamplefv(GL_SAMPLE_POSITION, index, glm::value_ptr(sample_position));
    //        debug_msg("GL_SAMPLE_POSITION[%d] = %s", index, glm::to_string(sample_position).c_str());
    //    }
    //
    //    glEnable(GL_SAMPLE_SHADING);
    //    glMinSampleShading(1.0f);

    //===================================================================================================================================================================================================================
    // Load noise textures - for very fast 2d noise calculation
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint noise_tex = glsl_noise::randomRGBA_shift_tex256x256(glm::ivec2(37, 17));
    glActiveTexture(GL_TEXTURE1);
    GLuint stone_tex = image::png::texture2d("../../../resources/tex2d/lava.png");    

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    //===================================================================================================================================================================================================================
    // main program loop : just clear the buffer in a loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        glm::mat4 camera_matrix = glm::inverse(window.camera.view_matrix);
        uniform_time = (float) glfw::time();
        uniform_camera_matrix = camera_matrix;
        uniform_hell = (int) window.hell;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glDeleteTextures(1, &noise_tex);
    glDeleteVertexArrays(1, &vao_id);

    glfw::terminate();
    return 0;
}