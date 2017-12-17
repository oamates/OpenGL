#pragma once

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "../core/ui.hpp"
#include "../core/assets_manager.hpp"
#include "../renderers/voxelizer_renderer.hpp"
#include "main_ui.hpp"
#include "log.hpp"

struct UIVoxelizationOptions : public ui_t
{
    UIVoxelizationOptions() {}
    ~UIVoxelizationOptions() override {}

    void render() override
    {
        if (!main_ui_t::drawVoxelizationOptions) return;
    
        static auto &assets = AssetsManager::Instance();
        static auto &voxel = *static_cast<VoxelizerRenderer*> (assets->renderers["Voxelizer"].get());
    
        if (ImGui::Begin("Voxelization", &main_ui_t::drawVoxelizationOptions, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static auto texRes = 5;
            static std::vector<int> sizes = { 8, 16, 32, 64, 128, 256, 512 };
            static std::vector<const char *> sizesStr = { "8", "16", "32", "64", "128", "256", "512" };
            
            ImGui::BeginGroup();                                                            // shadow map quality
            ImGui::Text("Volume Resolution");
    
            if(ImGui::Combo("Resolution: ", &texRes, sizesStr.data(), sizesStr.size()))
            {
                voxel.SetupVoxelVolumes(sizes[texRes]);
                voxel.RevoxelizeScene();
            }
    
            static int updateFrequency = 0;
            static bool revoxOnNeed = true;
    
            if (ImGui::Checkbox("Update When Needed", &revoxOnNeed))
                voxel.SetUpdateFrequency(-1);
    
            if (!revoxOnNeed && ImGui::InputInt("Update Frequency", &updateFrequency))
            {
                updateFrequency = glm::max(updateFrequency, 0);
                voxel.SetUpdateFrequency(updateFrequency);
            }
    
            ImGui::Separator();
            static auto mipLevel = 0, direction = 0;
            static bool drawAlbedo = false;
            static bool drawNormals = false;
            static auto colors = glm::vec4(1.0f);
            auto maxLevel = log2(voxel.VolumeDimension()) - 1;
    
            if (ImGui::DragFloat4("Color Channels", value_ptr(colors), 1.0f, 0.0f, 1.0f))
                voxel.SetupDrawVoxels(mipLevel, drawAlbedo ? 8 : drawNormals ? 7 : direction, colors);
    
            if (ImGui::Checkbox("Draw Albedo", &drawAlbedo))
            {
                if (drawAlbedo && drawNormals) drawNormals = false;
                voxel.SetupDrawVoxels(mipLevel, drawAlbedo ? 8 : drawNormals ? 7 : direction, colors);
            }
    
            ImGui::SameLine();
            if (ImGui::Checkbox("Draw Normal (Visibility)", &drawNormals))
            {
                if (drawAlbedo && drawNormals) drawAlbedo = false;
                voxel.SetupDrawVoxels(mipLevel, drawNormals ? 7 : drawNormals ? 7 : direction, colors);
            }
    
            if(!drawAlbedo && !drawNormals)
            {
                if (ImGui::SliderInt("Draw Mip Level", &mipLevel, 0, maxLevel))
                    voxel.SetupDrawVoxels(mipLevel, direction, colors);
    
                if (ImGui::SliderInt("Draw Mip Direction", &direction, 0, 5))
                    voxel.SetupDrawVoxels(mipLevel, direction, colors);
            }
    
            if (ImGui::Button("Voxelize Now"))
                voxel.RevoxelizeScene();
    
            ImGui::EndGroup();
        }
        ImGui::End();
    }        
};