//=======================================================================================================================================================================================================================
// STRIPE : global.hpp
//=======================================================================================================================================================================================================================

#ifndef GLOBAL_INCLUDED 
#define GLOBAL_INCLUDED

#include "polverts.hpp"

enum swap_type {ON, OFF};

#define   VRDATA        double
#define   MAX1            60
    
#define   PI            3.1415926573
#define   ATOI(C)        (C -'0')
#define   X              0
#define   Y              1
#define   Z              2
#define   EVEN(x)       (((x) & 1) == 0)
#define   MAX_BAND      10000

struct vert_struct
{
    VRDATA  x, y, z;    /* point coordinates */
};

extern int ids[MAX1];
extern int norms[MAX1];
extern int *vert_norms;
extern int *vert_texture;

#endif
