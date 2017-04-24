//=======================================================================================================================================================================================================================
// STRIPE: struct.hpp
// Contains routines that update structures, and miscellaneous routines.
//=======================================================================================================================================================================================================================

#include <cstdlib>
#include <cstring>

#include "log.hpp"
#include "common.hpp"
#include "struct.hpp"
#include "ties.hpp"
#include "util.hpp"

static int out1 = -1;
static int out2 = -1;

int Get_Edge(int *edge1,int *edge2,int *index,int face_id, int size, int id1, int id2)
{
    /* Put the edge that is adjacent to face_id into edge1 and edge2. For each edge see if it is adjacent to face_id. Id1 and id2 is the input edge, so see if the orientation is reversed, and save it in reversed. */
    int x;
    int reversed = -1;
    bool set = false;
  
    for (x = 0; x < size; x++)
    {
        if (x == (size - 1))
        {
            if ((*(index) == id1) && (*(index + size - 1) == id2))
            {
                if (set) return 1;
                reversed = 1;
            }
            else if ((*(index) == id2) && (*(index + size - 1) == id1))
            {
                if (set) return 0;
                reversed = 0;
            }
      
            if (Look_Up(*(index), *(index + size - 1), face_id))
            {
                if ((out1 != -1) && ((out1 == *(index)) || (out1 == *(index+size-1))) && ((out2 == *(index)) || (out2 == *(index + size - 1))))
                {
                    set = true;
                    *edge1 = *(index);
                    *edge2 = *(index + size - 1);
                }
                else if (out1 == -1)
                {
                    set = true;
                    *edge1 = *(index);
                    *edge2 = *(index + size - 1);
                }
                if ((reversed != -1) && (set)) return reversed;
            }
        }       
        else
        {
            if ((*(index + x) == id1) && (*(index + x + 1) == id2))
            {
                if (set) return 0;
                reversed = 0;
            }
            else if ((*(index + x) == id2) && (*(index + x + 1) == id1))
            {
                if (set) return 1;
                reversed = 1;
            }
      
            if (Look_Up(*(index+x),*(index+x+1),face_id))
            {
                if ((out1 != -1) && ((out1 == *(index + x)) || (out1 == *(index + x + 1))) && ((out2 == *(index + x)) || (out2 == *(index + x + 1))))
                {
                    set = true;
                    *edge1 = *(index + x);
                    *edge2 = *(index + x + 1);
                }
                else if (out1 == -1)
                {
                    set = true;
                    *edge1 = *(index + x);
                    *edge2 = *(index + x + 1);
                }
                if ((reversed != -1) && (set)) return reversed;
            }
        }
    }           
    if ((x == size) && (reversed != -1))                                                                                /* Could not find the output edge */
        exit_msg("Error in the Lookup %d %d %d %d %d %d %d %d\n", face_id, id1, id2, reversed, *edge1, *edge2, out1, out2);
    return reversed;
}


