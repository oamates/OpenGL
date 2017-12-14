#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "main_menu.hpp"

#include "../renderers/voxelizer_renderer.hpp"
#include "../rendering/render_window.hpp"

bool UIMainMenu::drawSceneLoader = true;
bool UIMainMenu::drawFramerate = false;
bool UIMainMenu::drawSceneCameras = false;
bool UIMainMenu::drawSceneLights = false;
bool UIMainMenu::drawFramebuffers = false;
bool UIMainMenu::drawShadowOptions = false;
bool UIMainMenu::drawVoxelizationOptions = false;
bool UIMainMenu::drawGIOptions = false;
bool UIMainMenu::drawSceneMaterials = false;
bool UIMainMenu::drawSceneNodes = false;

void UIMainMenu::Draw()
{
    static bool showAbout = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Scene Loader", nullptr, &drawSceneLoader);
            ImGui::MenuItem("Show Framerate", nullptr, &drawFramerate);
            ImGui::MenuItem("View Voxels", nullptr, &VoxelizerRenderer::ShowVoxels);
            ImGui::MenuItem("View Framebuffers", nullptr, &drawFramebuffers);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scene"))
        {
            ImGui::MenuItem("Cameras", nullptr, &drawSceneCameras);
            ImGui::MenuItem("Lights", nullptr, &drawSceneLights);
            ImGui::MenuItem("Materials", nullptr, &drawSceneMaterials);
            ImGui::MenuItem("Shapes", nullptr, &drawSceneNodes);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Options"))
        {
            ImGui::MenuItem("Shadows", nullptr, &drawShadowOptions);
            ImGui::MenuItem("Voxelization", nullptr, &drawVoxelizationOptions);
            ImGui::MenuItem("Global Illumination", nullptr, &drawGIOptions);
            ImGui::MenuItem("About", nullptr, &showAbout);
            ImGui::EndMenu();
        }

        if(ImGui::Button("Exit"))
            EngineBase::Instance()->Window().ShouldClose(true);

        ImGui::EndMainMenuBar();
    }

    if(showAbout)
    {
        if (ImGui::Begin("About...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Voxel PBR rendering.");
            ImGui::Text("Email: afoksha@gmail.com");
        }
        ImGui::End();
    }
}