//=======================================================================================================================================================================================================================
// DEMO 049 : GLFW + OpenGL 3.3 + ImGui Example
//=======================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "imgui_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "glsl_noise.hpp"
#include "image.hpp"

struct demo_window_t : public imgui_window_t
{
    camera_t camera;

    bool hell = true;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : imgui_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
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

        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE)) hell = !hell;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("GLFW + OpenGL 3.3 + ImGui Example", 4, 3, 3, 1920, 1080);


    //===================================================================================================================================================================================================================
    // Shader and uniform variables initialization
    //===================================================================================================================================================================================================================
    glsl_program_t ray_marcher(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ray_marcher.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/canyon.fs"));
    ray_marcher.enable();
    uniform_t uniform_camera_matrix = ray_marcher["camera_matrix"];
    uniform_t uniform_time = ray_marcher["time"];
    uniform_t uniform_hell = ray_marcher["hell"];

    glm::vec2 focal_scale = glm::vec2(1.0f / window.camera.projection_matrix[0][0], 1.0f / window.camera.projection_matrix[1][1]);
    ray_marcher["focal_scale"] = focal_scale;
    ray_marcher["value_texture"] = 1;
    ray_marcher["stone_texture"] = 2;

    //===================================================================================================================================================================================================================
    // Load noise textures - for very fast 2d noise calculation
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE1);
    GLuint noise_tex = glsl_noise::randomRGBA_shift_tex256x256(glm::ivec2(37, 17));
    glActiveTexture(GL_TEXTURE2);
    GLuint stone_tex = image::png::texture2d("../../../resources/tex2d/clay2.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);   

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    //===================================================================================================================================================================================================================
    // GUI data
    //===================================================================================================================================================================================================================
    bool show_test_window = true;
    bool show_another_window = true;
    float slider_value = 0.0f;
    ImVec4 clear_color = ImColor(114, 144, 154);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        //===============================================================================================================================================================================================================
        // clear back color buffer, process events and update timer
        //===============================================================================================================================================================================================================
        glClear(GL_COLOR_BUFFER_BIT);
        window.new_frame();

        //===============================================================================================================================================================================================================
        // 1. Show a simple window. If we don't call ImGui::Begin() / ImGui::End() the widgets appears in a window automatically called "Debug"
        //===============================================================================================================================================================================================================
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &slider_value, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color", (float*) &clear_color);
        if (ImGui::Button("Test Window")) show_test_window ^= 1;
        if (ImGui::Button("Another Window")) show_another_window ^= 1;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        //===============================================================================================================================================================================================================
        // 2. Show another simple window, this time using an explicit Begin/End pair
        //===============================================================================================================================================================================================================
        if (show_another_window)
        {
            ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello");
            ImGui::End();
        }

        //===============================================================================================================================================================================================================
        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        //===============================================================================================================================================================================================================
        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
            ImGui::ShowTestWindow(&show_test_window);
        }

        //===============================================================================================================================================================================================================
        // 4. Rendering scene
        //===============================================================================================================================================================================================================
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        ray_marcher.enable();
        glBindVertexArray(vao_id);
        glm::mat4 camera_matrix = glm::inverse(window.camera.view_matrix);
        uniform_time = window.frame_ts;
        uniform_camera_matrix = camera_matrix;
        uniform_hell = (int) window.hell;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        window.end_frame();
    }

    glfw::terminate();
    return 0;
}
