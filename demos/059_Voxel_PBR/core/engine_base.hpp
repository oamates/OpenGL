#pragma once

#include <memory>

struct RenderWindow;
struct GIDeferredRenderer;
struct AssetsManager;

// This is the entry point of the rendering engine where the main rendering loop resides and the rendering context is properly set up.
struct EngineBase
{
    /// The rendering window.
    std::unique_ptr<RenderWindow> renderWindow;
    /// Setups all the engine components, imports assets and initializes libraries.
    void Initialize() const;
    EngineBase();
    virtual ~EngineBase();
    /// Main rendering loop
    void MainLoop() const;
    /// The active context window.
    RenderWindow &Window() const;
    /// Returns the EngineBase singleton instance.
    static std::unique_ptr<EngineBase> &Instance();
    /// Terminates this instance.
    static void Terminate();
	// No copying, copy, move assignment allowed of this class or any derived class
	EngineBase(EngineBase const &r) = delete;
	EngineBase(EngineBase const &&r) = delete;
	EngineBase &operator=(EngineBase const &r) = delete;
};
