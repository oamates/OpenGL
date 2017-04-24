add_executable (tri_striper connectivity_graph.cpp main.cpp policy.cpp tri_stripper.cpp)

target_link_libraries(tri_striper LINK_PUBLIC framework)
