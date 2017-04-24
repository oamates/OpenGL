add_executable (octulus008 main.cpp)

target_link_libraries(octulus008 LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} OVR)
