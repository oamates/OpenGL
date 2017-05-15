#define GLEW_STATIC
#include <GL/glew.h> 
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cstring>
#include <array>
#include <set>

#include "log.hpp"
#include "constants.hpp"
#include "camera.hpp"
#include "image/stb_image.h"
#include "vao.hpp"
#include "vertex.hpp"


const char* pnt2_model_file = "../../../resources/models/vao/pnt2/chalet/chalet.vao";
const char* pnt2_texture_file = "../../../resources/models/vao/pnt2/chalet/chalet.jpg";


//========================================================================================================================================================================================================================
// camera-related variables
//========================================================================================================================================================================================================================

camera_t camera;

double frame_ts, mouse_ts;
double frame_dt = 0.0f, mouse_dt = 0.0f;

glm::dvec2 mouse = glm::dvec2(0.0);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
    else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
    else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
    else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);  
}

void mouse_move_callback(GLFWwindow* window, double x, double y)
{
    double t = glfwGetTime();
    glm::dvec2 mouse_np = glm::dvec2(x, y);
    glm::dvec2 mouse_delta = mouse_np - mouse;
    mouse = mouse_np;
    mouse_dt = t - mouse_ts;
    mouse_ts = t;
    mouse_delta.y = -mouse_delta.y;
    double norm = glm::length(mouse_delta);
    if (norm > 0.01)
        camera.rotateXY(mouse_delta / norm, norm * frame_dt);
}

const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const bool enableValidationLayers = false;

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
    PFN_vkCreateDebugReportCallbackEXT func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    return func ? func(instance, pCreateInfo, pAllocator, pCallback) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) 
{
    PFN_vkDestroyDebugReportCallbackEXT func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func)
        func(instance, callback, pAllocator);
}

//========================================================================================================================================================================================================================
// vulkan resource handling helper
//========================================================================================================================================================================================================================

template <typename T> struct VDeleter
{
    T object{VK_NULL_HANDLE};
    std::function<void(T)> delete_func;

    VDeleter() : VDeleter([](T, VkAllocationCallbacks*) {}) {}

    VDeleter(std::function<void(T, VkAllocationCallbacks*)> func)
        { delete_func = [=](T obj) { func(obj, 0); }; }

    VDeleter(const VDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> func)
        { delete_func = [&instance, func](T obj) { func(instance, obj, 0); }; }

    VDeleter(const VDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> func)
        { delete_func = [&device, func](T obj) { func(device, obj, 0); }; }

    ~VDeleter()
        { cleanup(); }

    const T* operator &() const
        { return &object; }

    T* replace()
    {
        cleanup();
        return &object;
    }

    operator T() const
        { return object; }

    void operator = (T rhs)
    {
        if (rhs == object) return;
        cleanup();
        object = rhs;
    }

    template<typename V> bool operator == (V rhs)
        { return object == T(rhs); }

    void cleanup()
    {
        if (object == VK_NULL_HANDLE) return;
        delete_func(object);
        object = VK_NULL_HANDLE;
    }
};

struct QueueFamilyIndices
{
    uint32_t graphicsFamily = -1;
    uint32_t presentFamily = -1;

