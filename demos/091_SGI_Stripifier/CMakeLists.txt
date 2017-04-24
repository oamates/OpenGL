add_executable (sgi_striper add.cpp common.cpp free.cpp global.cpp init.cpp local.cpp 
                            newpolve.cpp options.cpp output.cpp outputex.cpp 
                            partial.cpp queue.cpp sgi_triang.cpp sgi_triangex.cpp 
                            stripe.cpp struct.cpp structex.cpp ties.cpp util.cpp)

target_link_libraries(sgi_striper LINK_PUBLIC framework)
