#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../core/ui.hpp"
#include "main_ui.hpp"
#include "log.hpp"

struct UIFramerate : public ui_t
{
    UIFramerate() {}
    ~UIFramerate() override {}

    void render() override
    {
        if (!main_ui_t::drawFramerate) return;

        static ImVec2 position = ImVec2(io->DisplaySize.x - 80 - 3, io->DisplaySize.y - 50 - 3);
        ImGui::SetNextWindowPos(position);

        if (ImGui::Begin("Performance Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Text("Framerate");
            ImGui::Separator();
            ImGui::Text("%.3f", io->Framerate);
        }

        position = ImGui::GetWindowPos();
        ImGui::End();        
    }
};