//=======================================================================================================================================================================================================================
// STRIPE: structex.hpp
// This file contains routines that are used for various functions in the local algorithm.
//=======================================================================================================================================================================================================================

#include <cstdlib>
#include <cstring>

#include "log.hpp"
#include "common.hpp"
#include "polverts.hpp"
#include "queue.hpp"
#include "structex.hpp"
#include "ties.hpp"
#include "util.hpp"

static int out1Ex = -1;
static int out2Ex = -1;

int Get_EdgeEx(int* edge1, int* edge2, int* index, int face_id, int size, int id1, int id2)
{
    /* Put the edge that is adjacent to face_id into edge1 and edge2. For each edge see if it is adjacent to
    face_id. Id1 and id2 is the input edge, so see if the orientation is reversed, and save it in reversed. */
    int reversed = -1;
    bool set = false;
    int x;

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
                if ((out1Ex != -1) &&
                   ((out1Ex == *(index)) || (out1Ex == *(index+size-1))) &&
                   ((out2Ex == *(index)) || (out2Ex == *(index+size-1))))
                {
                    set = true;
                    *edge1 = *(index);
                    *edge2 = *(index + size - 1);
                }
                else if (out1Ex == -1)
                {
                    set = true;
                    *edge1 = *(index);
                    *edge2 = *(index + size - 1);
                }
                if ((reversed != -1) && (set))  
                    return reversed;
            }
        }       
        else
        {
            if ((*(index+x) == id1) && (*(index + x + 1) == id2))
            {
                if (set) return 0;
                reversed = 0;
            }
            else if ((*(index+x) == id2) && (*(index + x + 1) == id1))
            {
                if (set) return 1;
                reversed = 1;
            }
      
            if (Look_Up(*(index + x), *(index + x + 1), face_id))
            {
                if ((out1Ex != -1) &&
                   ((out1Ex == *(index + x)) || (out1Ex == *(index + x + 1))) &&
                   ((out2Ex == *(index + x)) || (out2Ex == *(index + x + 1))))
                {
                    set = true;
                    *edge1 = *(index + x);
                    *edge2 = *(index + x + 1);
                }
                else if (out1Ex == -1)
                {
                    set = true;
                    *edge1 = *(index + x);
                    *edge2 = *(index + x + 1);
                }
                if ((reversed != -1) && (set))
                    return reversed;
            }
        }
    }
           
    if ((x == size) && (reversed != -1))                                                                            /* Could not find the output edge */
        exit_msg("Error in the Lookup %d %d %d %d %d %d %d %d\n", face_id, id1, id2, reversed, *edge1, *edge2, out1Ex, out2Ex);
    return reversed;
}


