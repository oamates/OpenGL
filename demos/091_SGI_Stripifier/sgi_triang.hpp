//=======================================================================================================================================================================================================================
// STRIPE: sgi_triang.hpp
//=======================================================================================================================================================================================================================

#ifndef SGI_TRIANG_INCLUDED
#define SGI_TRIANG_INCLUDED

#include <cstdio>
#include "global.hpp"

int Adjacent(int id2,int id1, int *list, int size);
void Build_SGI_Table(int num_faces);
void Non_Blind_Triangulate(int size, int *index, FILE* output, int next_face_id, int face_id, int where, int color1, int color2, int color3);
void Blind_Triangulate(int size, int *index, bool begin, int where);

#endif
