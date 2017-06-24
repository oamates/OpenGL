//========================================================================================================================================================================================================================
// DEMO 085: CS Bitonic Sort Algorithm
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/ext.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    bool position_changed = false;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.5f);
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
    }

    //===================================================================================================================================================================================================================
    // mouse handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) { camera.move_forward(frame_dt);   position_changed = true; }
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) { camera.move_backward(frame_dt);  position_changed = true; }
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) { camera.straight_right(frame_dt); position_changed = true; }
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) { camera.straight_left(frame_dt);  position_changed = true; }
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

//========================================================================================================================================================================================================================
// bitonic sort algorithm implementation
//========================================================================================================================================================================================================================
template<typename T> void bitonic_sort(T* a, const int length)
{
    int half_length = length >> 1;
    int l = half_length;
    int stage = 0;

    while(l)
    {
        int pair_distance = 1 << stage;

        for(int pass = 0; pass <= stage; ++pass)
        {
            for(int id = 0; id < half_length; ++id)
            {
                int l = id + (id & (-pair_distance));
                int r = l + pair_distance;

                T l_elem = a[l];
                T r_elem = a[r];

                bool correct_order = ((id >> stage) & 1) == 0;
                bool actual_order = l_elem < r_elem;

                if (actual_order ^ correct_order)
                {
                    a[l] = r_elem;
                    a[r] = l_elem;
                }
            }
            pair_distance >>= 1;
        }

        stage++;
        l >>= 1;
    }
}

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

    demo_window_t window("CS Bitonic Sort Algorithm", 8, 4, 3, 1920, 1080, true);

    const unsigned int GROUP_SIZE = 128;
    const unsigned int GROUP_COUNT = 512;
    const unsigned int ARRAY_SIZE = GROUP_SIZE * GROUP_COUNT;
    int stages = 0;
    for(unsigned int temp = ARRAY_SIZE; temp > 1; temp >>= 1) ++stages;


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
    // create CS to do the same on GPU
    //===================================================================================================================================================================================================================
    glsl_program_t bitonic_sorter(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/bitonic_sort.cs"));

    bitonic_sorter.enable();
    uniform_t uni_bs_stage = bitonic_sorter["stage"];
    uniform_t uni_bs_pass = bitonic_sorter["pass"];

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

    int* sorted_data = (int*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
    FILE* res_gpu = fopen("res_gpu.txt", "w");
    for(unsigned int i = 0; i < ARRAY_SIZE; ++i)
        fprintf(res_gpu, "%d\n", sorted_data[i]);
    fclose(res_gpu);

    glUnmapBuffer(GL_ARRAY_BUFFER);

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    free(integral_data);
    glfw::terminate();
    return 0;
}