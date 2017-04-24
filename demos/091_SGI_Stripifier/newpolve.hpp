//=======================================================================================================================================================================================================================
// STRIPE : newpolve.hpp
//=======================================================================================================================================================================================================================

#ifndef NEWPOLVE_INCLUDED
#define NEWPOLVE_INCLUDED

#include <cstdio>
#include "global.hpp"
#include "polverts.hpp"

void Find_Bands(int numfaces, FILE* output_file, int* swaps, int* bands, int* cost, int* tri, int norms, int* vert_norms, int texture, int* vert_texture);
void Save_Walks(int numfaces);
void Save_Rest(int* numfaces);

#endif

