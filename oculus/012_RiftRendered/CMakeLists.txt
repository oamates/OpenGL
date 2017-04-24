add_executable (octulus011 main.cpp)

target_link_libraries(octulus011 LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} OVR)
