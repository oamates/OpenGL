//---------------------------------------------------------------------
//   STRIPE: polverts.h
//---------------------------------------------------------------------

#ifndef POLVERTS_INCLUDED
#define POLVERTS_INCLUDED

#include "queue.hpp"
    
typedef struct adjacencies
{
    Node ListNode;
    int face_id;
} ADJACENCIES, *P_ADJACENCIES;

typedef struct FVerts
{
    Node ListNode;
    int *pPolygon;
    int nPolSize;
    int nId;
} F_VERTS, *PF_VERTS;

/*Every time we need to use this, cast it ( ListInfo*)*/

typedef struct FEdges
{
    Node ListNode;
    int edge[3];
} F_EDGES, *PF_EDGES;

typedef struct FFaces
{
    Node ListNode;
    int *pPolygon;
    int *pNorms;
    int seen;
    int seen2;
    int seen3;
    int nPolSize;
    int nOrgSize;
    F_EDGES **VertandId;
    int *marked;
        int *walked;
} F_FACES,*PF_FACES;
    
typedef struct FVertices 
{ 
    Node ListNode;
    PF_FACES face;
} F_VERTICES, *PF_VERTICES;

typedef struct Strips
{
    Node ListNode;
    int face_id;
} Strips,*P_STRIPS;

struct vert_added
{
    int num;
    int *normal;
};


typedef struct face_adjacencies
{
    P_ADJACENCIES pfNode;
    int bucket;
    ListHead *head;
} FACE_ADJACENCIES, *P_FACE_ADJACENCIES;

// Globals

extern int num_faces;
extern ListHead **PolFaces;
extern ListHead **PolEdges;
extern ListHead *array[60];
extern P_FACE_ADJACENCIES face_array;  /* Pointers from face_id to face   */
extern ListHead **Vertices;            /* Pointers from vertex_id to face */
extern ListHead *strips[1];
extern int orient;
#endif
