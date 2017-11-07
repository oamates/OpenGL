//========================================================================================================================================================================================================================
// DEMO 080 : Fragment Shader Raymarcher
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "log.hpp"
#include "glfw_window.hpp"
#include "gl_aux.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "glsl_noise.hpp"

struct demo_window_t : public glfw_window_t
{
    glm::vec3 mouse_pos;
    bool pause = false;
    double pause_tt = 0.0;
    double pause_ts;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */),
          mouse_pos(0.0f, 0.0f, -1.0f)
    {
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        pause_ts = frame_ts;
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================
    void on_mouse_move() override
    {
        mouse_pos.x = mouse.x;
        mouse_pos.y = mouse.y;
    }

    void on_key(int key, int scancode, int action, int mods) override
    {
        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE))
        {
            pause = !pause;
            if (pause)
                pause_ts = frame_ts;
            else
                pause_tt += (frame_ts - pause_ts);
        }
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

    demo_window_t window("Fragment Shader Raymarcher", 4, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Load textures
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint noise_tex = glsl_noise::randomRG_shift_tex256x256(glm::ivec2(37, 17));
    // GLuint noise_tex = image::png::random_rgb(1024);
    glActiveTexture(GL_TEXTURE1);
    GLuint surface_tex = image::png::texture2d("../../../resources/tex2d/surface.png");
    glActiveTexture(GL_TEXTURE2);
    GLuint clay_tex = image::png::texture2d("../../../resources/tex2d/clay.png");
    glActiveTexture(GL_TEXTURE3);
    GLuint stone_tex = image::png::texture2d("../../../resources/tex2d/window_view.png");

    //===================================================================================================================================================================================================================
    // Shader and uniform variables initialization
    //===================================================================================================================================================================================================================
    glsl_program_t raymarch_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/raymarch.vs"),

//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/canyon.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/flame.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/fractal.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/frozen_glass.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/galaxy.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/klein.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/klein2.fs"));
                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/marble1.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/marble2.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/marble3.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/mobius.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/monster.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/mountains.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/noise.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ocean.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/scattering.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/sierpinski.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/spacelabyrinth.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/volcanic.fs"));
//                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/volumetric_fog.fs"));


    raymarch_renderer.enable();
    uniform_t uniform_mouse_pos = raymarch_renderer["mouse"];
    uniform_t uniform_time = raymarch_renderer["time"];      
    raymarch_renderer["resolution"] = glm::vec2(window.res_x, window.res_y);    

    raymarch_renderer["iChannel0"] = 0;
    raymarch_renderer["iChannel1"] = 1;
    raymarch_renderer["iChannel2"] = 2;
    raymarch_renderer["iChannel3"] = 3;

    //===================================================================================================================================================================================================================
    // Simple full-screen quad buffer setup
    //===================================================================================================================================================================================================================
    glm::vec2 uvs[] = 
    {
        glm::vec2(-1.0f, -1.0f),
        glm::vec2( 1.0f, -1.0f),
        glm::vec2( 1.0f,  1.0f),
        glm::vec2(-1.0f,  1.0f)
    };

    GLuint vao_id, vbo_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        if (!window.pause)
            uniform_time = (float) (window.frame_ts - window.pause_tt);

        uniform_mouse_pos = window.mouse_pos;

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}