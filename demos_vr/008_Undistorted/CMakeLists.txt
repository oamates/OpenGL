add_executable (octulus007 main.cpp)

target_link_libraries(octulus007 LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} OVR)
