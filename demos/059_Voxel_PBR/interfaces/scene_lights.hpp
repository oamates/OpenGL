#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "../core/interface.hpp"
#include "../core/assets_manager.hpp"
#include "../renderers/shadow_map_renderer.hpp"
#include "../renderers/voxelizer_renderer.hpp"
#include "../scene/scene.hpp"
#include "../scene/light.hpp"
#include "main_menu.hpp"

struct UISceneLights : public Interface
{
    UISceneLights() {}
    ~UISceneLights() override {}

    void Draw() override
    {
        static auto &assets = AssetsManager::Instance();
        static auto shadowing = static_cast<ShadowMapRenderer *> (assets->renderers["Shadowmapping"].get());
        static auto &voxel = *static_cast<VoxelizerRenderer *> (assets->renderers["Voxelizer"].get());
        static auto scene = static_cast<Scene *>(nullptr);
        static auto light = static_cast<Light *>(nullptr);
        static auto shadowingMethod = int(0);
        
        static auto selected = -1;                                                  // control variables
        static glm::vec3 position;
        static glm::vec3 angles;
        static glm::vec3 color[3];
        static glm::vec3 attenuation;
        static glm::vec3 intensities;
        static glm::vec2 cone;
        static Light::LightType type;
        static std::vector<char> name;
    
        if (scene != Scene::Active().get())                                         // active scene changed
        {
            scene = Scene::Active().get();
            light = nullptr;
            selected = -1;
        }
    
        if ((!scene) || (!UIMainMenu::drawSceneLights))
            return;
        
        ImGui::Begin("Lights", &UIMainMenu::drawSceneLights);                       // begin editor
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
        ImGui::Columns(2);
    
        if (ImGui::Button("New Light"))
            scene->lights.push_back(std::make_shared<Light>());
    
        for (auto i = 0; i < scene->lights.size(); i++)
        {
            auto &current = scene->lights[i];
            ImGui::PushID(i);
            ImGui::BeginGroup();
    
            if (ImGui::Selectable(current->name.c_str(), i == selected))            // selected becomes the clicked selectable
            {
                selected = i;
                light = current.get();
                shadowingMethod = light->mode[0].to_ulong();
                position = light->transform.Position();
                angles = degrees(light->transform.Angles());
                color[0] = light->Ambient();
                color[1] = light->Diffuse();
                color[2] = light->Specular();
                intensities = light->Intensities();
                type = light->Type();
                cone[0] = glm::degrees(light->AngleInnerCone());
                cone[1] = glm::degrees(light->AngleOuterCone());
                attenuation[0] = light->attenuation.Constant();
                attenuation[1] = light->attenuation.Linear();
                attenuation[2] = light->attenuation.Quadratic();
                name.clear();                                                       // copy name to a standard vector
                copy(light->name.begin(), light->name.end(), back_inserter(name));
                name.push_back('\0');
            }
    
            ImGui::EndGroup();
            ImGui::PopID();
        }
    
        ImGui::NextColumn();
    
        if (selected >= 0 && light != nullptr)
        {
            if (ImGui::InputText("Name", name.data(), name.size()))
                light->name = std::string(name.data());
    
            if ((type != Light::Directional) && (ImGui::DragFloat3("Position", value_ptr(position), 0.1f)))
                light->Position(position);
    
            if ((type != Light::Point) && (ImGui::DragFloat3("Rotation", value_ptr(angles), 0.1f)))
                light->Rotation(radians(angles));
    
            if (ImGui::Combo("Type", reinterpret_cast<int *>(&type), "Directional\0Point\0Spot"))
            {
                light->TypeCollection(type);
                light->RegisterChange();
            }
    
            if (type == Light::Spot)
            {
                ImGui::Text("Cone Angle");
                ImGui::Indent();
    
                if (ImGui::SliderFloat("Outer", &cone[1], 1.0f, 179.0f))
                {
                    light->AngleOuterCone(glm::radians(cone[1]));
                    light->RegisterChange();
                    cone[0] = glm::min(cone[0], cone[1]);
                }
    
                if (ImGui::SliderFloat("Inner", &cone[0], 1.0f, cone[1]))
                {
                    light->AngleInnerCone(glm::radians(cone[0]));
                    light->RegisterChange();
                }
    
                ImGui::Unindent();
            }
    
            
            if (type != Light::Directional)                                         // attenuation controls
            {
                ImGui::Text("Attenuation");
                ImGui::Indent();
    
                if (ImGui::DragFloat("Constant", &attenuation[0], 0.01f))
                {
                    attenuation[0] = glm::max(0.0f, attenuation[0]);
                    light->attenuation.Constant(attenuation[0]);
                    light->RegisterChange();
                }
    
                if (ImGui::DragFloat("Linear", &attenuation[1], 0.001f))
                {
                    attenuation[1] = glm::max(0.0f, attenuation[1]);
                    light->attenuation.Linear(attenuation[1]);
                    light->RegisterChange();
                }
    
                if (ImGui::DragFloat("Quadratic", &attenuation[2], 0.001f))
                {
                    attenuation[2] = glm::max(0.0f, attenuation[2]);
                    light->attenuation.Quadratic(attenuation[2]);
                    light->RegisterChange();
                }
    
                ImGui::Unindent();
            }
    
            ImGui::Text("Color");
            ImGui::Indent();
    
            if (ImGui::ColorEdit3("Ambient", value_ptr(color[0])))
            {
                light->Ambient(color[0]);
                light->RegisterChange();
            }
    
            if (ImGui::ColorEdit3("Diffuse", value_ptr(color[1])))
            {
                light->Diffuse(color[1]);
                light->RegisterChange();
            }
    
            if (ImGui::ColorEdit3("Specular", value_ptr(color[2])))
            {
                light->Specular(color[2]);
                light->RegisterChange();
            }
    
            if (ImGui::SliderFloat3("Intensities", value_ptr(intensities), 0.0f, 16.0f))
            {
                light->Intensities(intensities);
                light->RegisterChange();
            }
    
            ImGui::Unindent();
    
            if (ImGui::RadioButton("No Shadows", &shadowingMethod, 0))
            {
                light->mode[0].reset();
                light->RegisterChange();
                if (light == shadowing->Caster())
                    shadowing->Caster(nullptr);
            }
    
            if (light->Type() == Light::Directional && ImGui::RadioButton("Shadow Mapping", &shadowingMethod, 1))
            {
                shadowing->Caster(light);
                light->mode[0].reset();
                light->mode[0].set(0, 1);
                light->RegisterChange();
                
                for (auto &l : scene->lights)                                       // there can be only one with shadow mapping
                    if (l.get() != light && l->mode[0][0])
                        l->mode[0].reset();
            }
    
            if (ImGui::RadioButton("Trace Shadow Cones", &shadowingMethod, 2))
            {
                light->mode[0].reset();
                light->mode[0].set(1, 1);
                light->RegisterChange();
    
                if (light == shadowing->Caster())
                    shadowing->Caster(nullptr);
            }
    
            if (ImGui::Button("Delete"))
            {
                if(light->mode[0][0])                                               // method 1 indicates this light is used for shadow mapping
                {
                    light->mode[0].reset();                                         // no shadowing from this light source
                    shadowing->Caster(nullptr);
                }
    
                light = nullptr;
                scene->lights.erase(scene->lights.begin() + selected);              // delete from scene
                voxel.UpdateRadiance();
            }
        }
        else
            ImGui::Text("No Light Selected");
    
        ImGui::PopStyleVar();
        ImGui::End();
    }
};



