#ifndef _glfw_window_included_4283678947635786273465286034868743624563575676554
#define _glfw_window_included_4283678947635786273465286034868743624563575676554

//=======================================================================================================================================================================================================================
// GLFW based application window structure
//=======================================================================================================================================================================================================================
#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace glfw
{
    //===================================================================================================================================================================================================================
    // static, window-independent functions from GLFW3 library
    //===================================================================================================================================================================================================================
    int init();
    double time();
    void terminate();
    void poll_events();

    //===================================================================================================================================================================================================================
    // static, window-independent GLFW3 event callback functions
    //===================================================================================================================================================================================================================
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    void mouse_move_callback(GLFWwindow* window, double x, double y);
    void cursor_enter_callback(GLFWwindow* window, int enter);
    void character_callback(GLFWwindow* window, unsigned int codepoint);
    void scroll_callback(GLFWwindow* window, double x, double y);

} // namespace glfw

void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

struct glfw_window_t
{
    //===================================================================================================================================================================================================================
    // data members : resolution, window handle, mouse cursor position, mouse delta since last measurement
    //===================================================================================================================================================================================================================
    int64_t frame;
    int res_x, res_y;
    GLFWwindow* window;

    double frame_ts, mouse_ts;
    double frame_dt, mouse_dt;

    glm::dvec2 mouse;
    glm::dvec2 mouse_delta;

    //===================================================================================================================================================================================================================
    // GLFW window constructor + GLEW library initialization
    // the application will be terminated if one of the library fails to initialize or 
    // requested context version is not supported by OpenGL driver
    //===================================================================================================================================================================================================================
    glfw_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen, bool debug_context = false);
    virtual ~glfw_window_t();

    void new_frame();
    void end_frame();

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================
    virtual void on_key(int key, int scancode, int action, int mods) {};
    virtual void on_mouse_button(int button, int action, int mods) {};
    virtual void on_mouse_move() {};
    virtual void on_cursor_enter(int enter) {};
    virtual void on_character(unsigned int codepoint) {};
    virtual void on_scroll(double xoffset, double yoffset) {};
    virtual void on_resize(int width, int height) {};

    //===================================================================================================================================================================================================================
    // window-dependent interface functions of GLFW
    //===================================================================================================================================================================================================================
    void set_title(const char* title);
    void set_should_close(int value);
    void swap_buffers();
    bool should_close();
    double aspect();
    double inv_aspect();
    void disable_cursor();
    void enable_cursor();
    int key_state(int key);

    glm::dvec2 cursor_position();

};

#endif // _glfw_window_included_4283678947635786273465286034868743624563575676554