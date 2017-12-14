#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "interface.hpp"

Interface::Interface()
{
    io = &ImGui::GetIO();
    style = &ImGui::GetStyle();
    // style settings
    style->WindowRounding = 0.0f;
}

Interface::~Interface()
{
}

void Interface::DrawAll()
{
    if (renderer->disabled)
		return;

    for (auto &ui : instances)
    {
        ui->Draw();
    }
}