    bool isComplete()
    {
        return graphicsFamily >= 0 && presentFamily >= 0;
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct UniformBufferObject 
{
    glm::mat4 projection_view_matrix;
    glm::vec4 camera_ws4;
    glm::vec4 light_ws4;
};

struct GLFWVulkanApplication 
{

    GLFWwindow* window;

    VDeleter<VkInstance> instance {vkDestroyInstance};
    VDeleter<VkDebugReportCallbackEXT> callback {instance, DestroyDebugReportCallbackEXT};
    VDeleter<VkSurfaceKHR> surface {instance, vkDestroySurfaceKHR};

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VDeleter<VkDevice> device {vkDestroyDevice};

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    
    VDeleter<VkSwapchainKHR> swapChain {device, vkDestroySwapchainKHR};
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VDeleter<VkImageView>> swapChainImageViews;
    std::vector<VDeleter<VkFramebuffer>> swapChainFramebuffers;
    
    VDeleter<VkRenderPass> renderPass {device, vkDestroyRenderPass};
    VDeleter<VkDescriptorSetLayout> descriptorSetLayout {device, vkDestroyDescriptorSetLayout};
    VDeleter<VkPipelineLayout> pipelineLayout {device, vkDestroyPipelineLayout};
    VDeleter<VkPipeline> graphicsPipeline {device, vkDestroyPipeline};
    
    VDeleter<VkCommandPool> commandPool {device, vkDestroyCommandPool};
    VDeleter<VkImage> textureImage {device, vkDestroyImage};
    VDeleter<VkDeviceMemory> textureImageMemory {device, vkFreeMemory};
    VDeleter<VkImageView> textureImageView {device, vkDestroyImageView};
    VDeleter<VkSampler> textureSampler {device, vkDestroySampler};
    
    VDeleter<VkBuffer> vertexBuffer {device, vkDestroyBuffer};
    VDeleter<VkDeviceMemory> vertexBufferMemory {device, vkFreeMemory};
    VDeleter<VkBuffer> indexBuffer {device, vkDestroyBuffer};
    VDeleter<VkDeviceMemory> indexBufferMemory {device, vkFreeMemory};

    VDeleter<VkBuffer> uniformStagingBuffer {device, vkDestroyBuffer};
    VDeleter<VkDeviceMemory> uniformStagingBufferMemory {device, vkFreeMemory};
    VDeleter<VkBuffer> uniformBuffer {device, vkDestroyBuffer};
    VDeleter<VkDeviceMemory> uniformBufferMemory {device, vkFreeMemory};

    VDeleter<VkDescriptorPool> descriptorPool {device, vkDestroyDescriptorPool};
    VkDescriptorSet descriptorSet;

    std::vector<VkCommandBuffer> commandBuffers;

    VDeleter<VkSemaphore> imageAvailableSemaphore {device, vkDestroySemaphore};
    VDeleter<VkSemaphore> renderFinishedSemaphore {device, vkDestroySemaphore};

    VDeleter<VkImage> depthImage{device, vkDestroyImage};
	VDeleter<VkDeviceMemory> depthImageMemory{device, vkFreeMemory};
	VDeleter<VkImageView> depthImageView{device, vkDestroyImageView};

    uint32_t index_count;

    void initWindow()
    {
        const int res_x = 1920;
        const int res_y = 1080;

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(res_x, res_y, "Vulkan Ray Marcher", glfwGetPrimaryMonitor(), 0);

        glfwSetWindowUserPointer(window, this);
        glfwSetWindowSizeCallback(window, GLFWVulkanApplication::onWindowResized);

        glfwSetCursorPos(window, 0.5 * res_x, 0.5 * res_y);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_move_callback);
        glfwSetKeyCallback(window, key_callback);

        camera.infinite_perspective(constants::two_pi / 6.0f, float(res_x) / float(res_y), 0.5f);
        mouse_ts = frame_ts = glfwGetTime();

    }

    void loadPNT2Model(const char* file_name)
    {
        debug_msg("Loading model :: %s", file_name);
        //================================================================================================================================================================================================================
        // read buffer params
        //================================================================================================================================================================================================================
        vao_t::header_t header;

        FILE* f = fopen(file_name, "rb");
        fread (&header, sizeof(vao_t::header_t), 1, f);

        assert(header.layout == BUFFER_LAYOUT(3, 3, 2));
        assert(header.mode == GL_TRIANGLES);
        assert(header.type == GL_UNSIGNED_INT);

        uint32_t stride = 8 * sizeof(float);
        uint32_t index_size = sizeof(GLuint);
        index_count = header.ibo_size;

        //================================================================================================================================================================================================================
        // create vertex buffer
        //================================================================================================================================================================================================================
        VkDeviceSize vbo_size = stride * header.vbo_size;

        VDeleter<VkBuffer> stagingVertexBuffer{device, vkDestroyBuffer};
        VDeleter<VkDeviceMemory> stagingVertexBufferMemory{device, vkFreeMemory};
        createBuffer(vbo_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingVertexBuffer, stagingVertexBufferMemory);

        void* buf_ptr;
        vkMapMemory(device, stagingVertexBufferMemory, 0, vbo_size, 0, &buf_ptr);
        fread(buf_ptr, stride, header.vbo_size, f);
        vkUnmapMemory(device, stagingVertexBufferMemory);

        createBuffer(vbo_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
        copyBuffer(stagingVertexBuffer, vertexBuffer, vbo_size);

        //================================================================================================================================================================================================================
        // create index buffer
        //================================================================================================================================================================================================================
        VkDeviceSize ibo_size = index_size * header.ibo_size;

        VDeleter<VkBuffer> stagingIndexBuffer{device, vkDestroyBuffer};
        VDeleter<VkDeviceMemory> stagingIndexBufferMemory{device, vkFreeMemory};
        createBuffer(ibo_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIndexBuffer, stagingIndexBufferMemory);

        vkMapMemory(device, stagingIndexBufferMemory, 0, ibo_size, 0, &buf_ptr);
        fread(buf_ptr, index_size, header.ibo_size, f);
        vkUnmapMemory(device, stagingIndexBufferMemory);

        createBuffer(ibo_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
        copyBuffer(stagingIndexBuffer, indexBuffer, ibo_size);

        fclose(f);
        debug_msg("Loading done");
    }

    void initVulkan()
    {
        createInstance();
        setupDebugCallback();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createDepthBuffer();
        createFramebuffers();
        createCommandPool();
        loadPNT2Model(pnt2_model_file);
        createTextureImage();
        createTextureImageView();
        createTextureSampler();
        createUniformBuffer();
        createDescriptorPool();
        createDescriptorSet();
        createCommandBuffers();
        createSemaphores();
    }

    static void onWindowResized(GLFWwindow* window, int res_x, int res_y)
    {
        if (res_x == 0 || res_y == 0) return;    
        GLFWVulkanApplication* application = reinterpret_cast<GLFWVulkanApplication*>(glfwGetWindowUserPointer(window));
        application->recreateSwapChain();
    }

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
    	for (VkFormat format : candidates)
    	{
        	VkFormatProperties props;
        	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        	if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            	return format;
        	if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
	            return format;
    	}
    	throw std::runtime_error("failed to find supported format!");
	}

	VkFormat findDepthFormat()
	{
    	return findSupportedFormat(
        	{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        	 VK_IMAGE_TILING_OPTIMAL,
        	 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    	);
	}

	bool hasStencilComponent(VkFormat format)
	{
    	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

    void createDepthBuffer()
    {
        VkFormat depthFormat = findDepthFormat();
        createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, depthImageView);
        transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    };

    void recreateSwapChain() 
    {
        vkDeviceWaitIdle(device);
        createSwapChain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createDepthBuffer();
        createFramebuffers();
        createCommandBuffers();
    }

    void createInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport())
            throw std::runtime_error("validation layers requested, but not available!");

        VkApplicationInfo appInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = 0,
            .pApplicationName = "Fragment Shader Ray March",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = 0,
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0
        };

        std::vector<const char*> extensions = getRequiredExtensions();

        VkInstanceCreateInfo createInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = enableValidationLayers ? (uint32_t) validationLayers.size() : 0,
            .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : 0,
            .enabledExtensionCount = (uint32_t) extensions.size(),
            .ppEnabledExtensionNames = extensions.data()
        };

