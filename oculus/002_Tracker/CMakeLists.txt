add_executable (oculus002 main.cpp)

target_link_libraries(oculus002 LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} OVR)
