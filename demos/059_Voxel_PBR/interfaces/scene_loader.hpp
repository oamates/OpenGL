#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../core/ui.hpp"
#include "../core/assets_manager.hpp"
#include "../renderers/shadow_map_renderer.hpp"
#include "../util/scene_importer.hpp"
#include "../scene/camera.hpp"
#include "../scene/scene.hpp"
#include "../scene/light.hpp"
#include "main_ui.hpp"

struct EngineBase;

inline bool SceneName(void* data, int idx, const char** out_text)
{
    auto items = static_cast<std::map<std::string, std::shared_ptr<Scene>> *>(data);

    if (out_text)
    {
        auto begin = items->begin();
        advance(begin, idx);
        *out_text = begin->first.c_str();
    }

    return true;
}

struct UISceneLoader : public ui_t
{
    UISceneLoader() {}
    ~UISceneLoader() override {}
    
    void Draw() override
    {
        if (!main_ui_t::drawSceneLoader)
            return;
    
        static auto &assets = AssetsManager::Instance();
        static auto &scene = Scene::Active();
        static auto activeScene = -1;
        static Scene * reloaded = nullptr;
        ImGui::SetNextWindowPosCenter();
    
        if (ImGui::Begin("Load Scene", &main_ui_t::drawSceneLoader))
        {
            static auto advancedImport = false;
            static unsigned int flag = aiProcessPreset_TargetRealtime_Fast;
            static unsigned int flags[26] =
            {
                aiProcess_CalcTangentSpace,
                aiProcess_JoinIdenticalVertices,
                aiProcess_MakeLeftHanded,
                aiProcess_Triangulate,
                aiProcess_RemoveComponent,
                aiProcess_GenNormals,
                aiProcess_GenSmoothNormals,
                aiProcess_SplitLargeMeshes,
                aiProcess_PreTransformVertices,
                aiProcess_LimitBoneWeights,
                aiProcess_ValidateDataStructure,
                aiProcess_ImproveCacheLocality,
                aiProcess_RemoveRedundantMaterials,
                aiProcess_FixInfacingNormals,
                aiProcess_SortByPType,
                aiProcess_FindDegenerates,
                aiProcess_FindInvalidData,
                aiProcess_GenUVCoords,
                aiProcess_TransformUVCoords,
                aiProcess_FindInstances,
                aiProcess_OptimizeMeshes,
                aiProcess_OptimizeGraph,
                aiProcess_FlipUVs ,
                aiProcess_FlipWindingOrder,
                aiProcess_SplitByBoneCount,
                aiProcess_Debone,
            };
    
            if (reloaded)
                reloaded->SetAsActive();
    
            // active scene selector
            if (ImGui::Combo("Path", &activeScene, SceneName, &assets->scenes, static_cast<int>(assets->scenes.size())))
            {
                auto begin = assets->scenes.begin();
                advance(begin, activeScene);
                begin->second->SetAsActive();
                reloaded = nullptr;
            }
    
            if (scene) 
                ImGui::SameLine();
    
            if (scene && ImGui::Button(scene->IsLoaded() ? "Reload" : "Load"))
            {
                auto scenePtr = scene.get();
                scenePtr->IsLoaded() ? scenePtr->CleanImport(flag) : scenePtr->Import(flag);
                scenePtr->Load();
                static auto &shadowRender = *static_cast<ShadowMapRenderer *> (assets->renderers["Shadowmapping"].get());
    
                // set first directional light as shadowmapping
                for(auto &l : scenePtr->lights)
                {
                    if(l->Type() == Light::Directional)
                    {
                        l->mode[0].set(0, 1);
                        shadowRender.Caster(l.get());
                    }
                }
    
                if (shadowRender.Caster()) shadowRender.Caster()->RegisterChange();
    
                if (scenePtr->cameras.size() > 0) scenePtr->cameras.front()->SetAsActive();
    
                // to reset as active next frame
                reloaded = scenePtr;
            }
    
            static int chosen = 1;
    
            if (ImGui::Combo("Importing Preset", &chosen, "ToLeftHanded\0Fast\0Quality\0Max Quality", 4))
            {
                switch (chosen)
                {
                    case 0: flag = aiProcess_ConvertToLeftHanded; break;
                    case 1: flag = aiProcessPreset_TargetRealtime_Fast; break;
                    case 2: flag = aiProcessPreset_TargetRealtime_Quality; break;
                    case 3: flag = aiProcessPreset_TargetRealtime_MaxQuality; break;
                }
            }
    
            ImGui::Separator();
            ImGui::Checkbox("Show Advanced Import Settings", &advancedImport);
    
            if(advancedImport)
            {
                ImGui::Separator();
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
                ImGui::Columns(2);
    
                for (size_t i = 0; i < SceneImporter::FlagNames.size(); i++)
                {
                    if (2 * i == SceneImporter::FlagNames.size())
                        ImGui::NextColumn();
                    ImGui::CheckboxFlags(SceneImporter::FlagNames[i].c_str(), &flag, flags[i]);
                }
                ImGui::PopStyleVar();
            }
        }
        ImGui::End();
    }
};