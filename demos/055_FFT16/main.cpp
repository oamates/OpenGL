//========================================================================================================================================================================================================================
// DEMO 055: Fast Fourier Transform
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

const unsigned int n = 2048;

/*
const int br512[512] = {0x000, 0x100, 0x080, 0x180, 0x040, 0x140, 0x0C0, 0x1C0, 0x020, 0x120, 0x0A0, 0x1A0, 0x060, 0x160, 0x0E0, 0x1E0, 
                        0x010, 0x110, 0x090, 0x190, 0x050, 0x150, 0x0D0, 0x1D0, 0x030, 0x130, 0x0B0, 0x1B0, 0x070, 0x170, 0x0F0, 0x1F0, 
                        0x008, 0x108, 0x088, 0x188, 0x048, 0x148, 0x0C8, 0x1C8, 0x028, 0x128, 0x0A8, 0x1A8, 0x068, 0x168, 0x0E8, 0x1E8, 
                        0x018, 0x118, 0x098, 0x198, 0x058, 0x158, 0x0D8, 0x1D8, 0x038, 0x138, 0x0B8, 0x1B8, 0x078, 0x178, 0x0F8, 0x1F8, 
                        0x004, 0x104, 0x084, 0x184, 0x044, 0x144, 0x0C4, 0x1C4, 0x024, 0x124, 0x0A4, 0x1A4, 0x064, 0x164, 0x0E4, 0x1E4, 
                        0x014, 0x114, 0x094, 0x194, 0x054, 0x154, 0x0D4, 0x1D4, 0x034, 0x134, 0x0B4, 0x1B4, 0x074, 0x174, 0x0F4, 0x1F4, 
                        0x00C, 0x10C, 0x08C, 0x18C, 0x04C, 0x14C, 0x0CC, 0x1CC, 0x02C, 0x12C, 0x0AC, 0x1AC, 0x06C, 0x16C, 0x0EC, 0x1EC, 
                        0x01C, 0x11C, 0x09C, 0x19C, 0x05C, 0x15C, 0x0DC, 0x1DC, 0x03C, 0x13C, 0x0BC, 0x1BC, 0x07C, 0x17C, 0x0FC, 0x1FC, 
                        0x002, 0x102, 0x082, 0x182, 0x042, 0x142, 0x0C2, 0x1C2, 0x022, 0x122, 0x0A2, 0x1A2, 0x062, 0x162, 0x0E2, 0x1E2,
                        0x012, 0x112, 0x092, 0x192, 0x052, 0x152, 0x0D2, 0x1D2, 0x032, 0x132, 0x0B2, 0x1B2, 0x072, 0x172, 0x0F2, 0x1F2,
                        0x00A, 0x10A, 0x08A, 0x18A, 0x04A, 0x14A, 0x0CA, 0x1CA, 0x02A, 0x12A, 0x0AA, 0x1AA, 0x06A, 0x16A, 0x0EA, 0x1EA,
                        0x01A, 0x11A, 0x09A, 0x19A, 0x05A, 0x15A, 0x0DA, 0x1DA, 0x03A, 0x13A, 0x0BA, 0x1BA, 0x07A, 0x17A, 0x0FA, 0x1FA,
                        0x006, 0x106, 0x086, 0x186, 0x046, 0x146, 0x0C6, 0x1C6, 0x026, 0x126, 0x0A6, 0x1A6, 0x066, 0x166, 0x0E6, 0x1E6,
                        0x016, 0x116, 0x096, 0x196, 0x056, 0x156, 0x0D6, 0x1D6, 0x036, 0x136, 0x0B6, 0x1B6, 0x076, 0x176, 0x0F6, 0x1F6,
                        0x00E, 0x10E, 0x08E, 0x18E, 0x04E, 0x14E, 0x0CE, 0x1CE, 0x02E, 0x12E, 0x0AE, 0x1AE, 0x06E, 0x16E, 0x0EE, 0x1EE,
                        0x01E, 0x11E, 0x09E, 0x19E, 0x05E, 0x15E, 0x0DE, 0x1DE, 0x03E, 0x13E, 0x0BE, 0x1BE, 0x07E, 0x17E, 0x0FE, 0x1FE,
                        0x001, 0x101, 0x081, 0x181, 0x041, 0x141, 0x0C1, 0x1C1, 0x021, 0x121, 0x0A1, 0x1A1, 0x061, 0x161, 0x0E1, 0x1E1,
                        0x011, 0x111, 0x091, 0x191, 0x051, 0x151, 0x0D1, 0x1D1, 0x031, 0x131, 0x0B1, 0x1B1, 0x071, 0x171, 0x0F1, 0x1F1,
                        0x009, 0x109, 0x089, 0x189, 0x049, 0x149, 0x0C9, 0x1C9, 0x029, 0x129, 0x0A9, 0x1A9, 0x069, 0x169, 0x0E9, 0x1E9,
                        0x019, 0x119, 0x099, 0x199, 0x059, 0x159, 0x0D9, 0x1D9, 0x039, 0x139, 0x0B9, 0x1B9, 0x079, 0x179, 0x0F9, 0x1F9,
                        0x005, 0x105, 0x085, 0x185, 0x045, 0x145, 0x0C5, 0x1C5, 0x025, 0x125, 0x0A5, 0x1A5, 0x065, 0x165, 0x0E5, 0x1E5,
                        0x015, 0x115, 0x095, 0x195, 0x055, 0x155, 0x0D5, 0x1D5, 0x035, 0x135, 0x0B5, 0x1B5, 0x075, 0x175, 0x0F5, 0x1F5,
                        0x00D, 0x10D, 0x08D, 0x18D, 0x04D, 0x14D, 0x0CD, 0x1CD, 0x02D, 0x12D, 0x0AD, 0x1AD, 0x06D, 0x16D, 0x0ED, 0x1ED,
                        0x01D, 0x11D, 0x09D, 0x19D, 0x05D, 0x15D, 0x0DD, 0x1DD, 0x03D, 0x13D, 0x0BD, 0x1BD, 0x07D, 0x17D, 0x0FD, 0x1FD,
                        0x003, 0x103, 0x083, 0x183, 0x043, 0x143, 0x0C3, 0x1C3, 0x023, 0x123, 0x0A3, 0x1A3, 0x063, 0x163, 0x0E3, 0x1E3,
                        0x013, 0x113, 0x093, 0x193, 0x053, 0x153, 0x0D3, 0x1D3, 0x033, 0x133, 0x0B3, 0x1B3, 0x073, 0x173, 0x0F3, 0x1F3,
                        0x00B, 0x10B, 0x08B, 0x18B, 0x04B, 0x14B, 0x0CB, 0x1CB, 0x02B, 0x12B, 0x0AB, 0x1AB, 0x06B, 0x16B, 0x0EB, 0x1EB,
                        0x01B, 0x11B, 0x09B, 0x19B, 0x05B, 0x15B, 0x0DB, 0x1DB, 0x03B, 0x13B, 0x0BB, 0x1BB, 0x07B, 0x17B, 0x0FB, 0x1FB,
                        0x007, 0x107, 0x087, 0x187, 0x047, 0x147, 0x0C7, 0x1C7, 0x027, 0x127, 0x0A7, 0x1A7, 0x067, 0x167, 0x0E7, 0x1E7,
                        0x017, 0x117, 0x097, 0x197, 0x057, 0x157, 0x0D7, 0x1D7, 0x037, 0x137, 0x0B7, 0x1B7, 0x077, 0x177, 0x0F7, 0x1F7,
                        0x00F, 0x10F, 0x08F, 0x18F, 0x04F, 0x14F, 0x0CF, 0x1CF, 0x02F, 0x12F, 0x0AF, 0x1AF, 0x06F, 0x16F, 0x0EF, 0x1EF,
                        0x01F, 0x11F, 0x09F, 0x19F, 0x05F, 0x15F, 0x0DF, 0x1DF, 0x03F, 0x13F, 0x0BF, 0x1BF, 0x07F, 0x17F, 0x0FF, 0x1FF}; */


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
    glsl_program_t fft8_img_h(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/fft16_tb_h.cs"));
    fft8_img_h.enable();
    uniform_t uniform_ftt_time = fft8_img_h["time"];

    glsl_program_t fft8_img_v(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/fft16_tb_v.cs"));

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

            float a = 1.4 * glm::exp(-0.042478 * d);
            glm::vec2 q = a * glm::vec2(glm::gaussRand(0.0f, 16.0f), glm::gaussRand(0.0f, 16.0f));
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

    unsigned int* indices = (unsigned int*) glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

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

    //glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &input_tbo);
    glBindTexture(GL_TEXTURE_BUFFER, input_tbo);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, input_buffer);

    //glActiveTexture(GL_TEXTURE0);
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
//    glEnable(GL_CULL_FACE);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(-1);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        double time = window.frame_ts;

        fft8_img_h.enable();
        uniform_ftt_time = (float) time;
        glDispatchCompute(1, n, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
     
        fft8_img_v.enable();
        glDispatchCompute(n, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render.enable();
        glBindVertexArray(vao_id);
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        uniform_projection_view_matrix = projection_view_matrix;
        uniform_time = (float) time;

        glDrawElements(GL_TRIANGLE_STRIP, I, GL_UNSIGNED_INT, 0);

        window.end_frame();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}