static void Update_Face(int* next_bucket, int* min_face, int face_id, int *e1, int* e2, int temp1, int temp2, int* ties)
{
    /* We have a face id that needs to be decremented. We have to determine where it is in the structure, so that we can decrement it. */
    /* The number of adjacencies may have changed, so to locate it may be a little tricky. However we know that the number of adjacencies is less than or equal to the original number of adjacencies */
    int y,size;
    ListHead *pListHead;
    PF_FACES temp = NULL;
    PLISTINFO lpListInfo;
    static int each_poly = 0;
    bool there = false;
  
    pListHead = PolFaces[face_id];
    temp = ( PF_FACES ) PeekList( pListHead, LISTHEAD, 0 );
                    
    if (temp != NULL)                                                                                                   /* Check each edge of the face and tally the number of adjacent polygons to this face. */
    {
        size = temp->nPolSize;                                                                                          /* Size of the polygon */
        if (size == 1) return;                                                                                          /* We did it already */
        for (y = 0; y< size; y++)
        {
            /* If we are doing partial triangulation, we must check to see whether the edge is still there in the polygon, since we might have done a portion of the polygon and saved the rest for later. */
            if (y != (size-1))
            {
                if(((temp1 == *(temp->pPolygon + y)) && (temp2 ==*(temp->pPolygon + y + 1))) || ((temp2 == *(temp->pPolygon + y)) && (temp1 == *(temp->pPolygon + y + 1))))
                    there = true;                                                                                       /* edge is still there we are ok */
            }
            else
            {
                if( ((temp1 == *(temp->pPolygon)) && (temp2 == *(temp->pPolygon+size-1))) || ((temp2 == *(temp->pPolygon)) && (temp1 == *(temp->pPolygon + size - 1))))
                    there = true;                                                                                       /* edge is still there we are ok */
            }
        }
      
        if (!there) return;                                                                                             /* Original edge was already used, we cannot use this polygon */
                                                                                                                        /* We have a starting point to start our search to locate this polygon. */
        lpListInfo = Done(face_id,&y);                                                                                  /* Check to see if this polygon was done */
        if (lpListInfo == NULL) return;
      
      
        if (y == 0)                                                                                                     /* Was not done, but there is an error in the adjacency calculations */
            exit_msg("There is an error in finding the adjacencies\n");
                                                                                                                        
        Add_Sgi_Adj(y - 1, face_id);                                                                                    /* Now put the face in the proper bucket depending on tally. */  
        RemoveList(array[y], lpListInfo);                                                                               /* First add it to the new bucket, then remove it from the old */
      
        /* Save it if it was the smallest seen so far since then it will be the next face 
        Here we will have different options depending on what we want for resolving ties:
        1) First one we see we will use
        2) Random resolving
        3) Look ahead
        4) Alternating direction */
      
        if (*next_bucket == 60)                                                                                         /* At a new strip */
            *ties = *ties + each_poly;
      
        if (*next_bucket == (y - 1))                                                                                    /* Have a tie */
        {
            Add_Ties(face_id);
            each_poly++;
        }
      
        if (*next_bucket > (y - 1))                                                                                     /* At a new minimum */
        {
            *next_bucket = y - 1;
            *min_face = face_id;
            *e1 = temp1;
            *e2 = temp2;
            each_poly = 0;
            Clear_Ties();
            Add_Ties(face_id);
        }
    }
}

static void Delete_Adj(int id1, int id2, int* next_bucket, int* min_face, int current_face, int *e1, int *e2, int *ties)
{
    /* Find the face that is adjacent to the edge and is not the current face. Delete one adjacency from it. Save the min adjacency seen so far. */
    int count = 0;
    PF_EDGES temp = NULL;
    ListHead* pListHead;
    int next_face;
  
    switch_lower(&id1, &id2);                                                                                           /* Always want smaller id first */
  
    pListHead = PolEdges[id1];
    temp = (PF_EDGES) PeekList(pListHead, LISTHEAD, count);
    if (temp == NULL) return;                                                                                           /* It could be a new edge that we created. So we can exit, since there is not a face adjacent to it. */
    
    while (temp->edge[0] != id2)
    {
        count++;
        temp = (PF_EDGES) GetNextNode(temp);                                        
        if (temp == NULL) return;                                                                                       /* Was a new edge that was created and therefore does not have anything adjacent to it */
    }
  
    if (temp->edge[2] == -1) return;                                                                                    /* Was not adjacent to anything else except itself */
    else                                                                                                                /* Was adjacent to something */
    {
        if (temp->edge[2] == current_face)
            next_face = temp->edge[1];
        else 
            next_face = temp->edge[2];
    }  
    Update_Face(next_bucket, min_face, next_face, e1, e2, id1, id2, ties);                                              /* We have the other face adjacent to this edge, it is next_face. Now we need to decrement this faces' adjacencies. */
}

int Update_Adjacencies(int face_id, int* next_bucket, int* e1, int* e2, int* ties)
{
    /* Give the face with id face_id, we want to decrement all the faces that are adjacent to it, since we will be deleting face_id from the data structure. We will return the face that has the least number of adjacencies. */

    PF_FACES temp = NULL;
    ListHead *pListHead;
    int size, y, min_face = -1;
  
    *next_bucket = 60;
    pListHead = PolFaces[face_id];
    temp = (PF_FACES) PeekList(pListHead, LISTHEAD, 0);
  
    if (temp == NULL)
        exit_msg("The face was already deleted, there is an error\n");
      
    size = temp->nPolSize;                                                                                              /* Size of the polygon */
    for (y = 0; y < size; y++)
    {
        if (y != (size-1))
            Delete_Adj(*(temp->pPolygon + y),*(temp->pPolygon + y + 1), next_bucket, &min_face, face_id, e1, e2, ties);
        else
            Delete_Adj(*(temp->pPolygon),*(temp->pPolygon+(size-1)), next_bucket, &min_face, face_id, e1, e2, ties);
    }
    return (min_face);
}