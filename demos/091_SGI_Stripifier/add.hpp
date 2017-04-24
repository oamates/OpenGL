//=======================================================================================================================================================================================================================
// STRIPE : add.hpp
//=======================================================================================================================================================================================================================

#ifndef ADD_INCLUDED
#define ADD_INCLUDED

#include "global.hpp"

bool norm_array(int id, int vertex, double normal_difference, struct vert_struct *n, int num_vert);
void add_texture(int id, bool vertex);
int  add_vert_id(int id, int index_count);
void add_norm_id(int id, int index_count);
void AddNewFace(int ids[MAX1], int vert_count, int face_id, int norms[MAX1]);
void CopyFace(int ids[MAX1], int vert_count, int face_id, int norms[MAX1]);
void Add_AdjEdge(int v1,int v2, int fnum, int index1);

#endif
