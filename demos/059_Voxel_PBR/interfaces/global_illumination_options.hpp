#pragma once

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "../core/ui.hpp"
#include "../core/assets_manager.hpp"
#include "../renderers/gi_deferred_renderer.hpp"
#include "../renderers/voxelizer_renderer.hpp"
#include "main_ui.hpp"
#include "log.hpp"

struct UIGlobalIllumination : public ui_t
{
    UIGlobalIllumination() {}
    ~UIGlobalIllumination() override {}

    void render() override
    {
        if (!main_ui_t::drawGIOptions) return;

        static auto &assets = AssetsManager::Instance();
        static auto &deferred = *static_cast<GIDeferredRenderer *>
                                (assets->renderers["Deferred"].get());
    
        if (ImGui::Begin("Global Illumination", &main_ui_t::drawGIOptions, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static auto &voxel = *static_cast<VoxelizerRenderer *> (assets->renderers["Voxelizer"].get());
            static auto maxTracingConeDistance = deferred.MaxTracingDistance();
            static auto bounceStrength = deferred.GlobalIlluminationStrength();
            static auto aoFalloff = deferred.AmbientOclussionFalloff();
            static auto aoAlpha = deferred.AmbientOclussionAlpha();
            static auto mode = 0;
            static auto modes = "Direct + Indirect + Occlusion\0Direct + Indirec" \
                                "t\0Direct\0Indirect\0Ambient Occlusion";
            static auto firstB = voxel.InjectFirstBounce();
            static auto conesS = voxel.TraceShadowCones();
            static auto samplingFact = deferred.SamplingFactor();
            static auto fCone = deferred.ConeShadowTolerance();
            static auto fConeAperture = glm::degrees(deferred.ConeShadowAperture());
            static auto vWeightedLambert = voxel.WeightedLambert();
            static auto shadwMethod = 0;
    
            if (ImGui::SliderFloat("Maximum Trace Distance", &maxTracingConeDistance, 0.0f, 1.0f))
                deferred.MaxTracingDistance(maxTracingConeDistance);
    
            if (ImGui::SliderFloat("GI Strength", &bounceStrength, 0.0f, 32.0f))
                deferred.GlobalIlluminationStrength(bounceStrength);
    
            if (ImGui::SliderFloat("Sampling Factor", &samplingFact, 0.1f, 2.5f))
                deferred.SamplingFactor(glm::clamp(samplingFact, 0.1f, 2.5f));
    
            if (ImGui::Checkbox("Inject First Bounce", &firstB))
            {
                voxel.InjectFirstBounce(firstB);
                voxel.UpdateRadiance();
            }
    
            if (ImGui::Checkbox("Inject Raytraced Shadows", &conesS))
            {
                voxel.TraceShadowCones(conesS);
                voxel.UpdateRadiance();
            }
    
            if (ImGui::Checkbox("Use Normal Weighted Lambert", &vWeightedLambert))
            {
                voxel.WeightedLambert(vWeightedLambert);
                voxel.UpdateRadiance();
            }
    
            if (conesS)
            {
                ImGui::Indent();
                static auto umbra = voxel.TraceShadowHit();
    
                if(ImGui::SliderFloat("Tolerance", &umbra, 0.01f, 1.0f))
                {
                    voxel.TraceShadowHit(umbra);
                    voxel.UpdateRadiance();
                }
    
                ImGui::Unindent();
            }
    
            if(ImGui::Combo("Traced Shadow Cones", &shadwMethod, "Trace Per Fragment\0Sample Shadow Volume\0", 2))
                deferred.SampleVoxelShadowVolume(shadwMethod == 1);
    
            if (shadwMethod == 0)
            {
                ImGui::Indent();
                if (ImGui::SliderFloat("Fragment Shadow Cone Tolerance", &fCone, 0.0f, 1.0f))
                    deferred.ConeShadowTolerance(fCone);
                if (ImGui::SliderFloat("Fragment Shadow Cone Aperture", &fConeAperture, 1.0f, 90.0f))
                    deferred.ConeShadowAperture(glm::radians(fConeAperture));
                ImGui::Unindent();
            }
    
            if(ImGui::Combo("Render Mode", &mode, modes))
                deferred.RenderMode(mode);
    
            ImGui::Text("Ambient Occlusion");
            ImGui::Indent();
    
            if (ImGui::InputFloat("Falloff", &aoFalloff, 10))
                deferred.AmbientOclussionFalloff(aoFalloff);
    
            if (ImGui::SliderFloat("Alpha", &aoAlpha, 0.0f, 1.0f))
                deferred.AmbientOclussionAlpha(aoAlpha);
    
            ImGui::Unindent();
        }
    
        ImGui::End();
    }
};