#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../util/single_active.hpp"
#include "../types/instance_pool.hpp"

#include "../make_unique.hpp"

struct render_window_t;
struct ProgramShader;
struct Material;
struct Node;

// A base class for renderers. All the rendering logic should reside in the abstract method Render
struct Renderer : public SingleActive <Renderer>, public InstancePool<Renderer>
{
    std::unique_ptr<std::reference_wrapper<render_window_t>> window;        // The rendering window associated to this renderer. Usually the main window where the context was created.
    ProgramShader* program;                                                 // A direct reference to an active program shader
    
    Renderer() : window(nullptr), program(nullptr) {}                       // Initializes a new instance of the Renderer class.

    explicit Renderer(render_window_t &window)
        : program(nullptr)
        { this->window = std::make_unique<std::reference_wrapper<render_window_t>>(window); }

    virtual ~Renderer() {}                                                  // Finalizes an instance of the Renderer class.

    render_window_t& Window() const                                         // Returns the rendering window associated to this renderer. Usually the main window where the context was created.
        { return *window; }

    // Sets this program as the current active program in this renderer.
    template<class T> typename std::enable_if<std::is_base_of<ProgramShader, T>::value, void>::type CurrentProgram(T &program, bool use = true);
    // Returns the current active program associated to this renderer.
    template<class T> typename std::enable_if<std::is_base_of<ProgramShader, T>::value, T>::type &CurrentProgram() const;
    
    static void RenderAll()                                                 // Calls all the implementations of Render
    {
        for (auto &renderer : instances)
            renderer->Render();
    }
        
    virtual void Render() = 0;                                              // Rendering logic
    virtual void SetMatricesUniforms(const Node &node) const {}             // Sets the program matrices uniforms. Empty method, optionally implemented by inheriting classes.        
    virtual void SetMaterialUniforms(const Material &material) const {}     // Sets the program material uniforms. Empty method, optionally implemented by inheriting classes.        
};

template <class T> typename std::enable_if<std::is_base_of<ProgramShader, T>::value, void>::type Renderer::CurrentProgram(T& program, bool use)
{
    this->program = &program;
    if (use)
        program.Use();
}

template <class T> typename std::enable_if<std::is_base_of<ProgramShader, T>::value, T>::type& Renderer::CurrentProgram() const
    { return *static_cast<T*>(program); }
