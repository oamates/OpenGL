#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../rendering/ui_renderer.hpp"
#include "../core/engine.hpp"
#include "../types/instance_pool.hpp"

// Interface code in the Draw method is meant to be implemented by all inheriting classes.
// DrawAll will call all implementations of Draw from instances of classes inheriting from Interface

struct ui_t : public ui_renderer_t, public InstancePool<ui_t>
{
    ImGuiIO* io;
    ImGuiStyle* style;

    ui_t()
    {
        io = &ImGui::GetIO();
        style = &ImGui::GetStyle();
        style->WindowRounding = 0.0f;
    }

    virtual ~ui_t() {}

    virtual void render() = 0;                                // Called per frame, interface drawing logic resides on this method
//    friend void EngineBase::MainLoop() const;
    static void render_all()
    {
        if (renderer->disabled)
            return;

        for (auto &ui : instances)
            ui->render();
    }
};