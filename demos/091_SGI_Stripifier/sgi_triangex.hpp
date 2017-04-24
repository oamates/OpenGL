//=======================================================================================================================================================================================================================
// STRIPE: sgi_triangex.hpp
//=======================================================================================================================================================================================================================

#ifndef SGI_TRIANGEX_INCLUDED
#define SGI_TRIANGEX_INCLUDED

#include <cstdio>
#include "queue.hpp"

int AdjacentEx(int id2,int id1, int *list, int size);
void Delete_From_ListEx(int id, int *list, int size);
void Triangulate_PolygonEx(int out_edge1, int out_edge2, int in_edge1, int in_edge2, int size, int* index, FILE* fp, int reversed, int face_id, int where);
void Non_Blind_TriangulateEx(int size, int* index, FILE* output, int next_face_id, int face_id, int where);
void Rearrange_IndexEx(int* index, int size);
void Blind_TriangulateEx(int size, int *index, bool begin, int where);
  
#endif
