#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "log.hpp"

int main()
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create NO-API window
    // and try to invoke some vulkan
    //===================================================================================================================================================================================================================
    if (!glfwInit())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", 0, 0);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(0, &extensionCount, 0);

    debug_msg("%d extensions supported.", extensionCount);


    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}