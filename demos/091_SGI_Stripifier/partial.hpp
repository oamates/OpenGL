//=======================================================================================================================================================================================================================
// STRIPE : partial.hpp
//=======================================================================================================================================================================================================================

#ifndef PARTIAL_INCLUDED
#define PARTIAL_INCLUDED

#include <cstdio>
#include "polverts.hpp"
#include "queue.hpp"

void Partial_Triangulate(int size, int *index, FILE *output,int next_face_id,int face_id, int *next_id,ListHead *pListHead, P_ADJACENCIES temp, int where);
void Inside_Polygon(int size,int *index, int face_id,ListHead *pListHead, int where);

#endif
