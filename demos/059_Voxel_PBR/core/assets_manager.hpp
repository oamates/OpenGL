#pragma once

#include <memory>
#include <map>

struct Renderer;
struct Behavior;
struct ProgramShader;
struct Scene;
struct ui_t;

struct AssetsManager                                                        // This class holds all the assets used by the engine in runtime
{
    ~AssetsManager();
    AssetsManager();

    AssetsManager(AssetsManager const &r) = delete;                         // No copying, copy, move assignment allowed of this class or any derived class
    AssetsManager(AssetsManager const &&r) = delete;
    AssetsManager& operator = (AssetsManager const &r) = delete;
    
    static std::unique_ptr<AssetsManager> &Instance();                      // Instances and initializes the assets manager.    
    static void Terminate();                                                // Terminates this instance. Call at engine loop end
    
    std::map<std::string, std::shared_ptr<Scene>> scenes;                   // Contains pointers to the asset manager scenes
    std::map<std::string, std::shared_ptr<ui_t>> interfaces;                // Pointers to implemented interfaces
    std::map<std::string, std::shared_ptr<Behavior>> behaviors;             // Pointers to implemented behaviors
    std::map<std::string, std::shared_ptr<ProgramShader>> programs;         // Pointers to implemented shader programs
    std::map<std::string, std::shared_ptr<Renderer>> renderers;             // Pointers to implemented renderers
};