//=======================================================================================================================================================================================================================
// STRIPE : options.hpp
//=======================================================================================================================================================================================================================

#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED

enum file_options {ASCII, BINARY};
enum tie_options {FIRST, RANDOM, ALTERNATE, LOOK, SEQUENTIAL};
enum triangulation_options {PARTIAL, WHOLE};

void print_usage(void);
float get_options(int argc, char** argv, int* f, int* t, int* tr, int* group, int* orientation);

#endif
     
