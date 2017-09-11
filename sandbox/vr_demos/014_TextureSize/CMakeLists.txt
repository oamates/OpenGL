add_executable (octulus013 main.cpp)

target_link_libraries(octulus013 LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} OVR)
