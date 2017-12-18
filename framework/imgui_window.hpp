#ifndef _imgui_window_included_823715602756273652756108725634563735612873601239
#define _imgui_window_included_823715602756273652756108725634563735612873601239

//=======================================================================================================================================================================================================================
// GLFW3 + ImGui based application window structure
//=======================================================================================================================================================================================================================
#include <glm/glm.hpp>

#include "imgui/imgui.h"
#include "glfw_window.hpp"

namespace glfw
{
    //===================================================================================================================================================================================================================
    // static, window-independent GLFW3 + ImGui event callback functions
    //===================================================================================================================================================================================================================
    void imgui_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void imgui_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    void imgui_mouse_move_callback(GLFWwindow* window, double x, double y);
    void imgui_cursor_enter_callback(GLFWwindow* window, int enter);
    void imgui_character_callback(GLFWwindow* window, unsigned int codepoint);
    void imgui_scroll_callback(GLFWwindow* window, double x, double y);

} // namespace glfw

struct imgui_window_t
{
    //===================================================================================================================================================================================================================
    // data members : resolution, window handle, mouse cursor position, mouse delta since last measurement, mouse wheel and mouse buttons state
    //===================================================================================================================================================================================================================
    GLFWwindow* window;
    bool imgui_active;

    int64_t frame;
    int res_x, res_y;

    double initial_ts;
    double frame_ts, mouse_ts;
    double frame_dt, mouse_dt;

    glm::dvec2 mouse;
    glm::dvec2 mouse_delta;

    double mouse_wheel;
    bool mouse_pressed[3];

    //===================================================================================================================================================================================================================
    // UI render data
    //===================================================================================================================================================================================================================
    GLint texture_unit_max_index;
    GLint ui_program_id, ui_vs_id, ui_fs_id;
    GLint uni_projection_matrix_id;
    GLuint ui_vao_id, ui_vbo_id, ui_ibo_id;
    GLuint ui_font_tex_id;

    //===================================================================================================================================================================================================================
    // GLFW window constructor + GLEW library initialization
    // the application will be terminated if one of the library fails to initialize or
    // requested context version is not supported by OpenGL driver
    //===================================================================================================================================================================================================================
    imgui_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen, bool debug_context = false);
    virtual ~imgui_window_t();

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
    virtual void update_ui() {};

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
    double fps();
};

#endif // _imgui_window_included_823715602756273652756108725634563735612873601239