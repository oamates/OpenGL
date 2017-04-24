//=======================================================================================================================================================================================================================
// STRIPE : util.cpp
//=======================================================================================================================================================================================================================

#include <cstdlib>
#include "util.hpp"
#include "polverts.hpp"

void switch_lower (int *x, int *y)                                                              /* Put lower value in x */
{
    if (*y < *x)
    {
        int t = *x;
        *x = *y;
        *y = t;
    }
}

bool member(int x, int id1, int id2, int id3)                                                  /* Is x in the triangle specified by id1,id2,id3 */
{
    return (x == id1) || (x == id2) || (x == id3);
}

bool Exist(int face_id, int id1, int id2)                                                       /* Does the edge specified by id1 and id2 exist in this face currently? Maybe we deleted in partial triangulation */
{
    ListHead *pListHead;
    PF_FACES temp;
    register int x, size;
    bool a = false, b = false; 

    pListHead = PolFaces[face_id];
    temp = ( PF_FACES ) PeekList( pListHead, LISTHEAD, 0);
    size = temp->nPolSize;
    for (x = 0; x < size; x++)
    {
        if (*(temp->pPolygon + x) == id1)
            a = true;
        if (*(temp->pPolygon + x) == id2)
            b = true;
        if (a && b) return true;
    }
    return false;
}



int Different (int id1, int id2, int id3, int id4, int id5, int id6, int* x, int* y)            /* Find the vertex in the first 3 numbers that does not exist in the last three numbers */
{
  
    if ((id1 != id4) && (id1 != id5) && (id1 != id6)) { *x = id2; *y = id3; return id1; }
    if ((id2 != id4) && (id2 != id5) && (id2 != id6)) { *x = id1; *y = id3; return id2; }
    if ((id3 != id4) && (id3 != id5) && (id3 != id6)) { *x = id1; *y = id2; return id3; }
  
    *x = id5; *y = id6; return id4;                                                             /* Because there are degeneracies in the data, this might occur */
}

int Return_Other(int* index, int e1, int e2)                                                    /* We have a triangle and want to know the third vertex of it */
{
    for (int x = 0; x < 3; x++)
    {
        if ((*(index + x) != e1) && (*(index + x) != e2))
        return *(index + x);
    }
    return e1;                                                                                  /* If there is a degenerate triangle return arbitrary */
}

int Get_Other_Vertex(int id1, int id2, int id3, int* index)                                     /* We have a list index of 4 numbers and we wish to return the number that is not id1, id2 or id3 */
{
    int x;
    for (x = 0; x < 4; x++)
    {
        if ((*(index + x) != id1) && (*(index + x) != id2) && (*(index + x) != id3))
            return *(index + x);
    }
    
    if (x == 4) return id1;                                                                     /* If there is some sort of degeneracy this might occur, return arbitrary */
}

PLISTINFO Done(int face_id, int *bucket)
{
                                                                                                /* Check to see whether the polygon with face_id was used already, return NULL if it was, otherwise return a pointer to the face. */
    PLISTINFO lpListInfo;
    lpListInfo = (PLISTINFO) face_array[face_id].pfNode;
    if (lpListInfo != NULL)
        *bucket = face_array[face_id].bucket;
    return lpListInfo;
}

void First_Edge(int *id1,int *id2, int *id3)
{
                                                                                                /* Get the first triangle in the strip we just found, we will use this to try to extend backwards in the strip */
    ListHead *pListHead;
    register int num;
    P_STRIPS temp1,temp2,temp3;
    pListHead = strips[0];
    num = NumOnList(pListHead);
  
    if (num < 3) return;                                                                        /* Did not have a strip */
  
    temp1 = ( P_STRIPS ) PeekList( pListHead, LISTHEAD, 0);
    temp2 = ( P_STRIPS ) PeekList( pListHead, LISTHEAD, 1);
    temp3 = ( P_STRIPS ) PeekList( pListHead, LISTHEAD, 2);
    *id1 = temp1->face_id;
    *id2 = temp2->face_id;
    *id3 = temp3->face_id;
}

void Last_Edge(int *id1, int *id2, int *id3, bool save)
{
    static int v1, v2, v3;                                                                      /* We need the last edge that we had */
  
    if (save)
    {
        v1 = *id1;
        v2 = *id2;
        v3 = *id3;
    }
    else
    {
        *id1 = v1;
        *id2 = v2;
        *id3 = v3;
    }
}

