#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Add vulkan dependencies using enviornment variable from LunarG SDK Setup
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

set(VULKAN_SDK "/home/alexander/projects/VulkanSDK/1.0.46.0/x86_64")

include_directories("${VULKAN_SDK}/include")
link_directories("${VULKAN_SDK}/lib") 
link_directories("${VULKAN_SDK}/etc/explicit_layer.d")

add_subdirectory (001_First_Triangle)
add_subdirectory (002_VAO)
add_subdirectory (003_UBO)
add_subdirectory (004_Textures)
add_subdirectory (005_RayMarch)
add_subdirectory (006_Depth_Attachment)
add_subdirectory (007_ImGui)