add_executable (octulus006 main.cpp)

target_link_libraries(octulus006 LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} OVR)
