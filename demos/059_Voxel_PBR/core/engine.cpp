#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <oglplus/gl.hpp>
#include <oglplus/context.hpp>

#include <assimp/Importer.hpp>
#include <assimp/version.h>

#include "behavior.hpp"
#include "ui.hpp"
#include "renderer.hpp"
#include "assets_manager.hpp"
#include "../rendering/render_window.hpp"
#include "../types/transform.hpp"
#include "../make_unique.hpp"

#include "engine.hpp"
#include "log.hpp"

engine_t::engine_t()
{
    render_window = std::make_unique<render_window_t>();
}

engine_t::~engine_t()
{
    // release reserved data early (context dependent)
    ui_renderer_t::terminate();
    AssetsManager::Terminate();
}

std::unique_ptr<engine_t>& engine_t::instance()
{
    static std::unique_ptr<engine_t> instance = nullptr;
    if (!instance)
        instance.reset(new engine_t());
    return instance;
}

void engine_t::terminate()
{
    delete instance().release();
}

void engine_t::mainloop() const
{
    debug_msg("Entering main program loop");
    static bool enteredMainLoop = false;

    if (enteredMainLoop)
        return;
    
    initialize();                                               // import assets and initialize ext libraries
    enteredMainLoop = true;                                     // entered main loop, this function cannot be called again
    
    while (!render_window->ShouldClose())                       // main render loop
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_window_t::Events();                              // poll window inputs
        ui_renderer_t::new_frame();                             // setup interface renderer for a new frame
        ui_t::render_all();                                     // interfaces logic update
        Behavior::UpdateAll();                                  // behaviors update
        Renderer::RenderAll();                                  // call renderers
        ui_renderer_t::render();                                // ui render over scene
        render_window->SwapBuffers();                           // finally swap current frame
        Transform::CleanEventMap();                             // clean up
    }
}

render_window_t& engine_t::window() const
{
    return *render_window;
}

void engine_t::initialize() const
{
    if(!render_window->IsOpen())
    {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        render_window->WindowHint(FramebufferHints::RedBits, mode->redBits);         // open window
        render_window->WindowHint(FramebufferHints::GreenBits, mode->greenBits);
        render_window->WindowHint(FramebufferHints::BlueBits, mode->blueBits);
        render_window->WindowHint(FramebufferHints::RefreshRate, mode->refreshRate);
        render_window->WindowHint(FramebufferHints::DoubleBuffer, true);
        render_window->WindowHint(WindowHints::AutoIconify, false);
        render_window->WindowHint(WindowHints::Resizable, false);
        render_window->WindowHint(ContextHints::ContextVersionMajor, 4);
        render_window->WindowHint(ContextHints::ContextVersionMinor, 5);
        render_window->WindowHint(ContextHints::OpenGLProfile, Hint::OpenGLCoreProfile);
        render_window->WindowHint(ContextHints::OpenGLForwardCompatibility, Hint::False);
        render_window->WindowHint(ContextHints::OpenGLDebugContext, Hint::True);
        render_window->Open(WindowInfo(mode->width, mode->height, 0, 0, "Voxel Cone Tracing"), false, monitor, nullptr);
    }

    render_window->SetAsCurrentContext();                                            // and set window as rendering context
    oglplus::GLAPIInitializer();                                                    // initialize OpenGL API
    ui_renderer_t::initialize(*render_window);                                       // set interface to current renderwindow

    /* print library versions information */
    debug_msg("GLFW : %s", glfwGetVersionString());
    debug_msg("Assimp : %u.%u.%u", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision());
    debug_msg("OpenGL : %s", glGetString(GL_VERSION));
    debug_msg("GLSL : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    debug_msg("GLEW : %s", glewGetString(GLEW_VERSION));
    debug_msg("IMGUI : %s", ImGui::GetVersion());

    AssetsManager::Instance();                                                      // initialize assets manager, holds all engine assets
}
