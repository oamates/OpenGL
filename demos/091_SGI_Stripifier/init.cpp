//=======================================================================================================================================================================================================================
// STRIPE : global.hpp
// This file contains the initialization of data structures.
//=======================================================================================================================================================================================================================

#include <cstdio>
#include <cstdlib>
#include "init.hpp"
#include "global.hpp"
#include "polverts.hpp"
#include "queue.hpp"

void init_vert_norms(int num_vert)
{
    for (int x = 0; x < num_vert; x++)                                                                  /* Initialize vertex/normal array to have all zeros to start with. */
        *(vert_norms + x) = 0;
}

void init_vert_texture(int num_vert)
{
    for (int x = 0; x < num_vert; x++)                                                                  /* Initialize vertex/normal array to have all zeros to start with. */
        *(vert_texture + x) = 0;
}

static bool InitVertexTable(int nSize)
{
    Vertices = (ListHead**) malloc(sizeof(ListHead*) * nSize);                                          /* Initialize the face table */
    if (Vertices)
    {
        for (int nIndex = 0; nIndex < nSize; nIndex++ )
            Vertices[nIndex] = NULL;
        return true;
    }
    return false;
} 

static bool InitFaceTable (int nSize)
{
    PolFaces = (ListHead**) malloc(sizeof(ListHead*) * nSize);                                          /* Initialize the face table */
  
    if (PolFaces)
    {
        for (int nIndex = 0; nIndex < nSize; nIndex++ )
            PolFaces[nIndex] = NULL;
        return true;
    }
    return false;
} 

static bool InitEdgeTable( int nSize )
{
    PolEdges = (ListHead**) malloc(sizeof(ListHead*) * nSize);                                          /* Initialize the edge table */
    if (PolEdges)
    {
        for (int nIndex = 0; nIndex < nSize; nIndex++ )
            PolEdges[nIndex] = NULL;
        return true;
    }
    return false;
}


void InitStripTable(  )
{
    PLISTHEAD pListHead = ( PLISTHEAD ) malloc(sizeof(ListHead));                                       /* Initialize the strip table */
    if (pListHead)
    {
        InitList(pListHead);
        strips[0] = pListHead;
    }
    else
    {
        printf("Out of memory !\n");
        exit(0);
    }
}

void Init_Table_SGI(int numfaces)
{
    int max_adj = 60;
    for (int x = 0; x < max_adj; x++)                                                                   /* This routine will initialize the table that will have the faces sorted by the number of adjacent polygons to it. */
    {
        PLISTHEAD pListHead = (PLISTHEAD) malloc(sizeof(ListHead));                                     /* We are allowing the max number of sides of a polygon to be max_adj. */                      
        if (pListHead)
        {
            InitList( pListHead );
            array[x] = pListHead;
        }
        else
        {
            printf("Out of memory !\n");
            exit(0);
        }
    }
  
    if (face_array != NULL) free(face_array);                                                           /* It seems this function is called more than once so we'll free up the old stuff */   
    face_array = (P_FACE_ADJACENCIES) malloc (sizeof(FACE_ADJACENCIES) * numfaces); 
    if (face_array == NULL)
    {
        printf("Out of memory !!\n");
        exit(0);
    }
}



static void BuildVertexTable( int nSize )
{
    for (int nIndex = 0; nIndex < nSize; nIndex++)
    {
        PLISTHEAD pListHead = ( PLISTHEAD ) malloc(sizeof(ListHead));
        if (pListHead)
        {
            InitList(pListHead);
            Vertices[nIndex] = pListHead;
        }
        else
            return; 
    }
}

static void BuildFaceTable( int nSize )
{
    for (int nIndex = 0; nIndex < nSize; nIndex++ )
    {
        PLISTHEAD pListHead = (PLISTHEAD) malloc(sizeof(ListHead));
        if (pListHead)
        {
            InitList( pListHead );
            PolFaces[nIndex] = pListHead;
        }
        else
            return; 
    }
}

static void BuildEdgeTable( int nSize )
{
    for (int nIndex = 0; nIndex < nSize; nIndex++)
    {
        PLISTHEAD pListHead = ( PLISTHEAD ) malloc(sizeof(ListHead));
        if (pListHead)
        {
            InitList(pListHead);
            PolEdges[nIndex] = pListHead;
        }
        else
            return;
    }
}

void Start_Vertex_Struct(int numverts)
{
    if (InitVertexTable(numverts)) BuildVertexTable(numverts);
}

void Start_Face_Struct(int numfaces)
{
    if (InitFaceTable(numfaces)) BuildFaceTable(numfaces);
}

void Start_Edge_Struct(int numverts)
{
    if (InitEdgeTable(numverts)) BuildEdgeTable(numverts);
}
