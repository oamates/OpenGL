add_executable (octulus010 main.cpp)

target_link_libraries(octulus010 LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} OVR)
