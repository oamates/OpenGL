//========================================================================================================================================================================================================================
// DEMO 061 : GLSL Noise Speed Test
//========================================================================================================================================================================================================================
#include <cstdio>

#include "log.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"
#include "glsl_noise.hpp"

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    int samples = 8;
    glfw_window_t window("GLSL Noise perfomance test", samples, 4, 0, 1920, 1080, true);

    gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);

    
    //===================================================================================================================================================================================================================
    // Create the two textures required for the TEXTURE lookup noise implementations
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint perm_tex_id = glsl_noise::permutation_texture();

    glActiveTexture(GL_TEXTURE1);
    GLuint grad_tex_id = glsl_noise::gradient_texture();

    glActiveTexture(GL_TEXTURE2);
    GLuint noise_tex = glsl_noise::randomRGBA_shift_tex256x256(glm::ivec2(37, 17));
    
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
    // OpenGL rendering parameters setup : 
    // * DEPTH_TEST disabled for this simple benchmark
    //===================================================================================================================================================================================================================
    glDisable(GL_DEPTH_TEST);

    const char* FRAGMENT_SHADERS[][2] = {
                                             {"glsl/value.fs",           "Value    2D 1C : texture (37, 17) lookup ...."}
                                        };                      

    const int SHADER_COUNT = sizeof(FRAGMENT_SHADERS) / sizeof(FRAGMENT_SHADERS[0]);


    //===================================================================================================================================================================================================================
    // total time in seconds to run each shader
    //===================================================================================================================================================================================================================
    const double benchmarktime = 550.0;           
    int active_shader = -1;
    bool just_finished = true;
    
    glsl_program_t* noise = 0;
    GLuint uniform_time_id = -1;
    double ts;
    int frames;

    FILE* testlog = fopen("test.log", "w");

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        if (just_finished)
        {
            frames = 0;
            active_shader++;
            if (active_shader >= SHADER_COUNT) break;
            just_finished = false;
            if (noise) delete noise;
            noise = new glsl_program_t(glsl_shader_t(GL_VERTEX_SHADER, "glsl/noise.vs"),
                                       glsl_shader_t(GL_FRAGMENT_SHADER, FRAGMENT_SHADERS[active_shader][0]));
            fprintf(testlog, "Running %s ... ", FRAGMENT_SHADERS[active_shader][1]);
            
            ts = glfw::time();
            noise->enable();
            (*noise)["noise_texture"] = 0;
            (*noise)["gradTexture"] = 1;
            (*noise)["value_texture"] = 2;
            uniform_time_id = (*noise)["time"];
        }

        double time = glfw::time();
        glUniform1f(uniform_time_id, float(time));

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        frames++;

        time = glfw::time();

        if (time - ts >= benchmarktime)
        {
            just_finished = true;
            double fps = (double) frames / (time - ts);
            double rate = double(samples) * double(window.res_x) * double(window.res_y) * fps * 1e-6;
            fprintf(testlog, "... done. FPS = %0.2f. Speed = %0.2f millions samples/second.\n", fps, rate);
        }

        //===============================================================================================================================================================================================================
        // done : show back buffer and process events
        //===============================================================================================================================================================================================================
        window.swap_buffers();
        glfw::poll_events();
    } 

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    fclose(testlog);
    glfw::terminate();
    return 0;
}