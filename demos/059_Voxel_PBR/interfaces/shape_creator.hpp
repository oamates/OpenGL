#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "../core/ui.hpp"
#include "../core/assets_manager.hpp"
#include "../scene/scene.hpp"
#include "../scene/material.hpp"
#include "../scene/light.hpp"
#include "../renderers/voxelizer_renderer.hpp"
#include "../renderers/shadow_map_renderer.hpp"
#include "../primitives/shapes.hpp"

#include "main_ui.hpp"

inline bool ShapeName(void* data, int idx, const char** out_text)
{
    auto items = static_cast<std::vector<std::string> *>(data);

    if (out_text)
    {
        auto begin = items->begin();
        advance(begin, idx);
        *out_text = begin->c_str();
    }

    return true;
}

struct UIShapeCreator : public ui_t
{
    UIShapeCreator() {};
    ~UIShapeCreator() override {}

    void Draw() override
    {
        if (!main_ui_t::drawSceneNodes)
            return;
    
        static auto &assets = AssetsManager::Instance();
        static auto &voxel = *static_cast<VoxelizerRenderer*> (assets->renderers["Voxelizer"].get());
        static auto scene = static_cast<Scene*>(nullptr);
        static auto node = static_cast<Node*>(nullptr);
        static auto material = static_cast<Material*>(nullptr);
        
        static auto selected = -1;                                                          // control variables
        static glm::vec3 position;
        static glm::vec3 rotation;
        static glm::vec3 scale;
        static glm::vec3 ambient;
        static glm::vec3 specular;
        static glm::vec3 diffuse;
        static glm::vec3 emissive;
        static float shininess_exponent;
        static std::map<Scene *, std::vector<Node *>> addedNodes;
        static std::map<Node *, bool> customMat;
        static auto shapesNames = Shapes::ShapeNameList();
        static int shapeSelected = -1;
        static std::vector<char> name;
    
        if (scene != Scene::Active().get())                                                 // active scene changed
        {
            scene = Scene::Active().get();
            selected = -1;
            node = nullptr;
            auto it = addedNodes.find(scene);
            if (it == addedNodes.end())
                addedNodes[scene] = std::vector<Node*>();
        }
    
        if (!scene)                                                                         // no active scene
            return;
    
        ImGui::Begin("Objects", &main_ui_t::drawSceneNodes);                               // begin editor
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
        ImGui::Columns(2);
        ImGui::Combo("", &shapeSelected, ShapeName, &shapesNames, static_cast<int>(shapesNames.size()));
        ImGui::SameLine();
    
        if (shapeSelected >= 0 && ImGui::Button("Create Shape"))
        {
            auto shape = Shapes::GetShape(shapesNames[shapeSelected]);
            scene->rootNode->nodes.push_back(shape);
            scene->rootNode->BuildDrawList();
            addedNodes[scene].push_back(shape.get());                                       // add material from shape creator to scene
            auto it = find(scene->materials.begin(), scene->materials.end(), shape->meshes[0]->material);
    
            if (it == scene->materials.end())
                scene->materials.push_back(shape->meshes[0]->material);
    
            voxel.RevoxelizeScene();
        }
    
        for (auto i = 0; i < addedNodes[scene].size(); i++)
        {
            auto &current = addedNodes[scene][i];
            ImGui::PushID(i);
            ImGui::BeginGroup();
    
            if (ImGui::Selectable(current->name.c_str(), i == selected))                    // selected becomes the clicked selectable
            {
                selected = i;
                node = current;
                position = node->Position();
                rotation = degrees(node->Angles());
                scale = node->Scale();
                material = node->meshes[0]->material.get();                                 // all shapes have only mesh, thus one material
                ambient = material->ambient;
                diffuse = material->diffuse;
                specular = material->specular;
                shininess_exponent = material->shininess_exponent;
                emissive = material->emissive;
                name.clear();                                                               // copy name to a standard vector
                copy(node->name.begin(), node->name.end(), back_inserter(name));
                name.push_back('\0');
            }
            ImGui::EndGroup();
            ImGui::PopID();
        }
    
        ImGui::NextColumn();
    
        if (selected >= 0 && node != nullptr)
        {
            if (ImGui::InputText("Name", name.data(), name.size()))
                node->name = std::string(name.data());
    
            if (ImGui::DragFloat3("Position", value_ptr(position), 0.1f))
                node->Position(position);
    
            if (ImGui::DragFloat3("Rotation", value_ptr(rotation), 0.1f))
                node->Rotation(radians(rotation));
    
            if (ImGui::DragFloat3("Scale", value_ptr(scale), 0.1f))
                node->Scale(scale);
    
            auto it = customMat.find(node);
    
            if((it == customMat.end() || !it->second) && ImGui::Button("Create Custom Material"))
            {
                auto newMesh = std::make_shared<MeshDrawer>(*node->meshes[0]);
                auto newMaterial = std::make_shared<Material>(*node->meshes[0]->material);
                newMaterial->name = node->name + "CustomMat";
                material = newMaterial.get();
                ambient = material->ambient;
                diffuse = material->diffuse;
                specular = material->specular;
                shininess_exponent = material->shininess_exponent;
                scene->materials.push_back(newMaterial);
                node->meshes[0] = move(newMesh);
                node->meshes[0]->material = move(newMaterial);
                customMat[node] = true;
            }
    
            if (ImGui::ColorEdit3("Ambient", glm::value_ptr(ambient)))                           // draw custom material values
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
    
            if (ImGui::Button("Delete Shape"))
            {
                static auto &shadowRender = *static_cast<ShadowMapRenderer*> (assets->renderers["Shadowmapping"].get());
                auto itScene = find_if(scene->rootNode->nodes.begin(), scene->rootNode->nodes.end(), 
                                       [=] (const std::shared_ptr<Node>& ptr) { return ptr.get() == node; });
    
                if (it != customMat.end() && it->second)                                    // shape had custom material
                {
                    auto itMaterial = find(scene->materials.begin(), scene->materials.end(), node->meshes[0]->material);
                    if (itMaterial != scene->materials.end())
                        scene->materials.erase(itMaterial);
                }
    
                if (itScene != scene->rootNode->nodes.end())                                // remove from scene root node
                {
                    scene->rootNode->nodes.erase(itScene);
                    scene->rootNode->BuildDrawList();
                }
    
                auto itMap = find(addedNodes[scene].begin(), addedNodes[scene].end(), node);
                
                if (itMap != addedNodes[scene].end())                                       // remove from scene-node ui map
                    addedNodes[scene].erase(itMap);
    
                voxel.RevoxelizeScene();
                selected = -1;
                node = nullptr;
    
                if (shadowRender.Caster())
                    shadowRender.Caster()->RegisterChange();
            }
        }
        else
            ImGui::Text("No Node Selected");
    
        ImGui::PopStyleVar();
        ImGui::End();
    }    
};