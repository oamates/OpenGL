//========================================================================================================================================================================================================================
// DEMO 037 : Noise
//========================================================================================================================================================================================================================
#define GLM_FORCE_INLINE
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "vao.hpp"
#include "vertex.hpp"
#include "image.hpp"

#if defined(__AVX2__)
    #include <immintrin.h>              // AVX2 intrinsics header
    #pragma message("Noise Library will be compiled for AVX2 instructions set.")
#elif defined(__SSE4_1__)
    #include <smmintrin.h>              // SSE4.1 intrinsics header
    #pragma message("Noise Library will be compiled for SSE4.1 instructions set.")
#elif defined(__SSE2__)
    #include <xmmintrin.h>              // SSE intrinsics header
    #include <emmintrin.h>              // SSE 2 intrinsics header
    #pragma message("Noise Library will be compiled for SSE/SSE2 instructions set.")
#else
    #pragma message("Noise Library will be compiled for generic instructions set.")
#endif

#include "simdhelp.hpp"
#include "common.hpp"
#include "value.hpp"
#include "simplex.hpp"
#include "gradient.hpp"
#include "sse2.hpp"
#include "sse41.hpp"
#include "avx2.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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

//========================================================================================================================================================================================================================
// Implementation table :: 
//========================================================================================================================================================================================================================
//========================================================================================================================================================================================================================
//| ******************* |    cpp     |    sse2    |    sse41   |    avx2    |
//========================================================================================================================================================================================================================
//|   value2d (float)   | ********** |            |            | ********** |
//|   value3d (float)   | ********** |            |            |            |
//|   value4d (float)   | ********** |            |            | ********** |
//|                     |            |            |            |            |
//|  simplex2d (float)  | ********** |            |            |            |
//|  simplex3d (float)  | ********** |            |            |            |
//|  simplex4d (float)  | ********** |            |            |            |
//|                     |            |            |            |            |
//| gradient2d (float)  | ********** |            |            |            |
//| gradient3d (float)  | ********** |            |            |            |
//| gradient4d (float)  | ********** |            |            | ********** |
//|                     |            |            |            |            |
//|   value2d (double)  | ********** | ********** | ********** | ********** |
//|   value3d (double)  | ********** |            |            |            |
//|   value4d (double)  | ********** |            |            | ********** |
//|                     |            |            |            |            |
//|  simplex2d (double) | ********** |            |            | ********** |
//|  simplex3d (double) | ********** |            |            |            |
//|  simplex4d (double) | ********** |            |            |            |
//|                     |            |            |            |            |
//| gradient2d (double) | ********** |            | ********** | ********** |
//| gradient3d (double) | ********** |            |            |            |
//| gradient4d (double) | ********** |            |            | ********** |
//|                     |            |            |            |            |
//========================================================================================================================================================================================================================

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    glm::dvec2 w = glm::dvec2(3.1, 1.7);
    double v = noise::gradient_sse41(w);
    printf("v = %f\n\n\n\n\n\n", v);

    w = glm::dvec2(1.4, -1.32);
    v = noise::gradient_sse41(w);
    printf("v = %f\n\n\n\n\n\n", v);

