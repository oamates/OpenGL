add_executable (octulus009 main.cpp)

target_link_libraries(octulus009 LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} OVR)
