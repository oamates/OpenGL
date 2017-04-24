//========================================================================================================================================================================================================================
// DEMO 053: Fast Fourier Transform
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

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
#include "fft.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
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

const unsigned int n = 1024;
const unsigned int m = 10;

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

    demo_window_t window("FFT", 8, 4, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // FFT compute shader :: uses texture buffer as input, _h - shader does row transformation
    //                                                     _v - shader does column transformation
    //===================================================================================================================================================================================================================
    glsl_program_t fft4_img_h(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/fft4_tb_h.cs"));
    fft4_img_h.enable();
    uniform_t uniform_ftt_time = fft4_img_h["time"];

    glsl_program_t fft4_img_v(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/fft4_tb_v.cs"));

    //===================================================================================================================================================================================================================
    // render result
    //===================================================================================================================================================================================================================
    glsl_program_t render(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/render.vs"),
                          glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/render.fs"));
    render.enable();
    uniform_t uniform_projection_view_matrix = render["projection_view_matrix"];
    uniform_t uniform_time                   = render["time"];

    //===================================================================================================================================================================================================================
    // input buffer initialization 
    //===================================================================================================================================================================================================================
    GLuint size = n * n;
    GLuint input_buffer, output_buffer;

    glGenBuffers(1, &input_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, input_buffer);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(glm::vec2), 0, GL_DYNAMIC_COPY);
    glm::vec2* fourier_coeff = (glm::vec2*) glMapBufferRange(GL_ARRAY_BUFFER, 0, size * sizeof(glm::vec2), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);


    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            glm::vec2 dir = glm::vec2(i, j) - glm::vec2(0.5 * (n - 1));
            float d = glm::length(dir);
            float b = glm::dot(glm::normalize(dir), glm::vec2(0.8, 0.6));

            float a = 7.4 * glm::exp(-0.28 * d);
            glm::vec2 q = a * b * b * b * b * glm::vec2(glm::gaussRand(0.0f, 16.0f), glm::gaussRand(0.0f, 16.0f));
            fourier_coeff[n * j + i] = q;
        }
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);

    //===================================================================================================================================================================================================================
    // output buffer initialization 
    //===================================================================================================================================================================================================================
    GLuint vao_id, ibo_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glGenBuffers(1, &output_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, output_buffer);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(glm::vec2), 0, GL_DYNAMIC_COPY);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    GLuint I = (n - 1) * (2 * n + 1);
    GLuint element_buffer_size = I * sizeof(unsigned int);


    glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, 0, GL_STATIC_DRAW);

    unsigned int* indices = (unsigned int*) glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, element_buffer_size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    unsigned int idx = 0;
    unsigned int i = 0;
    for (int q = 0; q < n - 1; ++q)
    {
        for (int p = 0; p < n; ++p)
        {
            indices[idx++] = i;             
            indices[idx++] = i + n;         
            ++i;
        }
        indices[idx++] = -1;
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);


    GLuint input_tbo, output_tbo;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &input_tbo);
    glBindTexture(GL_TEXTURE_BUFFER, input_tbo);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, input_buffer);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &output_tbo);
    glBindTexture(GL_TEXTURE_BUFFER, output_tbo);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, output_buffer);

    glBindImageTexture(0, input_tbo, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG32F);
    glBindImageTexture(1, output_tbo, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG32F);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST disabled
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.0f, 0.05f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(-1);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        float time = window.frame_ts;

        fft4_img_h.enable();
        uniform_ftt_time = (float) time;
        glDispatchCompute(1, n, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        fft4_img_v.enable();
        glDispatchCompute(n, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        render.enable();
        glBindVertexArray(vao_id);
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        uniform_projection_view_matrix = projection_view_matrix;
        uniform_time = time;

        glDrawElements(GL_TRIANGLE_STRIP, I, GL_UNSIGNED_INT, 0);

        window.end_frame();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}