//========================================================================================================================================================================================================================
// DEMO 085: CS Bitonic Sort Algorithm
//========================================================================================================================================================================================================================

#include <random>

#include "log.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"
#include "sort.hpp"

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    glfw_window_t window("CS Bitonic Sort Algorithm", 8, 4, 3, 1920, 1080, true);

    const unsigned int GROUP_SIZE = 128;
    const unsigned int GROUP_COUNT = 512;
    const unsigned int ARRAY_SIZE = GROUP_SIZE * GROUP_COUNT;

    int* integral_data = (int*) malloc(ARRAY_SIZE * sizeof(int)); 

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> uniform_int(0, 0x7FFFFFFF);

    for(unsigned int i = 0; i < ARRAY_SIZE; ++i)
        integral_data[i] = uniform_int(generator);

    //===================================================================================================================================================================================================================
    // copy the generated data into OpenGL buffer and create image-like access to it for compute shader
    //===================================================================================================================================================================================================================
    GLuint ibo_id;

    glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ARRAY_BUFFER, ibo_id);
    glBufferData(GL_ARRAY_BUFFER, ARRAY_SIZE * sizeof(GLint), integral_data, GL_DYNAMIC_COPY);

    GLuint itb_id;
    glGenTextures(1, &itb_id);
    glBindTexture(GL_TEXTURE_BUFFER, itb_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, ibo_id);
    glBindImageTexture(0, itb_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);    

    //===================================================================================================================================================================================================================
    // CPU sort of the generated random data
    //===================================================================================================================================================================================================================
    bitonic_sort(integral_data, ARRAY_SIZE);

    FILE* res_cpu = fopen("res_cpu.txt", "w");
    for(unsigned int i = 0; i < ARRAY_SIZE; ++i)
        fprintf(res_cpu, "%d\n", integral_data[i]);
    fclose(res_cpu);

    //===================================================================================================================================================================================================================
    // create and run CS to do the same on GPU
    //===================================================================================================================================================================================================================
    glsl_program_t bitonic_sorter(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/bitonic_sort.cs"));

    bitonic_sorter.enable();
    uniform_t uni_bs_stage = bitonic_sorter["stage"];
    uniform_t uni_bs_pass  = bitonic_sorter["pass"];

    int stages = 0;
    for(unsigned int t = ARRAY_SIZE; t > 1; t >>= 1) ++stages;

    for(int stage = 0; stage < stages; ++stage)
    {
        uni_bs_stage = stage;
        for(int pass = 0; pass < stage + 1; ++pass)
        {
            uni_bs_pass = pass;
            glDispatchCompute(GROUP_COUNT / 2, 1, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
    }

    //===================================================================================================================================================================================================================
    // save gpu sorted buffer
    //===================================================================================================================================================================================================================
    int* sorted_data = (int*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

    FILE* res_gpu = fopen("res_gpu.txt", "w");
    for(unsigned int i = 0; i < ARRAY_SIZE; ++i)
        fprintf(res_gpu, "%d\n", sorted_data[i]);
    fclose(res_gpu);

    glUnmapBuffer(GL_ARRAY_BUFFER);

    //===================================================================================================================================================================================================================
    // done, clean up and exit
    //===================================================================================================================================================================================================================
    free(integral_data);
    glfw::terminate();
    return 0;
}