static void Update_FaceEx(int* next_bucket, int* min_face, int face_id, int* e1, int* e2, int temp1, int temp2, int* ties)
{
    /* We have a face id that needs to be decremented. We have to determine where it is in the structure, so that we can decrement it. */
    /* The number of adjacencies may have changed, so to locate it may be a little tricky. However we know that the number of adjacencies is less than or equal to the original number of adjacencies */
    int y, size; 
    ListHead *pListHead;
    PF_FACES temp = NULL;
    PLISTINFO lpListInfo;
    static int each_poly = 0;
    bool there = false;
    
    pListHead = PolFaces[face_id];
    temp = (PF_FACES) PeekList(pListHead, LISTHEAD, 0);
                    

    if (temp != NULL)                                                                                               /* Check each edge of the face and tally the number of adjacent polygons to this face. */
    {
        size = temp->nPolSize;                                                                                      /* Size of the polygon */
        for (y = 0; y < size; y++)
        {
        
            if (y != (size - 1))                                                                                    /* If we are doing partial triangulation, we must check to see whether the edge is still there in the polygon, since we might have done a portion of the polygon and saved the rest for later. */
            {
                if (((temp1 == *(temp->pPolygon + y)) && (temp2 == *(temp->pPolygon + y + 1))) || ((temp2 == *(temp->pPolygon + y)) && (temp1 == *(temp->pPolygon + y + 1))))
                    there = true;                                                                                   /* edge is still there we are ok */
            }
            else
            {
                if (((temp1 == *(temp->pPolygon)) && (temp2 == *(temp->pPolygon + size - 1))) || ((temp2 == *(temp->pPolygon)) && (temp1 == *(temp->pPolygon + size - 1))))
                    there = true;                                                                                   /* edge is still there we are ok */
            }
        }
      
        if (!there) return;                                                                                         /* Original edge was already used, we cannot use this polygon */
        lpListInfo = Done(face_id, &y);                                                                             /* We have a starting point to start our search to locate this polygon. */
                                                                                                                    /* Check to see if this polygon was done */                               
        if (lpListInfo == NULL) return;
        
        if (y == 0) return;                                                                                         /* Was not done, but there is an error in the adjacency calculations */
                                                                                                                    /* If more than one edge is adj to it then maybe it was not updated */ 
        Add_Sgi_Adj(y - 1, face_id);                                                                                /* Now put the face in the proper bucket depending on tally. */  
        RemoveList(array[y], lpListInfo);                                                                           /* First add it to the new bucket, then remove it from the old */
      
        /* Save it if it was the smallest seen so far since then it will be the next face 
           Here we will have different options depending on what we want for resolving ties:
            1) First one we see we will use
            2) Random resolving
            3) Look ahead
            4) Alternating direction */
        
        if (*next_bucket == 60) *ties = *ties + each_poly;                                                          /* At a new strip */
        if (*next_bucket == (y - 1))                                                                                /* Have a tie */
        {
            Add_Ties(face_id);
            each_poly++;
        }
        if (*next_bucket > (y - 1))                                                                                 /* At a new minimum */
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

void Delete_AdjEx(int id1, int id2, int* next_bucket, int* min_face, int current_face, int* e1, int* e2, int* ties)
{    
    int count = 0;                                                                                                  /* Find the face that is adjacent to the edge and is not the current face. Delete one adjacency from it. Save the min adjacency seen so far. */
    PF_EDGES temp = NULL;
    ListHead *pListHead;
    int next_face;
    
    switch_lower(&id1, &id2);                                                                                       /* Always want smaller id first */
  
    pListHead = PolEdges[id1];
    temp = (PF_EDGES) PeekList(pListHead,LISTHEAD,count);
    if (temp == NULL) return;
    
    while (temp->edge[0] != id2)                                                                                    /* It could be a new edge that we created. So we can exit, since there is not a face adjacent to it. */
    {
      count++;
      temp = (PF_EDGES) GetNextNode(temp);                                      
      if (temp == NULL) return;                                                                                     /* Was a new edge that was created and therefore does not have anything adjacent to it */    
    }
  
    if (temp->edge[2] == -1) return;                                                                                /* Was not adjacent to anything else except itself */
    else                                                                                                            /* Was adjacent to something */
    {
        if (temp->edge[2] == current_face)
            next_face = temp->edge[1];
        else 
            next_face = temp->edge[2];
    }
    Update_FaceEx(next_bucket, min_face, next_face, e1, e2, id1, id2, ties);                                        /* We have the other face adjacent to this edge, it is next_face. Now we need to decrement this faces' adjacencies. */
}

int Change_FaceEx(int face_id, int in1, int in2, ListHead* pListHead, bool no_check)
{
    /* We are doing a partial triangulation and we need to put the new face of triangle into the correct bucket */
    int input_adj, y;
    P_ADJACENCIES lpListInfo;  
  
    /* Find the old number of adjacencies to this face, so we know where to delete it from */
    y = Old_Adj(face_id);
    pListHead = array[y];
    lpListInfo = face_array[face_id].pfNode;
  
    if (lpListInfo == NULL)
        exit_msg("There is an error finding the next polygon3 %d\n", face_id);
  
    /* Do we need to change the adjacency? Maybe the edge on the triangle that was outputted was not adjacent to anything. We know if we
    have to check by "check". We came out on the output edge that we needed, then we know that the adjacencies will decrease by exactly one. */
    if (!no_check)
    {
        input_adj = Number_Adj(in1, in2, face_id);                                                                  /* If there weren't any then don't do anything */
        if (input_adj == 0) return y;
    }
  
    if (face_array[lpListInfo->face_id].head == pListHead)
        face_array[lpListInfo->face_id].pfNode = NULL;
    RemoveList(pListHead,(PLISTINFO)/*(temp*/lpListInfo);
    // Before we had a quad with y adjacencies. The in edge did not have an adjacency, since it was just deleted, since we came in on it. 
    // The outedge must have an adjacency otherwise we would have a bucket 0, and would not be in this routine. Therefore the new adjacency must be y - 1
  
    Add_Sgi_Adj(y - 1, face_id);
    return y - 1;
}

int Update_AdjacenciesEx(int face_id, int* next_bucket, int* e1, int* e2, int* ties)
{
    /* Give the face with id face_id, we want to decrement all the faces that are adjacent to it, since we will be deleting face_id from the data structure.
       We will return the face that has the least number of adjacencies. */
    PF_FACES temp = NULL;
    ListHead *pListHead;
    int size, y, min_face = -1;
  
    *next_bucket = 60;
    pListHead = PolFaces[face_id];
    temp = (PF_FACES) PeekList(pListHead, LISTHEAD, 0);
  
    if (temp == NULL)
        exit_msg("The face was already deleted, there is an error\n");
    
    size = temp->nPolSize;                                                                                          /* Size of the polygon */
    for (y = 0; y< size; y++)
    {
        if (y != (size - 1))
            Delete_AdjEx(*(temp->pPolygon + y), *(temp->pPolygon + y + 1), next_bucket, &min_face, face_id, e1, e2, ties);
        else
            Delete_AdjEx(*(temp->pPolygon), *(temp->pPolygon + (size - 1)), next_bucket, &min_face, face_id, e1, e2, ties);
    }
    return (min_face);
}

static void Find_Adj_TallyEx(int id1, int id2, int* next_bucket, int* min_face, int current_face, int* ties)
{
    int size, each_poly = 0, y, count = 0;                                                                          /* Find the face that is adjacent to the edge and is not the current face. Save the min adjacency seen so far. */
    PF_EDGES temp = NULL;
    PF_FACES temp2 = NULL;
    ListHead* pListHead;
    int next_face;
    bool there = false;
    
    switch_lower(&id1,&id2);                                                                                        /* Always want smaller id first */
  
    pListHead = PolEdges[id1];
    temp = (PF_EDGES) PeekList(pListHead,LISTHEAD,count);
    if (temp == NULL) return;                                                                                       /* This was a new edge that was created, so it is adjacent to nothing. */
    
    while (temp->edge[0] != id2)
    {
        count++;
        temp = (PF_EDGES) GetNextNode(temp);                                       
        if (temp == NULL) return;                                                                                   /* This was a new edge that we created */
    }
  
    if (temp->edge[2] == -1) return;                                                                                /* Was not adjacent to anything else except itself */
    else
    {
        if (temp->edge[2] == current_face)
            next_face =  temp->edge[1];
        else 
            next_face = temp->edge[2];
    }
    
    pListHead = PolFaces[next_face];                                                                                /* We have the other face adjacent to this edge, it is next_face. Find how many faces it is adjacent to. */
    temp2 = (PF_FACES) PeekList(pListHead, LISTHEAD, 0);
    /* Check each edge of the face and tally the number of adjacent polygons to this face. This will be the original number of
       polygons adjacent to this polygon, we must then see if this number has been decremented */                
    if (temp2 != NULL)
    {      
        size = temp2->nPolSize;                                                                                     /* Size of the polygon */
        for (y = 0; y < size; y++)
        {
      
            if (y != (size - 1))                                                                                    /* Make sure that the edge is still in the polygon and was not deleted, because if the edge was deleted, then we used it already. */
            {
                if (((id1 == *(temp2->pPolygon + y)) && (id2 ==*(temp2->pPolygon + y + 1))) || ((id2 == *(temp2->pPolygon + y)) && (id1 == *(temp2->pPolygon + y + 1))))
                    there = true;                                                                                   /* edge is still there we are ok */
            }
            else
            {       
                if (((id1 == *(temp2->pPolygon)) && (id2 == *(temp2->pPolygon + size - 1))) || ((id2 == *(temp2->pPolygon)) && (id1 == *(temp2->pPolygon + size - 1))))
                    there = true;                                                                                   /* edge is still there we are ok */
            }
        }
      
        if (!there) return;                                                                                         /* Edge already used and deleted from the polygon */
        if (Done(next_face,&y) == NULL) return;                                                                     /* See if the face was already deleted, and where it is if it was not */
      
        /* Save it if it was the smallest seen so far since then it will be the next face 
           Here we will have different options depending on what we want for resolving ties:
            1) First one we see we will use
            2) Random resolving
            3) Look ahead
            4) Alternating direction */
      
        if (*next_bucket == 60) *ties = *ties + each_poly;                                                          /* At a new strip */
        if (*next_bucket == (y - 1))                                                                                /* Have a tie */
        {
            Add_Ties(next_face);
            each_poly++;
        }
        if (*next_bucket > (y-1))                                                                                   /* At a new minimum */
        {
            *next_bucket = y-1;
            *min_face = next_face;
            each_poly = 0;
            Clear_Ties();
            Add_Ties(next_face);
        }
    }
}

int Min_Face_AdjEx(int face_id, int *next_bucket, int *ties)
{    
    PF_FACES temp = NULL;                                                                                           /* Used for the Partial triangulation to find the next face. It will return the minimum adjacency face id found at this face. */
    ListHead *pListHead;
    int size, y, min_face, test_face;
  
    *next_bucket = 60;
    pListHead = PolFaces[face_id];
    temp = (PF_FACES) PeekList(pListHead, LISTHEAD, 0);
  
    if (temp == NULL)
        exit_msg("The face was already deleted, there is an error\n");
  
  
    size = temp->nPolSize;                                                                                          /* Size of the polygon */
    for (y = 0; y < size; y++)
    {
        if (y != (size - 1))
            Find_Adj_TallyEx(*(temp->pPolygon + y), *(temp->pPolygon + y + 1), next_bucket, &min_face, face_id, ties);
        else
            Find_Adj_TallyEx(*(temp->pPolygon), *(temp->pPolygon + (size - 1)), next_bucket, &min_face, face_id, ties);
    }
    
    if (size == 4)                                                                                                  /* Maybe we can do better by triangulating the face, because by triangulating the face we will go to a polygon of lesser adjacencies */
    {      
        Check_In_Quad(face_id, &test_face);                                                                         /* Checking for a quad whether to do the whole polygon will result in better performance because the triangles in the polygon have less adjacencies */
        if (*next_bucket > test_face) min_face = face_id;                                                           /* We can do better by going through the polygon */
    }
    else                                                                                                            /* We have a polygon with greater than 4 sides, check to see if going inside is better than going outside the polygon for the output edge. */
    {
        Check_In_Polygon(face_id,&test_face,size);
        if (*next_bucket > test_face)
        min_face = face_id;                                                                                         /* We can do better by going through the polygon */
    }
    return (min_face);
}
