//========================================================================================================================================================================================================================
// DEMO 032: Image Filtering with GL_COMPUTE_SHADER
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "imgui_window.hpp"
#include "gl_info.hpp"
#include "shader.hpp"
#include "image.hpp"

struct subroutine_t
{
    const char* name;
    const char* description;
    GLuint index;
    subroutine_t(const char* name, const char* description) : name(name), description(description) {}
};

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW ImGui window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    imgui_window_t window("Image Filtering", 4, 4, 3, 1920, 1080, true, true);
    gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO | OPENGL_COMPUTE_SHADER_INFO);

    //===================================================================================================================================================================================================================
    // compute shader compilation and subroutine indices querying
    //===================================================================================================================================================================================================================
    glsl_program_t image_filter(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/image_filter.cs"));
    image_filter.enable();
    uniform_t uni_if_time = image_filter["time"];

    subroutine_t subroutines[] = 
    {
        subroutine_t("dithering_rgb",                   "\tRGB dithering using Monte-Carlo method. 8x16x8 colors."),                   
        subroutine_t("dithering_luminance",             "\tLuminance dithering using Monte-Carlo method. 17 shades of grey."),             
        subroutine_t("constant_hue_filter",             "\tHue component is set to a constant value."),             
        subroutine_t("sobel_edge_detector",             "\tConvolution with kernels:\n\t\t|-1.0 0.0 1.0|\t| 1.0  2.0  1.0|"                                                    
                                                                                   "\n\t\t|-2.0 0.0 2.0|\t| 0.0  0.0  0.0|"                                                    
                                                                                   "\n\t\t|-1.0 0.0 1.0|\t|-1.0 -2.0 -1.0|"                                                    
                                                        "\nis used to estimate the gradient of the luminosity component of the input.\nThe output is its weighted magnitude."),
        subroutine_t("scharr_edge_detector_hue",        "\tConvolution with kernels:\n\t\t| -3.0 0.0  3.0|\t| 3.0  10.0  3.0|"
                                                                                   "\n\t\t|-10.0 0.0 10.0|\t| 0.0   0.0  0.0|"
                                                                                   "\n\t\t| -3.0 0.0  3.0|\t|-3.0 -10.0 -3.0|"
                                                        "\nis used to estimate the gradient of the hue component of the input.\nThe output is its weighted magnitude."),
        subroutine_t("scharr_edge_detector_saturation", "\tConvolution with kernels:\n\t\t| -3.0 0.0  3.0|\t| 3.0  10.0  3.0|"                                                    
                                                                                   "\n\t\t|-10.0 0.0 10.0|\t| 0.0   0.0  0.0|"                                                    
                                                                                   "\n\t\t| -3.0 0.0  3.0|\t|-3.0 -10.0 -3.0|"                                                    
                                                        "\nis used to estimate the gradient of the saturation component of the input.\nThe output is its weighted magnitude."),
        subroutine_t("scharr_edge_detector_value",      "\tConvolution with kernels:\n\t\t| -3.0 0.0  3.0|\t| 3.0  10.0  3.0|"                                                           
                                                                                   "\n\t\t|-10.0 0.0 10.0|\t| 0.0   0.0  0.0|"                                                           
                                                                                   "\n\t\t| -3.0 0.0  3.0|\t|-3.0 -10.0 -3.0|"                                                           
                                                        "\nis used to estimate the gradient of the value component of the input.\nThe output is its weighted magnitude."),
        subroutine_t("scharr_edge_detector_luminosity", "\tConvolution with kernels:\n\t\t| -3.0 0.0  3.0|\t| 3.0  10.0  3.0|"                                                           
                                                                                   "\n\t\t|-10.0 0.0 10.0|\t| 0.0   0.0  0.0|"                                                           
                                                                                   "\n\t\t| -3.0 0.0  3.0|\t|-3.0 -10.0 -3.0|"                                                           
                                                        "\nis used to estimate the gradient of the luminosity component of the input.\nThe output is its weighted magnitude."),
        subroutine_t("gaussian_blur5x5_rgb",            "\tConvolution with 5x5 symmetric kernel:\n\t\t|1.0  4.0  7.0  4.0 1.0|"
                                                                                                "\n\t\t|4.0 16.0 26.0 16.0 4.0|"
                                                                                                "\n\t\t|7.0 26.0 41.0 26.0 7.0|"
                                                                                                "\n\t\t|4.0 16.0 26.0 16.0 4.0|"
                                                                                                "\n\t\t|1.0  4.0  7.0  4.0 1.0|"
                                                        "\nacting on rgb components of the input."),
        subroutine_t("gaussian_blur5x5_hsv",            "\tConvolution with 5x5 symmetric kernel:\n\t\t|1.0  4.0  7.0  4.0 1.0|"
                                                                                                "\n\t\t|4.0 16.0 26.0 16.0 4.0|"
                                                                                                "\n\t\t|7.0 26.0 41.0 26.0 7.0|"
                                                                                                "\n\t\t|4.0 16.0 26.0 16.0 4.0|"
                                                                                                "\n\t\t|1.0  4.0  7.0  4.0 1.0|"
                                                        "\nacting on hsv components of the input."),
        subroutine_t("bump_filter",                     "\tFilter produces normalmap from the input texture 1"),
        subroutine_t("bump_filter_luminosity",          "\tFilter produces normalmap from the input texture 2"),
        subroutine_t("laplace_sharpening",              "\tLaplace sharpening filter:")

    };

    
    const GLuint SUBROUTINE_COUNT = sizeof(subroutines) / sizeof(subroutine_t);

    for (int i = 0; i < SUBROUTINE_COUNT; ++i)
        subroutines[i].index = image_filter.subroutine_index(GL_COMPUTE_SHADER, subroutines[i].name);

    uniform_t::subroutine(GL_COMPUTE_SHADER, &subroutines[0].index);

    //===================================================================================================================================================================================================================
    // quad rendering shader
    //===================================================================================================================================================================================================================
    glsl_program_t quad_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/quad.fs"));
    quad_renderer.enable();
    uniform_t uniform_quad = quad_renderer["quad"];
    uniform_t uniform_teximage = quad_renderer["teximage"];

    //===================================================================================================================================================================================================================
    // create output texture that the compute program will write into
    // note :: input is converted to floating point texture (if not, correct internal format in the shader must be specified)
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE1);
    GLuint input_image = image::png::texture2d("../../../resources/tex2d/room.png", 0, GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, true);

    GLuint output_image;
    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &output_image);
    glBindTexture(GL_TEXTURE_2D, output_image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 1024, 1024);

    //===================================================================================================================================================================================================================
    // create fake VAO for rendering quads
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    //===================================================================================================================================================================================================================
    // Global GL settings :
    // DEPTH not needed, hence disabled
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glDisable(GL_DEPTH_TEST);

    //===================================================================================================================================================================================================================
    // UI variables
    //===================================================================================================================================================================================================================
    bool show_test_window = true;
    bool show_another_window = true;

    bool header_opened[SUBROUTINE_COUNT];
    for (int i = 0; i < SUBROUTINE_COUNT; ++i)
        header_opened[SUBROUTINE_COUNT] = false;

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        image_filter.enable();
        uni_if_time = (float) window.frame_ts;
        glBindImageTexture(0,  input_image, 0, GL_FALSE, 0,  GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, output_image, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glDispatchCompute(1024 >> 5, 1024 >> 5, 1);

        //===============================================================================================================================================================================================================
        // after compute shader is loaded, show and process the ui :: show options window
        //===============================================================================================================================================================================================================
        ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Image filtering demo using Compute Shaders", &show_another_window);
        
        for (int i = 0; i < SUBROUTINE_COUNT; ++i)
        {
            if (ImGui::CollapsingHeader(subroutines[i].name))
            {
                if (!header_opened[i])
                    uniform_t::subroutine(GL_COMPUTE_SHADER, &subroutines[i].index);
                ImGui::Text(subroutines[i].description);
                header_opened[i] = true;
            }
            else    
                header_opened[i] = false;
        }

        ImGui::End();

        //===============================================================================================================================================================================================================
        // show ImGui test window
        //===============================================================================================================================================================================================================
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
        ImGui::ShowTestWindow(&show_test_window);

        //===============================================================================================================================================================================================================
        // wait for the compute shader to finish its work and show both original and processed image
        //===============================================================================================================================================================================================================
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        quad_renderer.enable();
        glBindVertexArray(vao_id);

        const float margin = 0.125;
        float aspect = window.aspect();

        uniform_quad = glm::vec4(-1.0f, -0.5f * aspect, 0.0f, 0.5f * aspect) + margin * glm::vec4(1.0f, aspect, -1.0f, -aspect);
        uniform_teximage = 1;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        uniform_quad = glm::vec4( 0.0f, -0.5f * aspect, 1.0f, 0.5f * aspect) + margin * glm::vec4(1.0f, aspect, -1.0f, -aspect);
        uniform_teximage = 2;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        window.end_frame();
    }

    glDeleteTextures(1, &input_image);
    glDeleteTextures(1, &output_image);
    glDeleteVertexArrays(1, &vao_id);    

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}