//=======================================================================================================================================================================================================================
// STRIPE : common.hpp
// This file contains the code used to free the data structures.
//=======================================================================================================================================================================================================================

#include <cstdio>
#include <cstdlib>
#include "free.hpp"
#include "polverts.hpp"

static void ParseAndFreeList(ListHead* pListHead)                                                   /* Freeing a linked list */
{
    PLISTINFO value;
    int num = NumOnList(pListHead);
    for (int c = 0; c < num; c++)
        value = RemHead(pListHead);
} 

void Free_Strips()                                                                                  /* Free strips data structure */
{
    if (strips[0] == NULL) return;
    ParseAndFreeList(strips[0]);
}

static void FreeFaceTable(int nSize)
{
    for (int nIndex=0; nIndex < nSize; nIndex++)
    { 
        if (PolFaces[nIndex] != NULL) 
            ParseAndFreeList(PolFaces[nIndex]);
    }
    free(PolFaces);
}

static void FreeEdgeTable(int nSize)
{
    for (int nIndex = 0; nIndex < nSize; nIndex++)
    {
        if (PolEdges[nIndex] != NULL)
            ParseAndFreeList(PolEdges[nIndex]);
    }
    free(PolEdges);
}

void End_Face_Struct(int numfaces)
{
    FreeFaceTable(numfaces);
}

void End_Edge_Struct(int numverts)
{
    FreeEdgeTable(numverts);
}
