//=======================================================================================================================================================================================================================
// GLFW based application window structure
//=======================================================================================================================================================================================================================
#include <thread> 
#include <GL/glew.h> 

#include "glfw_window.hpp"
#include "log.hpp"
#include "image.hpp"

//=======================================================================================================================================================================================================================
// static, window-independent functions from GLFW3 library
//=======================================================================================================================================================================================================================

namespace glfw {

int init()
    { return glfwInit(); }

double time()
    { return glfwGetTime(); }

void terminate()
    { glfwTerminate(); }

void poll_events()
    { glfwPollEvents(); }

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{   
    glfw_window_t* glfw_window = static_cast<glfw_window_t*> (glfwGetWindowUserPointer(window));

    //===================================================================================================================================================================================================================
    // screenshot maker data
    //===================================================================================================================================================================================================================
    static char file_name[20] = {"screenshot/0000.png"};
    static char hex_digit[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    static uint8_t* pixel_buffer = 0;
    static std::thread screenshot_thread;
    static unsigned short shot = 0;    

    if ((key == GLFW_KEY_PRINT_SCREEN) && (action == GLFW_RELEASE))
    {
        debug_msg("Saving screenshot #%d: %s.", shot, file_name);

        int res_x = glfw_window->res_x; 
        int res_y = glfw_window->res_y; 

        if (pixel_buffer) 
            screenshot_thread.join();
        else
            pixel_buffer = (unsigned char *) malloc (res_x * res_y * 3);

        glReadPixels(0, 0, res_x, res_y, GL_RGB, GL_UNSIGNED_BYTE, pixel_buffer);
        screenshot_thread = std::thread(image::png::write, file_name, res_x, res_y, pixel_buffer, PNG_COLOR_TYPE_RGB);

        ++shot;
        file_name[11] = hex_digit[(shot & 0xF000) >> 0xC];
        file_name[12] = hex_digit[(shot & 0x0F00) >> 0x8];
        file_name[13] = hex_digit[(shot & 0x00F0) >> 0x4];
        file_name[14] = hex_digit[(shot & 0x000F) >> 0x0];
    };

    glfw_window->on_key(key, scancode, action, mods);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    glfw_window_t* glfw_window = static_cast<glfw_window_t*> (glfwGetWindowUserPointer(window));
    glfw_window->on_mouse_button(button, action, mods);
}

void mouse_move_callback(GLFWwindow* window, double x, double y)
{
    double t = glfw::time();
    glfw_window_t* glfw_window = static_cast<glfw_window_t*> (glfwGetWindowUserPointer(window));
    glm::dvec2 mouse_np = glm::dvec2(x, y);
    glfw_window->mouse_delta = mouse_np - glfw_window->mouse;
    glfw_window->mouse = mouse_np;
    glfw_window->mouse_dt = t - glfw_window->mouse_ts;
    glfw_window->mouse_ts = t;
    glfw_window->on_mouse_move();    
}

void cursor_enter_callback(GLFWwindow* window, int enter)
{
    glfw_window_t* glfw_window = static_cast<glfw_window_t*> (glfwGetWindowUserPointer(window));
    glfw_window->on_cursor_enter(enter);
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
    glfw_window_t* glfw_window = static_cast<glfw_window_t*> (glfwGetWindowUserPointer(window));
    glfw_window->on_character(codepoint);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    glfw_window_t* glfw_window = static_cast<glfw_window_t*> (glfwGetWindowUserPointer(window));
    glfw_window->on_scroll(xoffset, yoffset);
}

void resize_callback(GLFWwindow* window, int width, int height) 
{ 
    glfw_window_t* glfw_window = static_cast<glfw_window_t*> (glfwGetWindowUserPointer(window));
    glfw_window->on_resize(width, height);
}

} // namespace glfw

void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    (void) length;

    char *source_desc, *type_desc, *severity_desc;

    switch (source) 
    {
        case GL_DEBUG_SOURCE_API             : source_desc = (char*) "API";             break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM   : source_desc = (char*) "WINDOW_SYSTEM";   break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER : source_desc = (char*) "SHADER_COMPILER"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY     : source_desc = (char*) "THIRD_PARTY";     break;
        case GL_DEBUG_SOURCE_APPLICATION     : source_desc = (char*) "APPLICATION";     break;
        default                              : source_desc = (char*) "OTHER";
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR               : type_desc = (char*) "ERROR";               break; 
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR : type_desc = (char*) "DEPRECATED_BEHAVIOR"; break; 
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  : type_desc = (char*) "UNDEFINED_BEHAVIOR";  break; 
        case GL_DEBUG_TYPE_PORTABILITY         : type_desc = (char*) "PORTABILITY";         break; 
        case GL_DEBUG_TYPE_PERFORMANCE         : type_desc = (char*) "PERFORMANCE";         break; 
        case GL_DEBUG_TYPE_MARKER              : type_desc = (char*) "MARKER";              break; 
        case GL_DEBUG_TYPE_PUSH_GROUP          : type_desc = (char*) "PUSH_GROUP";          break; 
        case GL_DEBUG_TYPE_POP_GROUP           : type_desc = (char*) "POP_GROUP";           break; 
        default                                : type_desc = (char*) "OTHER";
    }                                                         
                                                              
