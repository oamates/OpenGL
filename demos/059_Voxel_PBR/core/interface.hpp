#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../rendering/interface_renderer.hpp"
#include "../core/engine_base.hpp"
#include "../types/instance_pool.hpp"

// Interface code in the Draw method is meant to be implemented by all inheriting classes.
// DrawAll will call all implementations of Draw from instances of classes inheriting from Interface

struct Interface : InterfaceRenderer, public InstancePool<Interface>
{
    ImGuiIO* io;
    ImGuiStyle* style;

    Interface()
    {
        io = &ImGui::GetIO();
        style = &ImGui::GetStyle();
        style->WindowRounding = 0.0f;
    }

    virtual ~Interface() {}

    virtual void Draw() = 0;                                // Called per frame, interface drawing logic resides on this method
    friend void EngineBase::MainLoop() const;
    static void DrawAll()
    {
        if (renderer->disabled)
            return;

        for (auto &ui : instances)
            ui->Draw();
    }
};