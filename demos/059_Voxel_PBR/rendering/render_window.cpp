#include <iostream>
#include <limits>
#include <stdexcept>

#include "render_window.hpp"
#include "log.hpp"

// Called when an error occurs using the GLFW window
void render_window_t::OnErrorCallback(int code, const char* description)
    { throw std::runtime_error(description); }


WindowInfo::WindowInfo() : WindowInfo(1280, 720, 0, 0, "Default") {}

// Initializes a new instance of the WindowInfo class.
WindowInfo::WindowInfo(const unsigned width, const unsigned height, const int x, const int y, const std::string &title)
    : displayWidth{width}, displayHeight{height}, framebufferWidth(0),
      framebufferHeight(0), x(x), y(y), title(title)
{
}

// Sets WindowHints hint value. Has to be called before calling WindowInfoRenderWindow.Open
void render_window_t::WindowHint(const WindowHints &target, const int value)
{
    if (static_cast<Hint>(value) == Hint::True || static_cast<Hint>(value) == Hint::False)
    {
        glfwWindowHint(static_cast<int>(target), value);
    }
}

// Sets FramebufferHints hint value. Has to be called before calling RenderWindow.Open()
void render_window_t::WindowHint(const FramebufferHints &target, const int value)
{
    if (target == FramebufferHints::Stereo ||
            target == FramebufferHints::SRGBCapable ||
            target == FramebufferHints::DoubleBuffer)
    {
        if (static_cast<Hint>(value) == Hint::True ||
                static_cast<Hint>(value) == Hint::False)
        {
            glfwWindowHint(static_cast<int>(target), value);
        }
    }
    else if (value > 0 || value < std::numeric_limits<int>::max() ||
             static_cast<Hint>(value) == Hint::DontCare)
    {
        glfwWindowHint(static_cast<int>(target), value);
    }
}

// Sets ContextHints hint value. Has to be called before calling RenderWindow.Open()
void render_window_t::WindowHint(const ContextHints &target, const int value)
{
    if (target == ContextHints::ClientAPI)
    {
        if (static_cast<Hint>(value) == Hint::OpenGLAPI || static_cast<Hint>(value) == Hint::OpenGLESAPI)
            glfwWindowHint(static_cast<int>(target), value);
    }
    else if (target == ContextHints::ContextRobustness)
    {
        if (static_cast<Hint>(value) == Hint::NoRobustness || static_cast<Hint>(value) == Hint::NoResetNotification || static_cast<Hint>(value) == Hint::LoseContextOnReset)
            glfwWindowHint(static_cast<int>(target), value);
    }
    else if (target == ContextHints::OpenGLForwardCompatibility || target == ContextHints::OpenGLDebugContext)
    {
        if (static_cast<Hint>(value) == Hint::True || static_cast<Hint>(value) == Hint::False)
            glfwWindowHint(static_cast<int>(target), value);
    }
    else if (target == ContextHints::OpenGLProfile)
    {
        if (static_cast<Hint>(value) == Hint::OpenGLAnyProfile || static_cast<Hint>(value) == Hint::OpenGLCoreProfile || static_cast<Hint>(value) == Hint::OpenGLCompatibilityProfile)
            glfwWindowHint(static_cast<int>(target), value);
    }
    else if (target == ContextHints::ContextReleaseBehavior)
    {
        if (static_cast<Hint>(value) == Hint::AnyReleaseBehavior || static_cast<Hint>(value) == Hint::FlushReleaseBehavior || static_cast<Hint>(value) == Hint::NoneReleaseBehavior)
            glfwWindowHint(static_cast<int>(target), value);
    }
    else if (target == ContextHints::ContextVersionMajor || target == ContextHints::ContextVersionMinor || target == ContextHints::ContextRevision)
        glfwWindowHint(static_cast<int>(target), value);
}

// Opens a GLFW window with the specified configuration.
void render_window_t::Open(const WindowInfo& windowConfig, bool setPosition, GLFWmonitor* monitor, GLFWwindow* share)
{
    if (isOpen || !glfwInit())
        return;

    windowInfo = std::move(windowConfig);
    windowHandler = glfwCreateWindow(windowInfo.displayWidth, windowInfo.displayHeight, windowInfo.title.c_str(), monitor, share);

    if (setPosition)
        glfwSetWindowPos(windowHandler, windowInfo.x, windowInfo.y);

    glfwSetWindowSize(windowHandler, windowInfo.displayWidth, windowInfo.displayHeight);
    glfwSetWindowTitle(windowHandler, windowInfo.title.c_str());
    glfwGetWindowPos(windowHandler, &windowInfo.x, &windowInfo.y);

    if (!windowHandler)
    {
        glfwTerminate();
        throw std::runtime_error("Couldn't create GLFW window.");
    }

    // save window info
    int w, h, displayW, displayH;;
    glfwGetWindowSize(windowHandler, &w, &h);
    glfwGetFramebufferSize(windowHandler, &displayW, &displayH);
    windowInfo.displayHeight = displayH;
    windowInfo.displayWidth = displayW;
    windowInfo.framebufferHeight = h;
    windowInfo.framebufferWidth = w;
    // successfull window open
    isOpen = true;
}

// Destroys the GLFW Window
void render_window_t::Destroy() const
    { glfwDestroyWindow(windowHandler); }

void render_window_t::SetPosition(const int x, const int y)
{
    glfwSetWindowPos(windowHandler, x, y);
    windowInfo.x = x;
    windowInfo.y = y;
}

void render_window_t::SetWindowSize(const int w, const int h)
{
    glfwSetWindowSize(windowHandler, w, h);
    windowInfo.displayWidth = w;
    windowInfo.displayHeight = h;
}

void render_window_t::SetWindowTitle(const std::string &title)
{
    glfwSetWindowTitle(windowHandler, title.c_str());
    windowInfo.title = title;
}

void render_window_t::SetAsCurrentContext() const
{
    glfwMakeContextCurrent(windowHandler);
}

// Looks up for input events.
void render_window_t::Events(EventMode mode)
{
    switch (mode)
    {
        case EventMode::Poll: glfwPollEvents(); break;
        case EventMode::Wait: glfwWaitEvents(); break;
        default: break;
    }
}

// Indicates if the render window should close or sets it to be closed.
int render_window_t::ShouldClose(bool sendClose) const
{
    if (sendClose)
        glfwSetWindowShouldClose(windowHandler, true);
    return glfwWindowShouldClose(windowHandler);
}

void render_window_t::SwapBuffers() const
    { glfwSwapBuffers(windowHandler); }

render_window_t::render_window_t() : windowHandler(nullptr), isOpen(false)
{
    glfwSetErrorCallback(OnErrorCallback);
    if (!glfwInit())
        debug_msg("Error initializing GLFW...");
}

render_window_t::~render_window_t()
    { glfwDestroyWindow(windowHandler); }