//    return 0;

    const int TEXTURE_SIZE = 4096;
    uint8_t* rgb_data = (uint8_t*) malloc(TEXTURE_SIZE * TEXTURE_SIZE * 3);
    uint8_t* p = rgb_data;

    double max = 0.0;
    double min = 0.0;

    glm::dmat2 m = 2.11 * glm::dmat2(glm::dvec2(0.6, 0.8), glm::dvec2(-0.8, 0.6));

    for(int u = 0; u < TEXTURE_SIZE; ++u)
    {
        for(int v = 0; v < TEXTURE_SIZE; ++v)
        {
//            glm::vec4 q = glm::vec4(-1.5 + 0.00555 * u, 1.7 + 0.00555 * v, 1.7 + 0.00555 * v, 1.7 + 0.00555 * u);
//            glm::dvec4 q = glm::dvec4(-1.5 + 0.00555 * u, 1.7 + 0.00555 * v, 1.7 + 0.00555 * v, 1.7 + 0.00555 * u);
//            glm::vec3 q = glm::vec3(-1.5 + 0.00555 * u, 1.7 + 0.00555 * v, 1.7 + 0.00555 * v);
//            glm::dvec4 q = glm::dvec4(-1.5 + 0.00555 * u, 1.7 + 0.00555 * v, -1.5 + 0.00555 * u, 1.7 + 0.00555 * v);
//            glm::vec2 q = glm::vec2(-1.5 + 0.00555 * u, 1.7 + 0.00555 * v);
            glm::dvec2 q = glm::dvec2(-1.5 + 0.00555 * u, 1.7 + 0.00555 * v);
//            double t = noise::gradient<double>(q);
            double t = noise::gradient_sse41<double>(q);



            double value = 0.5f + 0.5f * t;

            if (t < min) min = t;
            if (t > max) max = t;

            float value_norm = 255.0 * value;
            uint8_t red = (uint8_t) (value_norm); 

            *(p + 0) = red;
            *(p + 1) = red;
            *(p + 2) = red;
            p = p + 3;
        }
    }

    image::png::write("value_avx2.png", TEXTURE_SIZE, TEXTURE_SIZE, (unsigned char*) rgb_data, PNG_COLOR_TYPE_RGB);
    free(rgb_data);
    printf("min = %.32f\n", min);
    printf("max = %.32f\n", max);
    printf("NORMALIZATION FACTOR = %.32f\n", glm::min(-1.0 / min, 1.0 / max));

    printf("Refining normalization factor");

    std::mt19937 engine;

    for(int i = 0; i < 1024; ++i)
    {
        for(int j = 0; j < 65536 * 8; ++j)
        {
//            glm::vec4 q = glm::vec4(0.00134512362461 * int(engine()), 0.001391640751 * int(engine()), 0.001391640751 * int(engine()), 0.001391640751 * int(engine()));
//            glm::dvec4 q = glm::dvec4(0.00134512362461 * int(engine()), 0.001391640751 * int(engine()), 0.001391640751 * int(engine()), 0.001391640751 * int(engine()));
//            glm::vec3 q = glm::vec3(0.00134512362461 * int(engine()), 0.001391640751 * int(engine()), 0.001391640751 * int(engine()));
//            glm::dvec4 q = glm::dvec4(0.00134512362461 * int(engine()), 0.001391640751 * int(engine()), 0.001391640751 * int(engine()), 0.001391640751 * int(engine()));
//            glm::vec2 q = glm::vec2(0.00134512362461 * int(engine()), 0.001391640751 * int(engine()));
            glm::dvec2 q = glm::dvec2(0.00134512362461 * int(engine()), 0.001391640751 * int(engine()));
            double t = noise::gradient_sse41<double>(q);
//            float t = noise::gradient<float>(q);

            if (t < min) min = t;
            if (t > max) max = t;

        }
        printf("min = %.32f\n", min);
        printf("max = %.32f\n", max);
        printf("NORMALIZATION FACTOR = %.32f\n\n", glm::min(-1.0 / min, 1.0 / max));

    }

    return 0;

/*
    const int TEXTURE_SIZE = 4096;

    uint8_t* rgb_data = (uint8_t*) malloc(TEXTURE_SIZE * TEXTURE_SIZE * 3);

    uint8_t* p = rgb_data;

    double max = 0.0;
    double min = 0.0;

    double amplitudes[] = {0.5, 0.25, 0.125, 0.00625};

    glm::dmat2 m = glm::dmat2(1.91 * glm::dvec2(0.6, 0.8), 1.91 * glm::dvec2(-0.8, 0.6));
    glm::dmat2 matrices[4] = {m, m * m, m * m * m, m * m * m * m};
    glm::dvec2 shifts[4] = {
                                glm::dvec2(41.0, 93.0), 
                                glm::dvec2(13.0, 54.0), 
                                glm::dvec2(81.0, 15.0), 
                                glm::dvec2(27.0, 45.0)
                           };

    for(int u = 0; u < TEXTURE_SIZE; ++u)
    {
        for(int v = 0; v < TEXTURE_SIZE; ++v)
        {
            glm::dvec2 q = glm::dvec2(-1.5 + 0.0025 * double(u), 1.7 + 0.0025 * double(v));

            double t = noise::gradient_fbm4(q, matrices, shifts, amplitudes);
            double value = 0.5 + 0.5 * t;


            if (t < min) min = t;
            if (t > max) max = t;

            double value_norm = 255.0 * value;
            uint8_t red = (uint8_t) (value_norm); 

            *(p + 0) = red;
            *(p + 1) = red;
            *(p + 2) = red;
            p = p + 3;
        }
    }

    image::png::write("value2d_sse.png", TEXTURE_SIZE, TEXTURE_SIZE, (unsigned char*) rgb_data, PNG_COLOR_TYPE_RGB);
    free(rgb_data);
    printf("min = %.32f\n", min);
    printf("max = %.32f\n", max);
    printf("NORMALIZATION FACTOR = %.32f\n", glm::min(-1.0 / min, 1.0 / max));

    printf("Refining normalization factor");

    std::mt19937 engine;

    for(int i = 0; i < 65536; ++i)
    {
        for(int j = 0; j < 4096; ++j)
        {
            glm::dvec2 q = glm::dvec2(0.001 * int(engine()), 0.001 * int(engine()));

            //double t = noise::simplex_sse41(q);
            double t = noise::gradient_fbm4<double>(q, matrices, shifts, amplitudes);

            if (t < min) min = t;
            if (t > max) max = t;

        }
        printf("min = %.32f\n", min);
        printf("max = %.32f\n", max);
        printf("NORMALIZATION FACTOR = %.32f\n\n", glm::min(-1.0 / min, 1.0 / max));

    }

    return 0;
*/
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Noise", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();
        //===============================================================================================================================================================================================================
        // render scene
        //===============================================================================================================================================================================================================

        //===============================================================================================================================================================================================================
        // done : show back buffer
        //===============================================================================================================================================================================================================
        window.end_frame();
    } 

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;

}


