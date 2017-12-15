#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "engine_base.hpp"

#include "behavior.hpp"
#include "interface.hpp"
#include "renderer.hpp"
#include "assets_manager.hpp"
#include "../rendering/render_window.hpp"
#include "../types/transform.hpp"

#include <oglplus/gl.hpp>
#include <oglplus/context.hpp>
#include <iostream>

#include <FreeImagePlus.h>
#include <assimp/Importer.hpp>
#include <assimp/version.h>

#include "../make_unique.hpp"

#include "log.hpp"

EngineBase::EngineBase()
{
    renderWindow = std::make_unique<RenderWindow>();
}

EngineBase::~EngineBase()
{
    // release reserved data early (context dependent)
    InterfaceRenderer::Terminate();
    AssetsManager::Terminate();
}

std::unique_ptr<EngineBase> &EngineBase::Instance()
{
    static std::unique_ptr<EngineBase> instance = nullptr;

    if (!instance)
        instance.reset(new EngineBase());

    return instance;
}

void EngineBase::Terminate()
{
    delete Instance().release();
}

void EngineBase::MainLoop() const
{
    static oglplus::Context gl;
    static bool enteredMainLoop = false;

    if (enteredMainLoop)
        return;
    
    this->Initialize();                                     // import assets and initialize ext libraries
    enteredMainLoop = true;                                 // entered main loop, this function cannot be called again
    
    while (!renderWindow->ShouldClose())                    // main render loop
    {
        gl.ClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        gl.Clear().ColorBuffer().DepthBuffer();
        
        RenderWindow::Events();                             // poll window inputs
        InterfaceRenderer::NewFrame();                      // setup interface renderer for a new frame
        Interface::DrawAll();                               // interfaces logic update
        Behavior::UpdateAll();                              // behaviors update
        Renderer::RenderAll();                              // call renderers
        InterfaceRenderer::Render();                        // ui render over scene
        renderWindow->SwapBuffers();                        // finally swap current frame
        Transform::CleanEventMap();                         // clean up
    }
}

RenderWindow &EngineBase::Window() const
{
    return *renderWindow;
}

void EngineBase::Initialize() const
{
    if(!renderWindow->IsOpen())
    {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        renderWindow->WindowHint(FramebufferHints::RedBits, mode->redBits);         // open window
        renderWindow->WindowHint(FramebufferHints::GreenBits, mode->greenBits);
        renderWindow->WindowHint(FramebufferHints::BlueBits, mode->blueBits);
        renderWindow->WindowHint(FramebufferHints::RefreshRate, mode->refreshRate);
        renderWindow->WindowHint(FramebufferHints::DoubleBuffer, true);
        renderWindow->WindowHint(WindowHints::AutoIconify, false);
        renderWindow->WindowHint(WindowHints::Resizable, false);
        renderWindow->WindowHint(ContextHints::ContextVersionMajor, 4);
        renderWindow->WindowHint(ContextHints::ContextVersionMinor, 5);
        renderWindow->WindowHint(ContextHints::OpenGLProfile, Hint::OpenGLCoreProfile);
        renderWindow->WindowHint(ContextHints::OpenGLForwardCompatibility, Hint::False);
        renderWindow->Open(WindowInfo(mode->width, mode->height, 0, 0, "Voxel Cone Tracing"), false, monitor, nullptr);
    }

    renderWindow->SetAsCurrentContext();                                            // and set window as rendering context
    oglplus::GLAPIInitializer();                                                    // initialize OpenGL API
    InterfaceRenderer::Initialize(*renderWindow);                                   // set interface to current renderwindow

    /* print library versions information */
    debug_msg("GLFW : %s", glfwGetVersionString());
    debug_msg("Assimp : %u.%u.%u", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision());
    debug_msg("FreeImage : %s", FreeImage_GetVersion());
    debug_msg("OpenGL : %s", glGetString(GL_VERSION));
    debug_msg("GLSL : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    debug_msg("GLEW : %s", glewGetString(GLEW_VERSION));
    debug_msg("IMGUI : %s", ImGui::GetVersion());

    AssetsManager::Instance();                                                      // initialize assets manager, holds all engine assets
}
