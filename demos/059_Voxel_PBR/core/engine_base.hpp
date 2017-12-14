#pragma once

#include <memory>

struct RenderWindow;
struct GIDeferredRenderer;
struct AssetsManager;

// This is the entry point of the rendering engine where the main rendering loop resides and the rendering context is properly set up.
struct EngineBase
{
    std::unique_ptr<RenderWindow> renderWindow;                         // The rendering window.
    void Initialize() const;                                            // Setups all the engine components, imports assets and initializes libraries.
    EngineBase();
    virtual ~EngineBase();
    void MainLoop() const;                                              // Main rendering loop
    RenderWindow &Window() const;                                       // The active context window.
    static std::unique_ptr<EngineBase> &Instance();                     // Returns the EngineBase singleton instance.
    static void Terminate();                                            // Terminates this instance.
    
    EngineBase(EngineBase const &r) = delete;                           // No copying, copy, move assignment allowed of this class or any derived class
    EngineBase(EngineBase const &&r) = delete;
    EngineBase &operator=(EngineBase const &r) = delete;
};
