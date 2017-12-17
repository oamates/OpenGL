#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "../core/ui.hpp"
#include "../core/assets_manager.hpp"
#include "../renderers/voxelizer_renderer.hpp"
#include "../scene/scene.hpp"
#include "../scene/material.hpp"
#include "main_ui.hpp"

struct UISceneMaterials : public ui_t
{
    UISceneMaterials() {}
    ~UISceneMaterials() override {}

    void Draw() override
    {
        if (!main_ui_t::drawSceneMaterials)
            return;
    
        static auto &assets = AssetsManager::Instance();
        static auto &voxel = *static_cast<VoxelizerRenderer*> (assets->renderers["Voxelizer"].get());
        static auto scene = static_cast<Scene *>(nullptr);
        static auto material = static_cast<Material *>(nullptr);
        
        static auto selected = -1;                                                          // control variables
        static glm::vec3 ambient;
        static glm::vec3 specular;
        static glm::vec3 diffuse;
        static glm::vec3 emissive;
        static float shininess_exponent;
        static std::vector<char> name;
        
        if (scene != Scene::Active().get())                                                 // active scene changed
        {
            scene = Scene::Active().get();
            selected = -1;
            material = nullptr;
        }
    
        if (!scene)                                                                         // no active scene
            return;
        
        ImGui::Begin("Materials", &main_ui_t::drawSceneMaterials);                         // begin editor
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
        ImGui::Columns(2);
    
        for (auto i = 0; i < scene->materials.size(); i++)
        {
            auto &current = scene->materials[i];
            ImGui::PushID(i);
            ImGui::BeginGroup();
    
            if (ImGui::Selectable(current->name.c_str(), i == selected))                    // selected becomes the clicked selectable
            {
                selected = i;
                material = current.get();
                ambient = material->ambient;
                specular = material->specular;
                diffuse = material->diffuse;
                emissive = material->emissive;
                shininess_exponent = material->shininess_exponent;
                name.clear();                                                               // copy name to a standard vector
                copy(material->name.begin(), material->name.end(), back_inserter(name));
                name.push_back('\0');
            }
    
            ImGui::EndGroup();
            ImGui::PopID();
        }
    
        ImGui::NextColumn();
    
        if (selected >= 0 && material != nullptr)
        {
            if (ImGui::InputText("Name", name.data(), name.size()))
                material->name = std::string(name.data());
    
            if (ImGui::ColorEdit3("Ambient", glm::value_ptr(ambient)))
                material->ambient = ambient;
    
            if (ImGui::ColorEdit3("Diffuse", glm::value_ptr(diffuse)))
            {
                material->diffuse = diffuse;
                voxel.RevoxelizeScene();
            }
    
            if (ImGui::ColorEdit3("Specular", glm::value_ptr(specular)))
                material->specular = specular;
    
            if (ImGui::DragFloat("Glossiness", &shininess_exponent, 0.001f, 0.0f, 1.0f))
                material->shininess_exponent = shininess_exponent;
    
            if (ImGui::ColorEdit3("Emissive", glm::value_ptr(emissive)))
            {
                material->emissive = emissive;
                voxel.RevoxelizeScene();
            }
        }
        else
            ImGui::Text("No Material Selected");
    
        ImGui::PopStyleVar();
        ImGui::End();
    }
};
