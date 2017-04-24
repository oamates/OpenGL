//========================================================================================================================================================================================================================
// DEMO 060 : GLSL Noise Speed Test
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
    glEnable(GL_MULTISAMPLE);

    int param;
    glGetIntegerv(GL_SAMPLE_BUFFERS, &param);
    debug_msg("Number of sample buffers is %d.", param);
    glGetIntegerv(GL_SAMPLES, &param);
    debug_msg("Number of samples is %d.", param);

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
//                                             {"glsl/constant.fs",                 "Constant color .............................."},
//                                             {"glsl/texture.fs",                  "Texture read ................................"},
                                                                                                                                
//                                             {"glsl/value_2d_tl_1c.fs",           "Value    2D 1C : texture (37, 17) lookup ...."},
//                                             {"glsl/value_2d_tl_2c.fs",           "Value    2D 2C : texture (37, 17) lookup ...."},
//                                             {"glsl/value_2d_tl_3c.fs",           "Value    2D 3C : texture (37, 17) lookup ...."},
//                                             {"glsl/value_2d_tl_4c.fs",           "Value    2D 4C : texture (37, 17) lookup ...."},
//                                             {"glsl/value_3d_tl_1c.fs",           "Value    3D 1C : texture (37, 17) lookup ...."},
//
                                             {"glsl/simplex_2d_tl.fs",            "Simplex  2D : texture lookup ................"},
//                                             {"glsl/simplex_2d_compute.fs",       "Simplex  2D : computation ..................."},
                                             {"glsl/gradient_2d_tl.fs",           "Gradient 2D : texture lookup ................"},
                                             {"glsl/gradient_2d_compute.fs",      "Gradient 2D : computation ..................."},
                                                                                                                                
                                             {"glsl/simplex_3d_tl.fs",            "Simplex  3D : texture lookup ................"},
                                             {"glsl/simplex_3d_compute.fs",       "Simplex  3D : computation ..................."},
                                             {"glsl/gradient_3d_tl.fs",           "Gradient 3D : texture lookup ................"},
                                             {"glsl/gradient_3d_compute.fs",      "Gradient 3D : computation ..................."},

                                             {"glsl/simplex_4d_tl.fs",            "Simplex  4D : texture lookup ................"},
                                             {"glsl/simplex_4d_compute.fs",       "Simplex  4D : computation ..................."},
                                             {"glsl/gradient_4d_tl.fs",           "Gradient 4D : texture lookup ................"},
                                             {"glsl/gradient_4d_compute.fs",      "Gradient 4D : computation ..................."},
                                                                                                                                
                                             {"glsl/cellular_2d_comp2x2.fs",      "Cellular 2D : computation 2x2 ..............."},
                                             {"glsl/cellular_2d_comp3x3.fs",      "Cellular 2D : computation 3x3 ..............."},
                                             {"glsl/cellular_2d_comp_vec_3x3.fs", "Cellular 2D : computation 3x3 + vectors ....."},
                                             {"glsl/cellular_3d_comp3x3.fs",      "Cellular 3D : computation 3x3x3 ............."}

                                        };                      

    const int SHADER_COUNT = sizeof(FRAGMENT_SHADERS) / sizeof(FRAGMENT_SHADERS[0]);


    //===================================================================================================================================================================================================================
    // total time in seconds to run each shader
    //===================================================================================================================================================================================================================
    const double benchmarktime = 55.0;           
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
        window.new_frame();
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
        // done : show back buffer
        //===============================================================================================================================================================================================================
        window.end_frame();
    } 

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    fclose(testlog);
    glfw::terminate();
    return 0;
}