static void find_triangle_orientation(int vertex1, int vertex2, int vertex3, int* original_vertex)                                 
{
    int vertices,index;
    PF_VERTICES verts;
                                                                                                /* Search through face to match original vertices */
  
    verts = (PF_VERTICES) PeekList(Vertices[vertex1 - 1], LISTHEAD, 0);                         /* Start with vertex1's Vertices struct */  
    do
    {  
        index = 0;
        for (vertices = 0; vertices < verts->face->nOrgSize;vertices++)
        {
            if (vertex1 == verts->face->pPolygon[vertices] + 1 || 
                vertex2 == verts->face->pPolygon[vertices] + 1 ||
                vertex3 == verts->face->pPolygon[vertices] + 1 )
                original_vertex[index++] = verts->face->pPolygon[vertices] + 1;      
            if (index == 3) break;
        }
        if (index == 3) break;
    } 
    while ((verts = (PF_VERTICES) GetNextNode(verts)) != NULL);
  
    if (index != 3)
    {
        verts = (PF_VERTICES) PeekList(Vertices[vertex2 - 1], LISTHEAD, 0);                     /* Search vertex2's Vertices struct */  
        do
        {
            index = 0;
            for (vertices = 0; vertices < verts->face->nOrgSize;vertices++)
            {
                if (vertex1 == verts->face->pPolygon[vertices] + 1 || 
                    vertex2 == verts->face->pPolygon[vertices] + 1 ||
                    vertex3 == verts->face->pPolygon[vertices] + 1 )
                    original_vertex[index++] = verts->face->pPolygon[vertices] + 1;      
                if (index == 3) break;
            }
            if (index == 3) break;
        }
        while ((verts = (PF_VERTICES) GetNextNode(verts)) != NULL);
    }
  
    if (index != 3)                                                                             /* Search vertex3's Vertices struct */  
    {
        
        verts = (PF_VERTICES) PeekList(Vertices[vertex3-1],LISTHEAD,0);
        do
        {    
            index = 0;
            for (vertices = 0; vertices < verts->face->nOrgSize;vertices++)
            {
                if (vertex1 == verts->face->pPolygon[vertices] + 1 || 
                    vertex2 == verts->face->pPolygon[vertices] + 1 ||
                    vertex3 == verts->face->pPolygon[vertices] + 1 )
                    original_vertex[index++] = verts->face->pPolygon[vertices]+1;      
                if (index == 3) break;
            }
            if (index == 3) break;
        }
        while ((verts = (PF_VERTICES) GetNextNode(verts)) != NULL);
    }
}


void preserve_strip_orientation_with_normal(FILE *output, int vertex1, int normal1, int vertex2, int normal2, int vertex3, int normal3)
{
    int original_vertex[3];
    find_triangle_orientation(vertex1, vertex2, vertex3, original_vertex);
  
    if ((original_vertex[0] == vertex3 && original_vertex[1] == vertex2 && original_vertex[2] == vertex1) ||
        (original_vertex[0] == vertex2 && original_vertex[1] == vertex1 && original_vertex[2] == vertex3) ||
        (original_vertex[0] == vertex1 && original_vertex[1] == vertex3 && original_vertex[2] == vertex2))
    {
        fprintf(output," %d//%d",vertex2,normal2);                                                                  /* New Triangle is in an opposite orientation. Add vertex2 to correct it */
    }
}

void preserve_strip_orientation_with_texture(FILE *output, int vertex1, int texture1, int vertex2, int texture2, int vertex3, int texture3)
{
    int original_vertex[3];
    find_triangle_orientation(vertex1, vertex2, vertex3, original_vertex);
  
    if ((original_vertex[0] == vertex3 && original_vertex[1] == vertex2 && original_vertex[2] == vertex1) ||
        (original_vertex[0] == vertex2 && original_vertex[1] == vertex1 && original_vertex[2] == vertex3) ||
        (original_vertex[0] == vertex1 && original_vertex[1] == vertex3 && original_vertex[2] == vertex2))
    {
        /* New Triangle is in an opposite orientation. Add vertex2 to correct it */
        fprintf(output," %d/%d",vertex2,texture2);
    }
}

void preserve_strip_orientation_with_texture_and_normal(FILE *output, int vertex1, int texture1, int normal1, int vertex2, int texture2, int normal2, int vertex3, int texture3, int normal3)
{
    int original_vertex[3];
    find_triangle_orientation(vertex1, vertex2, vertex3, original_vertex);
  
    if ((original_vertex[0] == vertex3 && original_vertex[1] == vertex2 && original_vertex[2] == vertex1) ||
        (original_vertex[0] == vertex2 && original_vertex[1] == vertex1 && original_vertex[2] == vertex3) ||
        (original_vertex[0] == vertex1 && original_vertex[1] == vertex3 && original_vertex[2] == vertex2)) 
    {
        /* New Triangle is in an opposite orientation. Add vertex2 to correct it */
        fprintf(output," %d/%d/%d",vertex2,texture2,normal2);
    }
}

void preserve_strip_orientation(FILE *output,int vertex1, int vertex2,int vertex3)
{ 
    int original_vertex[3];
    find_triangle_orientation(vertex1, vertex2, vertex3, original_vertex);
  
    if ((original_vertex[0] == vertex3 && original_vertex[1] == vertex2 && original_vertex[2] == vertex1) ||
        (original_vertex[0] == vertex2 && original_vertex[1] == vertex1 && original_vertex[2] == vertex3) ||
        (original_vertex[0] == vertex1 && original_vertex[1] == vertex3 && original_vertex[2] == vertex2))
    {
        /* New Triangle is in an opposite orientation. Add vertex2 to correct it */
        fprintf(output," %d",vertex2);
    }
}
