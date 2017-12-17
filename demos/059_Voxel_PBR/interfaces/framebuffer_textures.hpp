#pragma once

#include "../core/ui.hpp"
#include "../core/assets_manager.hpp"
#include "../renderers/gi_deferred_renderer.hpp"
#include "../renderers/shadow_map_renderer.hpp"
#include "main_ui.hpp"

inline void DrawBufferTexture(const oglplus::Texture &tex, const std::string &name)
{
    using namespace oglplus;

    static ImVec2 size = ImVec2(160, 90);
    static ImVec2 uv1 = ImVec2(-1,  0);
    static ImVec2 uv2 = ImVec2( 0, -1);

    auto texName = reinterpret_cast<void*> (static_cast<intptr_t>(GetName(tex)));
    ImGui::BeginGroup();
    ImGui::Text(name.c_str());
    ImGui::Image(texName, size, uv1, uv2);

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Image(texName, ImVec2(160 * 4, 90 * 4), uv1, uv2);
        ImGui::EndTooltip();
    }

    ImGui::EndGroup();
}

struct UIFramebuffers : public ui_t
{
    ~UIFramebuffers() override {}
    UIFramebuffers() {}

    void Draw() override
    {
        if (!main_ui_t::drawFramebuffers)
            return;

        static auto& gbuffer = static_cast<GIDeferredRenderer *> (AssetsManager::Instance()->renderers["Deferred"].get())->BufferTextures();
        static auto& shadow = static_cast<ShadowMapRenderer *> (AssetsManager::Instance()->renderers["Shadowmapping"].get())->ShadowMap();

        ImGui::Begin("Geometry Buffer", &main_ui_t::drawFramebuffers, ImGuiWindowFlags_AlwaysAutoResize);              // begin editor
        ImGui::BeginGroup();
        ImGui::Text("Geometry Buffer");
        DrawBufferTexture(gbuffer[0], "Normal");
        ImGui::SameLine();
        DrawBufferTexture(gbuffer[1], "Albedo");
        ImGui::SameLine();
        DrawBufferTexture(gbuffer[2], "Specular");
        ImGui::SameLine();
        DrawBufferTexture(gbuffer[3], "Emissive");
        ImGui::SameLine();
        DrawBufferTexture(gbuffer[4], "Depth");
        ImGui::EndGroup();
        ImGui::BeginGroup();
        ImGui::Text("Shadow Mapping");
        DrawBufferTexture(shadow, "EVSM4");
        ImGui::EndGroup();
        ImGui::End();
    }
};