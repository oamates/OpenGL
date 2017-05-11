#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Add vulkan dependencies using enviornment variable from LunarG SDK Setup
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

set(VULKAN_SDK "/home/afoksha/projects/VulkanSDK/1.0.46.0/x86_64")

include_directories("${VULKAN_SDK}/include")
link_directories("${VULKAN_SDK}/lib") 
link_directories("${VULKAN_SDK}/etc/explicit_layer.d")

add_subdirectory (001_Vulkan_Instance)