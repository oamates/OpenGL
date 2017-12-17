#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "main_ui.hpp"

#include "../renderers/voxelizer_renderer.hpp"
#include "../rendering/render_window.hpp"

bool main_ui_t::drawSceneLoader = true;
bool main_ui_t::drawFramerate = false;
bool main_ui_t::drawSceneCameras = false;
bool main_ui_t::drawSceneLights = false;
bool main_ui_t::drawFramebuffers = false;
bool main_ui_t::drawShadowOptions = false;
bool main_ui_t::drawVoxelizationOptions = false;
bool main_ui_t::drawGIOptions = false;
bool main_ui_t::drawSceneMaterials = false;
bool main_ui_t::drawSceneNodes = false;

void main_ui_t::Draw()
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
            engine_t::instance()->window().ShouldClose(true);

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