        if (vkCreateInstance(&createInfo, 0, instance.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create instance!");
    }

    void createTextureSampler() 
    {
        VkSamplerCreateInfo samplerInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = 16,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = 0.0f, 
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE
        };

        if (vkCreateSampler(device, &samplerInfo, nullptr, textureSampler.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create texture sampler!");
    }

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VDeleter<VkImage>& image, VDeleter<VkDeviceMemory>& imageMemory)
    {

        VkImageCreateInfo imageInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = VkExtent3D
            {
                .width = width,
                .height = height,
                .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = tiling,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = 0,
            .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED
        };

        if (vkCreateImage(device, &imageInfo, nullptr, image.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create image!");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = 0,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
        };

        if (vkAllocateMemory(device, &allocInfo, nullptr, imageMemory.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate image memory!");

        vkBindImageMemory(device, image, imageMemory, 0);
    }

    VkCommandBuffer beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = 0,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = 0,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = 0
        };
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = 0,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = 0,
            .pWaitDstStageMask = 0,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = 0
        };

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    void createTextureImage()
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(pnt2_texture_file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels)
            throw std::runtime_error("failed to load texture image!");

        VkDeviceSize imageSize = texWidth * texHeight * 4;


        VDeleter<VkImage> stagingImage{device, vkDestroyImage};
        VDeleter<VkDeviceMemory> stagingImageMemory{device, vkFreeMemory};

        createImage (
            texWidth, texHeight, 
            VK_FORMAT_R8G8B8A8_UNORM, 
            VK_IMAGE_TILING_LINEAR, 
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            stagingImage, 
            stagingImageMemory
        );

        VkImageSubresource subresource = 
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .arrayLayer = 0
        };

        VkSubresourceLayout stagingImageLayout;
        vkGetImageSubresourceLayout(device, stagingImage, &subresource, &stagingImageLayout);

        void* data;
        vkMapMemory(device, stagingImageMemory, 0, imageSize, 0, &data);

        int rowPitch = texWidth * 4;

        if (stagingImageLayout.rowPitch == rowPitch)
            memcpy(data, pixels, (size_t) imageSize);
        else
        {
            uint8_t* dataBytes = (uint8_t*) data;
            for (int y = 0; y < texHeight; y++)
                memcpy(&dataBytes[y * stagingImageLayout.rowPitch], &pixels[y * rowPitch], rowPitch);
        }

        vkUnmapMemory(device, stagingImageMemory);
        stbi_image_free(pixels);


        createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
        transitionImageLayout(stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyImage(stagingImage, textureImage, texWidth, texHeight);
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);        
    }

