//=======================================================================================================================================================================================================================
// STRIPE : init.hpp
//=======================================================================================================================================================================================================================

#ifndef INIT_INCLUDED
#define INIT_INCLUDED

#include "global.hpp"

void init_vert_norms(int num_vert);
void init_vert_texture(int num_vert);
void InitStripTable(  );
void Init_Table_SGI(int numfaces);
void Start_Vertex_Struct(int numverts);
void Start_Face_Struct(int numfaces);
void Start_Edge_Struct(int numverts);

#endif
