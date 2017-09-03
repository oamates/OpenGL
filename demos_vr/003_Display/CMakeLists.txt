add_executable (oculus003 main.cpp)

target_link_libraries(oculus003 LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} LibOVR)