    void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VDeleter<VkImageView>& imageView)
    {
        VkImageViewCreateInfo viewInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components = VkComponentMapping
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = VkImageSubresourceRange
            {
                .aspectMask = aspectFlags,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        if (vkCreateImageView(device, &viewInfo, 0, imageView.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create texture image view!");
    }

    void createTextureImageView()
    {
        createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureImageView);
    }

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkAccessFlags srcAccessMask, dstAccessMask;
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (hasStencilComponent(format))
                aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        } 
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            srcAccessMask = 0;
            dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }
        else
            throw std::invalid_argument("unsupported layout transition!");

        VkImageMemoryBarrier barrier = 
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = 0,
            .srcAccessMask = srcAccessMask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = VkImageSubresourceRange
            {
                .aspectMask = aspectMask,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, 0, 0, 0, 1, &barrier);
        endSingleTimeCommands(commandBuffer);
    }

    void setupDebugCallback()
    {
        if (!enableValidationLayers) return;

        VkDebugReportCallbackCreateInfoEXT createInfo = {};

        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = debugCallback;

        if (CreateDebugReportCallbackEXT(instance, &createInfo, 0, callback.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to set up debug callback!");
    }

    void createSurface()
    {
        if (glfwCreateWindowSurface(instance, window, 0, surface.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create window surface!");
    }

    void pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, 0);

        if (deviceCount == 0)
            throw std::runtime_error("failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const VkPhysicalDevice& device : devices)
        {
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("failed to find a suitable GPU!");
    }    

    void createLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = 
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .queueFamilyIndex = queueFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            };
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .queueCreateInfoCount = (uint32_t) queueCreateInfos.size(),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledLayerCount = enableValidationLayers ? (uint32_t) validationLayers.size() : 0,
            .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : 0,
            .enabledExtensionCount = (uint32_t) deviceExtensions.size(),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures = &deviceFeatures
        };

        if (vkCreateDevice(physicalDevice, &createInfo, 0, device.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create logical device!");

        vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
    }

    void createSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if ((swapChainSupport.capabilities.maxImageCount > 0) && (imageCount > swapChainSupport.capabilities.maxImageCount))
            imageCount = swapChainSupport.capabilities.maxImageCount;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

        VkSwapchainKHR oldSwapChain = swapChain;

        VkSwapchainCreateInfoKHR createInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = 0,
            .flags = 0,
            .surface = surface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = (indices.graphicsFamily != indices.presentFamily) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = (indices.graphicsFamily != indices.presentFamily) ? 2u : 0u,
            .pQueueFamilyIndices = (indices.graphicsFamily != indices.presentFamily) ? queueFamilyIndices : 0,
            .preTransform = swapChainSupport.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = oldSwapChain
        };

        VkSwapchainKHR newSwapChain;
        
        if (vkCreateSwapchainKHR(device, &createInfo, 0, &newSwapChain) != VK_SUCCESS)
            throw std::runtime_error("failed to create swap chain!");

        swapChain = newSwapChain;

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, 0);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void createImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size(), VDeleter<VkImageView>{device, vkDestroyImageView});

        for (uint32_t i = 0; i < swapChainImages.size(); i++)
            createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, swapChainImageViews[i]);
    }

    void createRenderPass()
    {
        VkAttachmentDescription colorAttachment = 
        {
            .flags = 0,
            .format = swapChainImageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentDescription depthAttachment = 
        {
            .flags = 0,
            .format = findDepthFormat(),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkAttachmentReference colorAttachmentRef = 
        {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkAttachmentReference depthAttachmentRef = 
        {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription subpass = 
        {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = 0,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pResolveAttachments = 0,
            .pDepthStencilAttachment = 0,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = 0
        };

        VkSubpassDependency dependency =
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = 0
        };

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

        VkRenderPassCreateInfo renderPassInfo =
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .attachmentCount = attachments.size(),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
        };

        if (vkCreateRenderPass(device, &renderPassInfo, 0, renderPass.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create render pass!");
    }

    void createDescriptorSetLayout()
    {

        VkDescriptorSetLayoutBinding uboLayoutBinding = 
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = 0
        };

        VkDescriptorSetLayoutBinding samplerLayoutBinding = 
        {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = 0
        };
        
        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

        VkDescriptorSetLayoutCreateInfo layoutInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .bindingCount = bindings.size(),
            .pBindings = bindings.data()
        };        

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, 0, descriptorSetLayout.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create descriptor set layout!");
    }    

    void createGraphicsPipeline() 
    {
        std::vector<char> vertShaderCode = readFile("glsl/vert.spv");
        std::vector<char> fragShaderCode = readFile("glsl/frag.spv");

        VDeleter<VkShaderModule> vertShaderModule {device, vkDestroyShaderModule};
        VDeleter<VkShaderModule> fragShaderModule {device, vkDestroyShaderModule};
        createShaderModule(vertShaderCode, vertShaderModule);
        createShaderModule(fragShaderCode, fragShaderModule);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertShaderModule,
            .pName = "main",
            .pSpecializationInfo = 0
        };

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragShaderModule,
            .pName = "main",
            .pSpecializationInfo = 0
        };

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkVertexInputBindingDescription bindingDescription = 
        {
            .binding = 0,
            .stride = sizeof(vertex_pnt2_t),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = 
        {
            VkVertexInputAttributeDescription
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(vertex_pnt2_t, position)
            },
            VkVertexInputAttributeDescription
            {
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(vertex_pnt2_t, normal)
            },
            VkVertexInputAttributeDescription
            {
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(vertex_pnt2_t, uv)
            }
        };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = attributeDescriptions.size(),
            .pVertexAttributeDescriptions = attributeDescriptions.data()
        }; 

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .primitiveRestartEnable = VK_FALSE
        };

        VkViewport viewport = 
        {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float) swapChainExtent.width,
            .height = (float) swapChainExtent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        VkRect2D scissor = 
        {
            .offset = VkOffset2D {.x = 0, .y = 0},
            .extent = swapChainExtent
        };

        VkPipelineViewportStateCreateInfo viewportState = 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor
        };

        VkPipelineRasterizationStateCreateInfo rasterizer = 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f
        };        

        VkPipelineMultisampleStateCreateInfo multisampling = 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0,
            .pSampleMask = 0,
            .alphaToCoverageEnable = 0,
            .alphaToOneEnable = 0
        };

        VkPipelineDepthStencilStateCreateInfo depthStencil = 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp  = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f
        };

        VkPipelineColorBlendAttachmentState colorBlendAttachment = 
        {
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo colorBlending = 
        {

            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
        };

        VkDescriptorSetLayout setLayouts[] = {descriptorSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .setLayoutCount = 1,
            .pSetLayouts = setLayouts,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = 0
        };

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, pipelineLayout.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");

        VkGraphicsPipelineCreateInfo pipelineInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pTessellationState = 0,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &colorBlending,
            .pDynamicState = 0,
            .layout = pipelineLayout,
            .renderPass = renderPass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0
        };

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, graphicsPipeline.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create graphics pipeline!");
    }

    void createFramebuffers()
    {
        swapChainFramebuffers.resize(swapChainImageViews.size(), VDeleter<VkFramebuffer>{device, vkDestroyFramebuffer});

        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo = 
            {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .renderPass = renderPass,
                .attachmentCount = attachments.size(),
                .pAttachments = attachments.data(),
                .width = swapChainExtent.width,
                .height = swapChainExtent.height,
                .layers = 1
            };

            if (vkCreateFramebuffer(device, &framebufferInfo, 0, swapChainFramebuffers[i].replace()) != VK_SUCCESS)
                throw std::runtime_error("failed to create framebuffer!");
        }
    }

    void createCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .queueFamilyIndex = queueFamilyIndices.graphicsFamily
        };

        if (vkCreateCommandPool(device, &poolInfo, 0, commandPool.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create command pool!");
    }

    void createUniformBuffer()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformStagingBuffer, uniformStagingBufferMemory);
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uniformBuffer, uniformBufferMemory);
    }

    void createDescriptorPool()
    {
        VkDescriptorPoolSize poolSize = 
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1
        };

        VkDescriptorPoolCreateInfo poolInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize
        };

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, descriptorPool.replace()) != VK_SUCCESS)
            throw std::runtime_error("Failed to create descriptor pool!");
    }

    void createDescriptorSet() 
    {
        VkDescriptorSetLayout layouts[] = {descriptorSetLayout};

        VkDescriptorSetAllocateInfo allocInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = 0,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = layouts
        };

        if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate descriptor set!");

        VkDescriptorBufferInfo bufferInfo = 
        {
            .buffer = uniformBuffer,
            .offset = 0,
            .range = sizeof(UniformBufferObject)
        };

        VkDescriptorImageInfo imageInfo = 
        {
            .sampler = textureSampler,
            .imageView = textureImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        std::array<VkWriteDescriptorSet, 2> descriptorWrites = 
        {
            VkWriteDescriptorSet
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = 0,
                .dstSet = descriptorSet,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo = 0,
                .pBufferInfo = &bufferInfo,
                .pTexelBufferView = 0
            },
            VkWriteDescriptorSet
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = 0,
                .dstSet = descriptorSet,
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &imageInfo,
                .pBufferInfo = 0,
                .pTexelBufferView = 0                
            }
        };

        vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, 0);
    }

    //====================================================================================================================================================================================================================
    // buffer helper functions
    //====================================================================================================================================================================================================================

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VDeleter<VkBuffer>& buffer, VDeleter<VkDeviceMemory>& bufferMemory)
    {
        VkBufferCreateInfo bufferInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = 0
        };

        if (vkCreateBuffer(device, &bufferInfo, nullptr, buffer.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create buffer!");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = 0,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
        };

        if (vkAllocateMemory(device, &allocInfo, nullptr, bufferMemory.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate buffer memory!");

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }


    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();
        VkBufferCopy copyRegion = 
        {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size   
        };
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        endSingleTimeCommands(commandBuffer);
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) return i;

        throw std::runtime_error("Failed to find suitable memory type!");
    }


    void createCommandBuffers()
    {
        if (commandBuffers.size() > 0)
            vkFreeCommandBuffers(device, commandPool, commandBuffers.size(), commandBuffers.data());

        commandBuffers.resize(swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = 0,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = (uint32_t) commandBuffers.size()
        };

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate command buffers!");

        for (size_t i = 0; i < commandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo = 
            {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = 0,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                .pInheritanceInfo = 0
            };

            vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

            std::array<VkClearValue, 2> clearValues;
            clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};

            VkRenderPassBeginInfo renderPassInfo = 
            {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .pNext = 0,
                .renderPass = renderPass,
                .framebuffer = swapChainFramebuffers[i],
                .renderArea = VkRect2D
                {
                    .offset = {0, 0},
                    .extent = swapChainExtent
                },
                .clearValueCount = clearValues.size(),
                .pClearValues = clearValues.data()
            };
            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            VkBuffer vertexBuffers[] = {vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, 0);
            vkCmdDrawIndexed(commandBuffers[i], index_count, 1, 0, 0, 0);
            vkCmdEndRenderPass(commandBuffers[i]);

            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to record command buffer!");
        }
    }

    void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageSubresourceLayers subResource = 
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        VkImageCopy region = 
        {
            .srcSubresource = subResource,
            .srcOffset = {0, 0, 0},
            .dstSubresource = subResource,
            .dstOffset = {0, 0, 0},
            .extent = {width, height, 1}

        };

        vkCmdCopyImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        endSingleTimeCommands(commandBuffer);
    }

    void createSemaphores()
    {
        VkSemaphoreCreateInfo semaphoreInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = 0,
            .flags = 0
        };

        if (vkCreateSemaphore(device, &semaphoreInfo, 0, imageAvailableSemaphore.replace()) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, 0, renderFinishedSemaphore.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create semaphores!");
    }

    void updateUniformBuffer()
    {
        float time = glfwGetTime();

        UniformBufferObject ubo;
        ubo.projection_view_matrix = camera.projection_view_matrix();
        ubo.camera_ws4 = glm::vec4(camera.position(), 0.0f);
        ubo.light_ws4 = glm::vec4(12.0f, 0.0f, 0.0f, time);

        void* data;
        vkMapMemory(device, uniformStagingBufferMemory, 0, sizeof(UniformBufferObject), 0, &data);
        memcpy(data, &ubo, sizeof(UniformBufferObject));
        vkUnmapMemory(device, uniformStagingBufferMemory);

        copyBuffer(uniformStagingBuffer, uniformBuffer, sizeof(UniformBufferObject));
    }

    void drawFrame()
    {
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("failed to acquire swap chain image!");

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};

        VkSubmitInfo submitInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = 0,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffers[imageIndex],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores
        };

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
            throw std::runtime_error("failed to submit draw command buffer!");

        VkSwapchainKHR swapChains[] = {swapChain};

        VkPresentInfoKHR presentInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = 0,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapChains,
            .pImageIndices = &imageIndex,
            .pResults = 0
        };

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS)
            throw std::runtime_error("failed to present swap chain image!");
    }

    void createShaderModule(const std::vector<char>& code, VDeleter<VkShaderModule>& shaderModule)
    {
        std::vector<uint32_t> codeAligned(code.size() / 4 + 1);
        memcpy(codeAligned.data(), code.data(), code.size());

        VkShaderModuleCreateInfo createInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .codeSize = code.size(),
            .pCode = codeAligned.data()
        };

        if (vkCreateShaderModule(device, &createInfo, 0, shaderModule.replace()) != VK_SUCCESS)
            throw std::runtime_error("failed to create shader module!");
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        if ((availableFormats.size() == 1) && (availableFormats[0].format == VK_FORMAT_UNDEFINED))
            return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

        for (const auto& availableFormat : availableFormats)
            if ((availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM) && (availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) return availableFormat;
        
        return availableFormats[0];
    }


    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
    {
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

        for (const VkPresentModeKHR& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;
            else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
                bestMode = availablePresentMode;
        }

        return bestMode;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        VkExtent2D extent;
        glfwGetWindowSize(window, (int*) &extent.width, (int*) &extent.height);

        extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
        extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));

        return extent;
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, 0);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, 0);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) 
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, 0, &extensionCount, 0);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, 0, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const VkExtensionProperties& extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);

        return requiredExtensions.empty();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
        {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (queueFamily.queueCount > 0 && presentSupport)
                indices.presentFamily = i;

            if (indices.isComplete()) break;

            i++;
        }
        return indices;
    }

    std::vector<const char*> getRequiredExtensions()
    {
        std::vector<const char*> extensions;

        unsigned int glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (unsigned int i = 0; i < glfwExtensionCount; i++)
            extensions.push_back(glfwExtensions[i]);

        if (enableValidationLayers)
            extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

        return extensions;
    }

    bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, 0);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;
            for (const VkLayerProperties& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) return false;
        }
        return true;
    }

    static std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("failed to open file!");

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* message, void* userData)
    {
        debug_msg("Validation layer :: %s", message);
        return VK_FALSE;
    }
};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{

    GLFWVulkanApplication application;

    try
    {
        application.initWindow();
        application.initVulkan();

        while (!glfwWindowShouldClose(application.window))
        {
            double t = glfwGetTime();
            frame_dt = t - frame_ts;
            frame_ts = t;

            glfwPollEvents();
            application.updateUniformBuffer();
            application.drawFrame();
        }

        vkDeviceWaitIdle(application.device);
        glfwDestroyWindow(application.window);
        glfwTerminate();
    }
    catch (const std::runtime_error& exception)
    {
        debug_msg("Program execution exception :: %s", exception.what());
        return EXIT_FAILURE;
    }

    return 0;
}