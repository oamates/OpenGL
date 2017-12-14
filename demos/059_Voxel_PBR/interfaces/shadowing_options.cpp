#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "shadowing_options.hpp"

#include "main_menu.hpp"
#include "../core/assets_manager.hpp"
#include "../renderers/shadow_map_renderer.hpp"
#include "../scene/light.hpp"

#include <string>
#include <glm/gtc/type_ptr.hpp>

void UIShadowingOptions::Draw()
{
    if (!UIMainMenu::drawShadowOptions)
        return;

    static auto &assets = AssetsManager::Instance();
    static auto &shadowRender = *static_cast<ShadowMapRenderer *> (assets->renderers["Shadowmapping"].get());

    if (ImGui::Begin("Shadows", &UIMainMenu::drawShadowOptions, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static auto selectedH = 5, selectedW = 5;
        static std::vector<int> sizes = { 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 };
        static std::vector<const char *> sizesStr = { "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192" };
        static std::vector<const char *> filtersStr = { "None", "Bilinear", "Trilinear" };
        // shadow map quality
        ImGui::Text("Shadowmap Resolution");

        if (ImGui::Combo("Height: ", &selectedH, sizesStr.data(), sizesStr.size()))
        {
            shadowRender.SetupFramebuffers(sizes[selectedW], sizes[selectedH]);
            if (shadowRender.Caster())
                shadowRender.Caster()->RegisterChange();
        }

        if (ImGui::Combo("Width: ", &selectedW, sizesStr.data(), sizesStr.size()))
        {
            shadowRender.SetupFramebuffers(sizes[selectedW], sizes[selectedH]);
            if (shadowRender.Caster())
                shadowRender.Caster()->RegisterChange();
        }

        // shadow map blurring / filtering
        static auto blurScale = 0.5f;
        static auto blurQuality = 1;
        static auto filtering = 1;
        static auto aniso = 8;

        if(ImGui::SliderFloat("Blur Scale", &blurScale, 0.0f, 8.0f))
        {
            shadowRender.BlurScale(blurScale);
            if (shadowRender.Caster())
                shadowRender.Caster()->RegisterChange();
        }

        if (ImGui::SliderInt("Blur Quality", &blurQuality, 1, 3))
        {
            shadowRender.BlurQuality(blurQuality);
            if (shadowRender.Caster())
                shadowRender.Caster()->RegisterChange();
        }

        if (ImGui::SliderInt("Anisotropic Filtering", &aniso, 0, 16))
        {
            shadowRender.Anisotropy(aniso);
            if (shadowRender.Caster())
                shadowRender.Caster()->RegisterChange();
        }

        if (ImGui::Combo("Filtering", &filtering, filtersStr.data(), filtersStr.size()))
        {
            shadowRender.Filtering(filtering);
            if (shadowRender.Caster())
                shadowRender.Caster()->RegisterChange();
        }

        // vsm and evsm setup
        static auto exponents = shadowRender.Exponents();
        static auto reduction = shadowRender.LightBleedingReduction();

        if(ImGui::SliderFloat("Light Bleeding Reduction", &reduction, 0.0f, 1.0f))
        {
            shadowRender.LightBleedingReduction(reduction);
            if (shadowRender.Caster())
                shadowRender.Caster()->RegisterChange();
        }

        if(ImGui::DragFloat2("Exponents", value_ptr(exponents)))
        {
            shadowRender.Exponents(exponents);
            if (shadowRender.Caster())
                shadowRender.Caster()->RegisterChange();
        }
    }

    ImGui::End();
}