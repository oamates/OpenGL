//========================================================================================================================================================================================================================
// DEMO 001 : Empty OpenGL window
//========================================================================================================================================================================================================================
#include "log.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"

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

    glfw_window_t window("Empty OpenGL 3.3 window", 4, 4, 3, 1920, 1080, true);
    gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO | OPENGL_COMPUTE_SHADER_INFO);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : background color -- dark blue
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.0f, 0.08f, 0.0f);


    glsl_program_t compute_shader = glsl_program_t(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/sdf_compute.cs"));
    uniform_t uni_compute_value = compute_shader["value"];

    glsl_program_t combine_shader = glsl_program_t(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/sdf_combine.cs"));
    uniform_t uni_combine_sigma = combine_shader["sigma"];


    const int size = 256;


    GLuint uint32_texture_id1, uint32_texture_id2;

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &uint32_texture_id1);
    glBindTexture(GL_TEXTURE_3D, uint32_texture_id1);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, 256, 256, 256);


    compute_shader.enable();
    uni_compute_value = (unsigned int) 1024;
    glBindImageTexture(0, uint32_texture_id1, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
    glDispatchCompute(32, 32, 32);


    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &uint32_texture_id2);
    glBindTexture(GL_TEXTURE_3D, uint32_texture_id2);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, 256, 256, 256);

    uni_compute_value = (unsigned int) 512;
    glBindImageTexture(0, uint32_texture_id2, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
    glDispatchCompute(32, 32, 32);





    GLuint texture_id;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_3D, texture_id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, 256, 256, 256);

    combine_shader.enable();
    glBindImageTexture(0, texture_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
    glBindImageTexture(1, uint32_texture_id1, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
    glBindImageTexture(2, uint32_texture_id2, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);

    glDispatchCompute(32, 32, 32);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    GLuint pixel_data_size = 4 * 256 * 256 * 256;
    GLvoid* pixels = (GLvoid*) malloc(pixel_data_size);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, texture_id);
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, pixels);

    debug_msg("Printing float texture");

    int w = 112;

    for (int i0 = w; i0 < 256 - w; ++i0) for (int i1 = w; i1 < 256 - w; ++i1) for (int i2 = w; i2 < 256 - w; ++i2)
    {
        int index = i0 + 256 * i1 + 65536 * i2;
        GLfloat value = ((GLfloat *) pixels)[index];
        debug_msg("texture[%u, %u, %u] = %f.", i0, i1, i2, value);
    }

    free(pixels);















    //===================================================================================================================================================================================================================
    // main program loop : just clear the color and depth buffer in a loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}