    switch (severity)                                         
    {
        case GL_DEBUG_SEVERITY_HIGH   : severity_desc = (char*) "HIGH";         break;      
        case GL_DEBUG_SEVERITY_MEDIUM : severity_desc = (char*) "MEDIUM";       break;
        case GL_DEBUG_SEVERITY_LOW    : severity_desc = (char*) "LOW";          break;
        default                       : severity_desc = (char*) "NOTIFICATION";
    }                                                        
                                                         
    printf("OpenGL debug message : id = %u.\n\tSOURCE : %s.\n\tTYPE : %s.\n\tSEVERITY : %s.\n\tmessage : %s.\n\n", id, source_desc, type_desc, severity_desc, message);
}

//=======================================================================================================================================================================================================================
// GLFW window constructor + GLEW library initialization
// the application will be terminated if one of the library fails to initialize or 
// requested context version is not supported by OpenGL driver
//=======================================================================================================================================================================================================================
glfw_window_t::glfw_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen, bool debug_context)
{
    frame = 0;
    glfw_window_t::res_x = res_x;
    glfw_window_t::res_y = res_y;

    //===============================================================================================================================================================================================================
    // Set the number of antialiasing samples, OpenGL major and minor context version and
    // forward compatible core profile (deprecated functions not to be supported)
    //===============================================================================================================================================================================================================
    glfwWindowHint(GLFW_SAMPLES, glfw_samples);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version_minor);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);                                                                    
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (debug_context)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    //===============================================================================================================================================================================================================
    // Create window ...
    //===============================================================================================================================================================================================================
    window = glfwCreateWindow(res_x, res_y, title, fullscreen ? glfwGetPrimaryMonitor() : 0, 0); 
    if(!window)
    {
        glfw::terminate();
        exit_msg("Failed to open GLFW window. No open GL %d.%d support.", version_major, version_minor);
    }

    //===============================================================================================================================================================================================================
    // ... and make it current
    //===============================================================================================================================================================================================================
    glfwMakeContextCurrent(window);
    debug_msg("GLFW initialization done ... ");

    //===============================================================================================================================================================================================================
    // GLEW library initialization
    //===============================================================================================================================================================================================================
    glewExperimental = true;                                                                                                // needed in core profile 
    GLenum result = glewInit();                                                                                             // initialise GLEW
    if (result != GLEW_OK) 
    {
        glfw::terminate();
        exit_msg("Failed to initialize GLEW : %s", glewGetErrorString(result));
    }
    debug_msg("GLEW library initialization done ... ");

    //===============================================================================================================================================================================================================
    // enable debug output and set up callback routine
    //===============================================================================================================================================================================================================
    if (debug_context)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debugCallback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, true);
    }

    //===============================================================================================================================================================================================================
    // mouse and keyboard event callback functions
    //===============================================================================================================================================================================================================
    glfwSetCursorPos(window, 0.5 * res_x, 0.5 * res_y);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, glfw::key_callback);
    glfwSetMouseButtonCallback(window, glfw::mouse_button_callback);
    glfwSetCursorPosCallback(window, glfw::mouse_move_callback);
    glfwSetCursorEnterCallback(window, glfw::cursor_enter_callback);
    glfwSetCharCallback(window, glfw::character_callback);
    glfwSetScrollCallback(window, glfw::scroll_callback);

    //===================================================================================================================================================================================================================
    // time and mouse state variables
    //===================================================================================================================================================================================================================
    mouse_ts = frame_ts = glfw::time();
    mouse_delta = mouse = glm::dvec2(0.0);
    frame_dt = mouse_dt = 0.0;
}

void glfw_window_t::new_frame()
{
    //===================================================================================================================================================================================================================
    // Setup time step
    //===================================================================================================================================================================================================================
    double current_ts = glfw::time();
    frame_dt = current_ts - frame_ts;
    frame_ts = current_ts;
    glfw::poll_events();
}

void glfw_window_t::end_frame()
{
    frame++;
    swap_buffers();
}

void glfw_window_t::swap_buffers()
    { glfwSwapBuffers(window); }

void glfw_window_t::set_should_close(int value)
    { glfwSetWindowShouldClose(window, value); }

bool glfw_window_t::should_close()
    { return glfwWindowShouldClose(window); }

double glfw_window_t::aspect()
    { return double(res_x) / double(res_y); }

double glfw_window_t::inv_aspect()
    { return double(res_y) / double(res_x); }


void glfw_window_t::disable_cursor()
    { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }

void glfw_window_t::enable_cursor()
    { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }

int glfw_window_t::key_state(int key)
    { return glfwGetKey(window, key); }

glm::dvec2 glfw_window_t::cursor_position()
{
    glm::dvec2 pos;
    glfwGetCursorPos(window, &pos.x, &pos.y);
    return pos;
}

glfw_window_t::~glfw_window_t()
    { glfwDestroyWindow(window); }



