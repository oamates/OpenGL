#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "../core/ui.hpp"
#include "../scene/scene.hpp"
#include "../scene/camera.hpp"
#include "main_ui.hpp"
#include "log.hpp"

struct UISceneCameras : public ui_t
{
    UISceneCameras() {}
    ~UISceneCameras() override {}

    void render() override
    {
        if (!main_ui_t::drawSceneCameras) return;

        static auto scene = static_cast<Scene *>(nullptr);
        static auto camera = static_cast<Camera *>(nullptr);
        
        static auto selected = -1;                                                  // control variables
        static glm::vec3 position;
        static glm::vec3 angles;
        static glm::vec3 proj;
        static std::vector<char> name;
        
        if (scene != Scene::Active().get())                                         // active scene changed
        {
            scene = Scene::Active().get();
            selected = -1;
            camera = nullptr;
        }
    
        if (!scene)                                                                 // no active scene
            return;
    
        ImGui::Begin("Cameras", &main_ui_t::drawSceneCameras);                     // begin editor
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
        ImGui::Columns(2);
    
        if (ImGui::Button("New Camera"))
        {
            scene->cameras.push_back(std::make_shared<Camera>());
            if (scene->cameras.size() == 1)                                         // just added a camera to a scene with no cameras thus make it default as active
                scene->cameras[0]->SetAsActive();
        }
    
        for (int i = 0; i < scene->cameras.size(); i++)
        {
            auto &current = scene->cameras[i];
            ImGui::PushID(i);
            ImGui::BeginGroup();
            if (ImGui::Selectable(current->name.c_str(), i == selected))            // selected becomes the clicked selectable
            {
                selected = i;
                camera = current.get();
                position = camera->transform.Position();
                angles = degrees(camera->transform.Angles());
                proj[0] = glm::degrees(camera->FieldOfView());
                proj[1] = camera->ClipPlaneNear();
                proj[2] = camera->ClipPlaneFar();
                name.clear();                                                       // copy name to a standard vector
                copy(camera->name.begin(), camera->name.end(), back_inserter(name));
                name.push_back('\0');
            }
            if (current->IsActive())
            {
                ImGui::SameLine();
                ImGui::Bullet();
            }
            ImGui::EndGroup();
            ImGui::PopID();
        }
    
        ImGui::NextColumn();
    
        if (selected >= 0 && camera != nullptr)
        {
            if (ImGui::InputText("Name", name.data(), name.size()))
                camera->name = std::string(name.data());
    
            if (ImGui::DragFloat3("Position", value_ptr(position), 0.1f))
                camera->transform.Position(position);
    
            if (ImGui::DragFloat3("Rotation", value_ptr(angles), 0.1f))
                camera->transform.Rotation(radians(angles));
    
            if (ImGui::SliderFloat("Field of View", &proj[0], 1.0f, 179.0f))
                camera->FieldOfView(glm::radians(proj[0]));
    
            ImGui::Text("Clipping Planes");
            ImGui::Indent();
    
            if (ImGui::DragFloat("Near", &proj[1], 0.1f, 0.01f))
            {
                proj[1] = glm::max(proj[1], 0.01f);
                camera->ClipPlaneNear(proj[1]);
            }
    
            if (ImGui::DragFloat("Far", &proj[2], 0.1f, 0.01f))
            {
                proj[2] = glm::max(proj[2], 0.01f);
                camera->ClipPlaneFar(proj[2]);
            }
    
            ImGui::Unindent();
    
            if(ImGui::Button("To Center"))
            {
                position = scene->rootNode->boundaries.Center();
                camera->Position(position);
            }
    
            ImGui::SameLine();
    
            if (ImGui::Button("Set as Active"))
                camera->SetAsActive();
    
            ImGui::SameLine();
    
            if (ImGui::Button("Delete"))
            {
                auto &toDelete = scene->cameras[selected];                          // temporal shared ptr ref
                scene->cameras.erase(scene->cameras.begin() + selected);            // delete camera in scene cameras
                camera = nullptr;
                if (scene->cameras.size() > 0 && toDelete->IsActive())              // in case the camera deleted was marked as active, set another camera as active
                {
                    scene->cameras[0]->SetAsActive();
                    selected = 0;
                }
            }
        }
        else
            ImGui::Text("No Camera Selected");
    
        ImGui::PopStyleVar();
        ImGui::End();
    }
};