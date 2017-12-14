#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../core/interface.hpp"
#include "main_menu.hpp"

struct UIFramerate : public Interface
{
    UIFramerate() {}
    ~UIFramerate() override {}

    void Draw() override
    {
        if (!UIMainMenu::drawFramerate)
            return;

        static auto position = ImVec2(io->DisplaySize.x - 80 - 3, io->DisplaySize.y - 50 - 3);
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