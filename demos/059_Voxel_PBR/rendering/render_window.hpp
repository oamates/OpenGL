#pragma once

#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

enum struct WindowHints
{
    Resizable = GLFW_RESIZABLE,
    Visible = GLFW_VISIBLE,
    Decorated = GLFW_DECORATED,
    Focused = GLFW_FOCUSED,
    AutoIconify = GLFW_AUTO_ICONIFY,
    Floating = GLFW_FLOATING
};

enum struct FramebufferHints
{
    RedBits = GLFW_RED_BITS,
    GreenBits = GLFW_GREEN_BITS,
    BlueBits = GLFW_BLUE_BITS,
    AlphaBits = GLFW_ALPHA_BITS,
    DepthBits = GLFW_DEPTH_BITS,
    StencilBits = GLFW_STENCIL_BITS,
    AccumRedBits = GLFW_ACCUM_RED_BITS,
    AccumGreenBits = GLFW_ACCUM_GREEN_BITS,
    AccumBlueBits = GLFW_ACCUM_BLUE_BITS,
    AccumAlphaBits = GLFW_ACCUM_ALPHA_BITS,
    AuxBuffers = GLFW_AUX_BUFFERS,
    Samples = GLFW_SAMPLES,
    RefreshRate = GLFW_REFRESH_RATE,
    Stereo = GLFW_STEREO,
    SRGBCapable = GLFW_SRGB_CAPABLE,
    DoubleBuffer = GLFW_DOUBLEBUFFER
};

enum struct ContextHints
{
    ClientAPI = GLFW_CLIENT_API,
    ContextVersionMajor = GLFW_CONTEXT_VERSION_MAJOR,
    ContextVersionMinor = GLFW_CONTEXT_VERSION_MINOR,
    ContextRevision = GLFW_CONTEXT_REVISION,
    ContextRobustness = GLFW_CONTEXT_ROBUSTNESS,
    OpenGLForwardCompatibility = GLFW_OPENGL_FORWARD_COMPAT,
    OpenGLDebugContext = GLFW_OPENGL_DEBUG_CONTEXT,
    OpenGLProfile = GLFW_OPENGL_PROFILE,
    ContextReleaseBehavior = GLFW_CONTEXT_RELEASE_BEHAVIOR
};

enum struct Hint
{
    True = GL_TRUE,
    False = GL_FALSE,
    DontCare = GLFW_DONT_CARE,
    OpenGLAPI = GLFW_OPENGL_API,
    OpenGLESAPI = GLFW_OPENGL_ES_API,
    NoRobustness = GLFW_NO_ROBUSTNESS,
    NoResetNotification = GLFW_NO_RESET_NOTIFICATION,
    LoseContextOnReset = GLFW_LOSE_CONTEXT_ON_RESET,
    OpenGLAnyProfile = GLFW_OPENGL_ANY_PROFILE,
    OpenGLCoreProfile = GLFW_OPENGL_CORE_PROFILE,
    OpenGLCompatibilityProfile = GLFW_OPENGL_COMPAT_PROFILE,
    AnyReleaseBehavior = GLFW_ANY_RELEASE_BEHAVIOR,
    FlushReleaseBehavior = GLFW_RELEASE_BEHAVIOR_FLUSH,
    NoneReleaseBehavior = GLFW_RELEASE_BEHAVIOR_NONE,
};

enum struct EventMode
{
    Poll,
    Wait
};

struct WindowInfo
{
    unsigned int displayWidth;
    unsigned int displayHeight;
    unsigned int framebufferWidth;
    unsigned int framebufferHeight;
    int x;
    int y;
    std::string title;

    WindowInfo();
    WindowInfo(const unsigned width, const unsigned height, const int x, const int y, const std::string& title);
    virtual ~WindowInfo() {};
};

// Handles the instancing for the rendering context using GLFW.
// The rendering window settings such as size, position and hints can be set up with this class.
struct render_window_t
{
    static void WindowHint(const WindowHints &target, const int value);
    static void WindowHint(const FramebufferHints &target, const int value);
    static void WindowHint(const ContextHints &target, const int value);
    template<typename T> void WindowHint(T &&target, const Hint value);

    void Open(const WindowInfo &windowConfig, bool setPosition = true, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);
    void Destroy() const;

    void SetPosition(const int x, const int y);
    void SetWindowSize(const int w, const int h);
    void SetWindowTitle(const std::string &title);

    static void Events(EventMode mode = EventMode::Poll);
    int ShouldClose(bool sendClose = false) const;

    void SetAsCurrentContext() const;
    void SwapBuffers() const;

    render_window_t();
    virtual ~render_window_t();

    GLFWwindow* Handler() const { return windowHandler; }
    const WindowInfo &Info() const { return windowInfo; }
    bool IsOpen() const { return isOpen; }

	WindowInfo windowInfo;
    GLFWwindow* windowHandler;
    bool isOpen;
    static void OnErrorCallback(int code, const char * description);
};

template<typename T> void render_window_t::WindowHint(T&& target, const Hint value)
{
    WindowHint(std::forward<T>(target), static_cast<int>(value));
}