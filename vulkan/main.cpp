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
    {
        debug_msg("Failed to initialize GLFW library. Exiting ...");
        return -1;
    }

    debug_msg("GLFW Library initialized.");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", 0, 0);

    //===================================================================================================================================================================================================================
    // create VkApplicationInfo structure
    //===================================================================================================================================================================================================================
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = 0;
    appInfo.pApplicationName = 0;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = 0;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    //===================================================================================================================================================================================================================
    // get a list of extensions needed for GLFW window that Vulkan implementation must support
    //===================================================================================================================================================================================================================
    unsigned int glfwExtensionCount;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    debug_msg("Vulkan extensions required by GLFW :: ");
    for(int i = 0; i < glfwExtensionCount; ++i)
        debug_msg("\t%s", glfwExtensions[i]);

    //===================================================================================================================================================================================================================
    // initialize VkInstanceCreateInfo structure
    //===================================================================================================================================================================================================================
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 0;

    //===================================================================================================================================================================================================================
    // try to create VkInstance
    //===================================================================================================================================================================================================================
    VkInstance instance;
    VkResult result = vkCreateInstance(&createInfo, 0, &instance);
    if (result != VK_SUCCESS)
    {
        switch (result)
        {
            case VK_ERROR_OUT_OF_HOST_MEMORY :    debug_msg("Failed to create Vulkan instance :: VK_ERROR_OUT_OF_HOST_MEMORY");
            case VK_ERROR_OUT_OF_DEVICE_MEMORY :  debug_msg("Failed to create Vulkan instance :: VK_ERROR_OUT_OF_DEVICE_MEMORY");
            case VK_ERROR_INITIALIZATION_FAILED : debug_msg("Failed to create Vulkan instance :: VK_ERROR_INITIALIZATION_FAILED");
            case VK_ERROR_LAYER_NOT_PRESENT :     debug_msg("Failed to create Vulkan instance :: VK_ERROR_LAYER_NOT_PRESENT");
            case VK_ERROR_EXTENSION_NOT_PRESENT : debug_msg("Failed to create Vulkan instance :: VK_ERROR_EXTENSION_NOT_PRESENT");
            case VK_ERROR_INCOMPATIBLE_DRIVER :   debug_msg("Failed to create Vulkan instance :: VK_ERROR_INCOMPATIBLE_DRIVER");
            default : debug_msg("Failed to create Vulkan instance :: UNKNOWN ERROR");
        }
        return -1;
    }

    //===================================================================================================================================================================================================================
    // get the number of Vulkan-capable physical devices
    //===================================================================================================================================================================================================================
    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, 0);

    if (!physicalDeviceCount)
        exit_msg("No Vulkan capable device found in the system.");

    debug_msg("%u Vulkan devices found in the system :: ", physicalDeviceCount);

    VkPhysicalDevice* physicalDevices = (VkPhysicalDevice*) malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);
    VkPhysicalDevice* physicalDevice = 0;
    uint32_t familyIndex;

    //===================================================================================================================================================================================================================
    // among them, find the one that supports graphics queue
    //===================================================================================================================================================================================================================
    for (uint32_t i = 0; i < physicalDeviceCount; ++i)
    {
        VkPhysicalDevice& dev = physicalDevices[i];
        familyIndex = -1;

        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, 0);

        VkQueueFamilyProperties* familyProperties = (VkQueueFamilyProperties*) malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, familyProperties);

        for(uint32_t j = 0; j < queueFamilyCount; ++j)
        {
            VkQueueFamilyProperties& properties = familyProperties[j];
            if ((properties.queueCount > 0) && (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                familyIndex = j;
                physicalDevice = &dev;
                break;
            }
        }
        free(familyProperties);
    }        

    if (!physicalDevice)
    {
        debug_msg("Vulkan capable devices do not support VK_QUEUE_GRAPHICS_BIT");
        return -1;
    }

    //===================================================================================================================================================================================================================
    // create Vulkan logical device with one graphics queue
    //===================================================================================================================================================================================================================

    VkDeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = familyIndex;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures;

    VkDeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = 0;
    deviceCreateInfo.enabledLayerCount = 0;

    VkDevice device;
    VkQueue queue;

    result = vkCreateDevice(*physicalDevice, &deviceCreateInfo, 0, &device);
    if (result != VK_SUCCESS)
    {
        debug_msg("Failed to create logical device!");
        return -1;
    }

    debug_msg("Vulkan device successfully created. Implementation features :: ");


    debug_msg("\t robustBufferAccess                      : %s", deviceFeatures.robustBufferAccess                      ? "true" : "false");
    debug_msg("\t fullDrawIndexUint32                     : %s", deviceFeatures.fullDrawIndexUint32                     ? "true" : "false");
    debug_msg("\t imageCubeArray                          : %s", deviceFeatures.imageCubeArray                          ? "true" : "false");
    debug_msg("\t independentBlend                        : %s", deviceFeatures.independentBlend                        ? "true" : "false");
    debug_msg("\t geometryShader                          : %s", deviceFeatures.geometryShader                          ? "true" : "false");
    debug_msg("\t tessellationShader                      : %s", deviceFeatures.tessellationShader                      ? "true" : "false");
    debug_msg("\t sampleRateShading                       : %s", deviceFeatures.sampleRateShading                       ? "true" : "false");
    debug_msg("\t dualSrcBlend                            : %s", deviceFeatures.dualSrcBlend                            ? "true" : "false");
    debug_msg("\t logicOp                                 : %s", deviceFeatures.logicOp                                 ? "true" : "false");
    debug_msg("\t multiDrawIndirect                       : %s", deviceFeatures.multiDrawIndirect                       ? "true" : "false");
    debug_msg("\t drawIndirectFirstInstance               : %s", deviceFeatures.drawIndirectFirstInstance               ? "true" : "false");
    debug_msg("\t depthBiasClamp                          : %s", deviceFeatures.depthBiasClamp                          ? "true" : "false");
    debug_msg("\t depthBiasClamp                          : %s", deviceFeatures.depthBiasClamp                          ? "true" : "false");
    debug_msg("\t fillModeNonSolid                        : %s", deviceFeatures.fillModeNonSolid                        ? "true" : "false");
    debug_msg("\t depthBounds                             : %s", deviceFeatures.depthBounds                             ? "true" : "false");
    debug_msg("\t wideLines                               : %s", deviceFeatures.wideLines                               ? "true" : "false");
    debug_msg("\t largePoints                             : %s", deviceFeatures.largePoints                             ? "true" : "false");
    debug_msg("\t alphaToOne                              : %s", deviceFeatures.alphaToOne                              ? "true" : "false");
    debug_msg("\t multiViewport                           : %s", deviceFeatures.multiViewport                           ? "true" : "false");
    debug_msg("\t samplerAnisotropy                       : %s", deviceFeatures.samplerAnisotropy                       ? "true" : "false");
    debug_msg("\t textureCompressionETC2                  : %s", deviceFeatures.textureCompressionETC2                  ? "true" : "false");
    debug_msg("\t textureCompressionASTC_LDR              : %s", deviceFeatures.textureCompressionASTC_LDR              ? "true" : "false");
    debug_msg("\t textureCompressionBC                    : %s", deviceFeatures.textureCompressionBC                    ? "true" : "false");
    debug_msg("\t occlusionQueryPrecise                   : %s", deviceFeatures.occlusionQueryPrecise                   ? "true" : "false");
    debug_msg("\t pipelineStatisticsQuery                 : %s", deviceFeatures.pipelineStatisticsQuery                 ? "true" : "false");
    debug_msg("\t vertexPipelineStoresAndAtomics          : %s", deviceFeatures.vertexPipelineStoresAndAtomics          ? "true" : "false");
    debug_msg("\t fragmentStoresAndAtomics                : %s", deviceFeatures.fragmentStoresAndAtomics                ? "true" : "false");
    debug_msg("\t shaderTessellationAndGeometryPointSize  : %s", deviceFeatures.shaderTessellationAndGeometryPointSize  ? "true" : "false");
    debug_msg("\t shaderImageGatherExtended               : %s", deviceFeatures.shaderImageGatherExtended               ? "true" : "false");
    debug_msg("\t shaderStorageImageExtendedFormats       : %s", deviceFeatures.shaderStorageImageExtendedFormats       ? "true" : "false");
    debug_msg("\t shaderStorageImageMultisample           : %s", deviceFeatures.shaderStorageImageMultisample           ? "true" : "false");
    debug_msg("\t shaderStorageImageReadWithoutFormat     : %s", deviceFeatures.shaderStorageImageReadWithoutFormat     ? "true" : "false");
    debug_msg("\t shaderStorageImageWriteWithoutFormat    : %s", deviceFeatures.shaderStorageImageWriteWithoutFormat    ? "true" : "false");
    debug_msg("\t shaderUniformBufferArrayDynamicIndexing : %s", deviceFeatures.shaderUniformBufferArrayDynamicIndexing ? "true" : "false");
    debug_msg("\t shaderSampledImageArrayDynamicIndexing  : %s", deviceFeatures.shaderSampledImageArrayDynamicIndexing  ? "true" : "false");
    debug_msg("\t shaderStorageBufferArrayDynamicIndexing : %s", deviceFeatures.shaderStorageBufferArrayDynamicIndexing ? "true" : "false");
    debug_msg("\t shaderStorageImageArrayDynamicIndexing  : %s", deviceFeatures.shaderStorageImageArrayDynamicIndexing  ? "true" : "false");
    debug_msg("\t shaderClipDistance                      : %s", deviceFeatures.shaderClipDistance                      ? "true" : "false");
    debug_msg("\t shaderCullDistance                      : %s", deviceFeatures.shaderCullDistance                      ? "true" : "false");
    debug_msg("\t shaderFloat64                           : %s", deviceFeatures.shaderFloat64                           ? "true" : "false");
    debug_msg("\t shaderInt64                             : %s", deviceFeatures.shaderInt64                             ? "true" : "false");
    debug_msg("\t shaderInt16                             : %s", deviceFeatures.shaderInt16                             ? "true" : "false");
    debug_msg("\t shaderResourceResidency                 : %s", deviceFeatures.shaderResourceResidency                 ? "true" : "false");
    debug_msg("\t shaderResourceMinLod                    : %s", deviceFeatures.shaderResourceMinLod                    ? "true" : "false");
    debug_msg("\t sparseBinding                           : %s", deviceFeatures.sparseBinding                           ? "true" : "false");
    debug_msg("\t sparseResidencyBuffer                   : %s", deviceFeatures.sparseResidencyBuffer                   ? "true" : "false");
    debug_msg("\t sparseResidencyImage2D                  : %s", deviceFeatures.sparseResidencyImage2D                  ? "true" : "false");
    debug_msg("\t sparseResidencyImage3D                  : %s", deviceFeatures.sparseResidencyImage3D                  ? "true" : "false");
    debug_msg("\t sparseResidency2Samples                 : %s", deviceFeatures.sparseResidency2Samples                 ? "true" : "false");
    debug_msg("\t sparseResidency4Samples                 : %s", deviceFeatures.sparseResidency4Samples                 ? "true" : "false");
    debug_msg("\t sparseResidency8Samples                 : %s", deviceFeatures.sparseResidency8Samples                 ? "true" : "false");
    debug_msg("\t sparseResidency16Samples                : %s", deviceFeatures.sparseResidency16Samples                ? "true" : "false");
    debug_msg("\t sparseResidencyAliased                  : %s", deviceFeatures.sparseResidencyAliased                  ? "true" : "false");
    debug_msg("\t variableMultisampleRate                 : %s", deviceFeatures.variableMultisampleRate                 ? "true" : "false");
    debug_msg("\t inheritedQueries                        : %s", deviceFeatures.inheritedQueries                        ? "true" : "false");

    vkGetDeviceQueue(device, familyIndex, 0, &queue);

    //===================================================================================================================================================================================================================
    // create Vulkan surface
    //===================================================================================================================================================================================================================
    VkSurfaceKHR surface;
    result = glfwCreateWindowSurface(instance, window, 0, &surface);

    if (result != VK_SUCCESS)
    {
        debug_msg("Failed to create Vulkan window surface!");
        return -1;
    }

    debug_msg("Vulkan window surface created successfully");
    //===================================================================================================================================================================================================================
    // create Vulkan swapchain
    //===================================================================================================================================================================================================================
/*
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {(uint32_t) indices.graphicsFamily, (uint32_t) indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, swapChain.replace()) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }
        
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
*/
    debug_msg("Vulkan Instance successfully created. Running the main loop...");

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}