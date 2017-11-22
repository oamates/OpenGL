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
#include "glfw_window.hpp"
#include "../../framework/gl_aux.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "../../framework/camera.hpp"
#include "vao.hpp"
#include "vertex.hpp"

#include "gl_aux.hpp"
#include "parallax.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    SceneParallax* scene;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
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

        if (action != GLFW_RELEASE) return;

        if (key == GLFW_KEY_D)
            scene->_displayMode = (scene->_displayMode + 1) % 3;
        if (key == GLFW_KEY_A)
            scene->_amplitude += 0.01f;
        if (key == GLFW_KEY_Z)
            scene->_amplitude = std::max(0.f, scene->_amplitude - 0.01f);
        if (key == GLFW_KEY_L)
            scene->_nbLayers++;
        if (key == GLFW_KEY_M)
            scene->_nbLayers = (scene->_nbLayers == 1) ? 1 : scene->_nbLayers - 1;
        if (key == GLFW_KEY_I)
            scene->_interpolation = !scene->_interpolation;
        if (key == GLFW_KEY_S)
            scene->_selfShadow = !scene->_selfShadow;
        if (key == GLFW_KEY_C)
            scene->_crop = !scene->_crop;
        if (key == GLFW_KEY_V)
            scene->_specularMapping = !scene->_specularMapping;
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

    void on_scroll(double xoffset, double yoffset)
    {
        float distance = scene->_camera.getDistance();
        distance *= 1.f + 0.02f * yoffset / res_y;
        scene->_camera.setDistance(distance);
    }
};

/*
sf::Vector2f getRelativeMouseCoords(sf::Window const& window)
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

    sf::Vector2f pos(mousePos.x, window.getSize().y - mousePos.y);
    pos.x /= static_cast<float>(window.getSize().x);
    pos.y /= static_cast<float>(window.getSize().y);
    return pos;
}

bool isMouseInWindow(sf::Window const& window)
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    return (mousePos.x >= 0) && (mousePos.y >= 0) &&
           (mousePos.x < window.getSize().x) && (mousePos.y < window.getSize().y);
}
*/
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

    /* Font loading */
    /*
    sf::Font font;
    if (!font.loadFromFile("fonts/font.ttf")) {
        std::cerr << "Warning: unable to load fonts/font.ttf." << std::endl;
    }
    sf::Text text("", font, 18);
    text.setColor(sf::Color::White);
    */

    /* Scene loading */
    SceneParallax scene(res_x, res_y);
    window.scene = &scene;

    //    sf::Vector2f mousePos = getRelativeMouseCoords(window);
    unsigned int loop = 0;

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();

        glViewport(0, 0, res_x, res_y);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*
        sf::Event event;
        while (window.pollEvent(event)) {
            scene.handleEvents(event);

        }


        */

        /* Drawing */
        //fpsClock.restart();
        scene.render();

        /* For displaying text, SMFL used old OpenGL */
        //float fps = 1.0f / fpsClock.getElapsedTime().asSeconds();
        //text.setString(scene.getDisplayText(fps));

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}


