//=======================================================================================================================================================================================================================
// STRIPE : ties.cpp
// This file will contain all the routines used to determine the next face if there is a tie
//=======================================================================================================================================================================================================================

#include <cstdlib>

#include "common.hpp"
#include "options.hpp"
#include "polverts.hpp"
#include "queue.hpp"
#include "sgi_triangex.hpp"
#include "structex.hpp"
#include "ties.hpp"
#include "util.hpp"

#define MAX_TIE 60
static int ties_array[60];
static int last = 0;

void Clear_Ties()
{
    last = 0;                                                                                               /* Clear the buffer, because we do not have the tie any more that we had before */
}

void Add_Ties(int id)
{
    ties_array[last++] = id;                                                                                /* We have a tie to add to the buffer */
}

static int Alternate_Tie()
{
    static int x = 0;                                                                                       /* Alternate in what we choose to break the tie. We are just alternating between the first and second thing that we found */
    int t = ties_array[x];
    x++;
    if (x == 2) x = 0;
    return t;
}

static int Random_Tie()
{
    int num = rand();                                                                                       /* Randomly choose the next face with which to break the tie */
    while (num >= last) num = num / 20;
    return (ties_array[num]);
}

static int Look_Ahead(int id)
{
    return Min_Adj(id);                                                                                     /* Look ahead at this face and save the minimum adjacency of all the faces that are adjacent to this face. */
}

static int Random_Look(int id[],int count)
{
    int num = rand();                                                                                       /* We had a tie within a tie in the lookahead, break it randomly */
    while (num >= count) num = num/20;
    return id[num];
}


static int Look_Ahead_Tie()
{
    int id[60], f = 0, min = 60;                                                                            /* Look ahead and find the face to go to that will give the least number of adjacencies */
    for (int x = 0; x < last; x++)
    {
        int t = Look_Ahead(ties_array[x]);
      
        if (t == min)                                                                                       /* We have a tie */
            id[f++] = ties_array[x];
        if (t < min)
        {
            f = 0;
            min = t;
            id[f++] = ties_array[x];
        }
    }
    if ((f == 1) || (min == 0)) return id[0];                                                               /* No tie within the tie. Or ties, but we are at the end of strips */
    return (Random_Look(id, f));
}


static int Sequential_Tri(int *index)
{
    /* We have a triangle and need to break the ties at it. We will choose the edge that is sequential. There is definitely one since we know we have a triangle
       and that there is a tie and there are only 2 edges for the tie. */
    int reversed, e1, e2, e3, output1, output2, output3, output4;
  
    Last_Edge(&e1, &e2, &e3, 0);                                                                            /* e2 and e3 are the input edge to the triangle */
    if ((e2 == 0) && (e3 == 0)) return ties_array[0];                                                       /* Starting the strip, don't need to do this */
  
    reversed = Get_EdgeEx(&output1, &output2, index, ties_array[0], 3, 0, 0);                               /* For the 2 ties find the edge adjacent to face id */
    reversed = Get_EdgeEx(&output3, &output4, index, ties_array[1], 3, 0, 0);
  
    if ((output1 == e3) || (output2 == e3)) return ties_array[0];
    if ((output3 == e3) || (output4 == e3)) return ties_array[1];
    printf("There is an error trying to break sequential triangle \n");
}

static int Sequential_Quad(int *index,  int triangulate)
{
    int reversed, output1, output2, x, e1, e2, e3;                                                          /* We have a quad that need to break its ties, we will try and choose a side that is sequential, otherwise use lookahead */

    Last_Edge(&e1, &e2, &e3, 0);                                                                            /* e2 and e3 are the input edge to the quad */
    if ((e2 == 0) && (e3 == 0)) return ties_array[0];                                                       /* No input edge */
  
    /* Go through the ties and see if there is a sequential one */
    for (x = 0; x < last; x++)
    {
        reversed = Get_EdgeEx(&output1,&output2,index,ties_array[x],4,0,0);
      
        if (((output1 == e3) || (output2 == e3)) && (triangulate == PARTIAL)) return ties_array[x];         /* Partial and whole triangulation will have different requirements */
        if (((output1 != e3) && (output1 != e2) && (output2 != e3) && (output2 != e2))) return ties_array[x];
    }
  
    return Look_Ahead_Tie();                                                                                /* There was not a tie that was sequential */
}

