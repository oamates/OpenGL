//========================================================================================================================================================================================================================
// DEMO 032: Parallax mapping advanced technique
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "log.hpp"
#include "constants.hpp"
#include "imgui_window.hpp"
#include "../../framework/gl_aux.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "../../framework/camera.hpp"
#include "vao.hpp"
#include "vertex.hpp"

#include "gl_aux.hpp"
#include "parallax.hpp"

struct demo_window_t : public imgui_window_t
{
    camera_t camera;

    SceneParallax* scene;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : imgui_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
          camera(16.0f, 0.5f, glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.125f);
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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

        glm::dvec2 ndelta = mouse_delta / glm::dvec2(res_x, res_y);
        bool shiftPressed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
        scene->mouseMoved(ndelta, shiftPressed);
    }

    void on_scroll(double xoffset, double yoffset) override
    {
        float distance = scene->_camera.getDistance();
        distance *= 1.f + 0.02f * yoffset / res_y;
        scene->_camera.setDistance(distance);
    }

    void update_ui() override
    {
        ImGui::SetNextWindowSize(ImVec2(768, 768), ImGuiWindowFlags_NoResize | ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Parallax map demo", 0);
        ImGui::Text("Application average framerate (%.3f FPS)", ImGui::GetIO().Framerate);

        if (ImGui::CollapsingHeader("Display mode"))
        {
            ImGui::RadioButton("FLAT", &scene->_displayMode, SceneParallax::FLAT);
            ImGui::SameLine();
            ImGui::RadioButton("NORMAL", &scene->_displayMode, SceneParallax::NORMAL);
            ImGui::SameLine();
            ImGui::RadioButton("PARALLAX", &scene->_displayMode, SceneParallax::PARALLAX);
        }

        ImGui::SliderFloat("Amplitude", &scene->_amplitude, 0.25f * 0.03125f, 0.25f, "%.3f");
        ImGui::SliderInt("Layers", &scene->_nbLayers, 1, 32);

        ImGui::Checkbox("Interpolation", &scene->_interpolation);
        ImGui::Checkbox("Self-shadowing", &scene->_selfShadow);
        ImGui::Checkbox("Cropping", &scene->_crop);
        ImGui::Checkbox("Specular Mapping", &scene->_specularMapping);

        ImGui::End();
    }

};

int main(int argc, char *argv[])
{
    const int res_x = 1920;
    const int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW ImGui window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Parallax mapping advanced", 4, 3, 3, res_x, res_y, true);

    SceneParallax scene(res_x, res_y);
    window.scene = &scene;

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();

        glClearColor(0.04f, 0.09f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene.render();

        //===============================================================================================================================================================================================================
        // After end_frame call ::
        //  - GL_DEPTH_TEST is disabled
        //  - GL_CULL_FACE is disabled
        //  - GL_SCISSOR_TEST is enabled
        //  - GL_BLEND is enabled -- blending mode GL_SRC_ALPHA/GL_ONE_MINUS_SRC_ALPHA with blending function GL_FUNC_ADD
        //  - VAO binding is destroyed
        //===============================================================================================================================================================================================================
        window.end_frame();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_BLEND);
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}


