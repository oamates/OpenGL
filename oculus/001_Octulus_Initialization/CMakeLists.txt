add_executable (oculus001 main.cpp)

target_link_libraries(oculus001 LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} OVR)