static void Whole_Output(int in1, int *index, int size, int *out1, int *out2)
{
    /* Used to sequentially break ties in the whole triangulation for polygons greater than 4 sides. We will find the output edge that is good for sequential triangulation. */
    int half;
  
    Rearrange_IndexEx(index, size);                                                                         /* Put the input edge first in the list */
  
    if (!(EVEN(size)))
    {
        if (*(index) == in1)
            half = size / 2;
        else
            half = size / 2 + 1;
    }
    else
        half = size / 2;
  
    *out1 = *(index+half);
    *out2 = *(index+half+1);
}

static int Sequential_Poly(int size, int *index, int triangulate)
{
    int x, reversed, output1, output2, e1, e2, e3, saved1 = -1, saved2 = -1, output3, output4;              /* We have a polygon of greater than 4 sides and wish to break the tie in the most sequential manner. */
    Last_Edge(&e1, &e2, &e3, 0);                                                                            /* e2 and e3 are the input edge to the quad */
  
    if (triangulate == WHOLE)                                                                               /* If we are using whole, find the output edge that is sequential */
        Whole_Output(e2,index,size,&output3,&output4);
  
    if ((e2 == 0) && (e3 == 0)) return ties_array[0];                                                       /* No input edge */
  
    for (x = 0; x < last ; x++)
    {
        reversed = Get_EdgeEx(&output1,&output2,index,ties_array[x],size,0,0);
      
        if (((output1 == e3) || (output2 == e3)) && (triangulate == PARTIAL))                               /* Partial that can be removed in just one triangle */
            saved1 = ties_array[x];
        if ((output1 != e3) && (output1 != e2) && (output2 != e3) && (output2 != e2) && (triangulate == PARTIAL) && (saved2 != -1))
            saved2 = ties_array[x];                                                                         /* Partial removed in more than one triangle */
        if (((output1 == output3) && (output2 == output4)) || ((output1 == output4) && (output2 == output3)) && (triangulate == WHOLE))
            return ties_array[x];                                                                           /* Whole is not so easy, since the whole polygon must be done. Given an input edge there is only one way to come out, approximately half way around the polygon. */
    }
  
    if (saved1 != -1) return saved1;
    if (saved2 != -1) return saved2;
  
  
    return Look_Ahead_Tie();                                                                                /* There was not a tie that was sequential */
}

static int Sequential_Tie(int face_id,int triangulate)
{
    /* Break the tie by choosing the face that will not give us a swap and is sequential. If there is not one, then do the lookahead to break the tie. */
    /* Separate into 3 cases for simplicity, if the current polygon has 3 sides, 4 sides or if the sides were greater. We can do the smaller cases faster, so that is why I separated the cases. */
  
    ListHead *pListFace;
    PF_FACES face;
  
  
    pListFace  = PolFaces[face_id];                                                                         /* Get the polygon with id face_id */
    face = (PF_FACES) PeekList(pListFace, LISTHEAD, 0);
  
    if (face->nPolSize == 3) return(Sequential_Tri(face->pPolygon));
    if (face->nPolSize == 4) return(Sequential_Quad(face->pPolygon, triangulate));
    return Sequential_Poly(face->nPolSize, face->pPolygon, triangulate);  
}

int Get_Next_Face(int t, int face_id, int triangulate)
{
    if (last == 1) return(ties_array[0]);                                                                   /* Get the next face depending on what the user specified */
    if (t == RANDOM) return Random_Tie();                                                                   /* Did not have a tie, don't do anything */                 
    if (t == ALTERNATE) return Alternate_Tie();
    if (t == LOOK) return Look_Ahead_Tie();
    if (t == SEQUENTIAL) return Sequential_Tie(face_id,triangulate);
  
    printf("Illegal option specified for ties, using first\n");
    return ties_array[0];
}