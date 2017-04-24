//=======================================================================================================================================================================================================================
// STRIPE: structex.hpp
//=======================================================================================================================================================================================================================

#ifndef STRUCTEX_INCLUDED
#define STRUCTEX_INCLUDED

#include "polverts.hpp"
#include "queue.hpp"

int Get_EdgeEx(int* edge1, int* edge2, int* index, int face_id, int size, int id1, int id2);
void Delete_AdjEx(int id1, int id2, int* next_bucket, int* min_face, int current_face, int* e1, int* e2, int* ties);
int Change_FaceEx(int face_id, int in1, int in2, ListHead* pListHead, bool no_check);
int Update_AdjacenciesEx(int face_id, int* next_bucket, int* e1, int* e2, int* ties);
int Min_Face_AdjEx(int face_id, int* next_bucket, int* ties);

#endif
