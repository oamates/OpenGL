add_executable (nvtristriper nvtristrip.cpp nvtristripobjects.cpp main.cpp)

target_link_libraries(nvtristriper LINK_PUBLIC framework)
