#pragma once

#include <memory>

struct render_window_t;

// This is the entry point of the rendering engine where the main rendering loop resides and the rendering context is properly set up.
struct engine_t
{
    std::unique_ptr<render_window_t> render_window;                     // The rendering window.

    engine_t();
    virtual ~engine_t();

    void initialize() const;                                            // Setups all the engine components, imports assets and initializes libraries.
    void mainloop() const;                                              // Main rendering loop
    render_window_t& window() const;                                    // The active context window.
    static std::unique_ptr<engine_t>& instance();                       // Returns the EngineBase singleton instance.
    static void terminate();                                            // Terminates this instance.
    
    engine_t(engine_t const &r) = delete;                               // No copying, copy, move assignment allowed of this class or any derived class
    engine_t(engine_t const &&r) = delete;
    engine_t& operator = (engine_t const &r) = delete;
};
