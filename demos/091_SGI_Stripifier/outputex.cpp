//=======================================================================================================================================================================================================================
// STRIPE : outputex.hpp
// This file contains routines that are used for various functions in the local algorithm.
//=======================================================================================================================================================================================================================

#include <cstdio>
#include <cstdlib>

#include "log.hpp"
#include "common.hpp"
#include "options.hpp"
#include "output.hpp"
#include "outputex.hpp"
#include "partial.hpp"
#include "polverts.hpp"
#include "queue.hpp"
#include "sgi_triangex.hpp"
#include "structex.hpp"
#include "ties.hpp"
#include "util.hpp"


void Output_TriEx(int id1, int id2, int id3, FILE* output, int flag, int where)
{
    /* We will save everything into a list, rather than output at once, as was done in the old routine. This way for future modifications we can change the strips later on if we want to. */
  
    int swap,temp1,temp2,temp3;
    static int total = 0;
    static int tri = 0;
    static int strips = 0;
    static int cost = 0;
  
    if (flag == -20)
    {
        cost = cost + where+total+tri+strips+strips;
        printf("We will need to send %d vertices to the renderer\n",cost);
        total = 0;
        tri = 0;
        strips = 0;
        return;
    }
  
  
    if (flag == -10)
    /* We are finished, now is time to output the triangle list */
    {
        fprintf(output,"\nt");
        tri = tri + Finished(&swap,output,1);
        total = total + swap;
        strips++;
        /* printf("There are %d swaps %d tri %d strips\n",total,tri,strips); */
    }
    else
    {
        Last_Edge(&temp1,&temp2,&temp3,0);
        Add_Id_Strips(id1,where);
        Add_Id_Strips(id2,where);
        Add_Id_Strips(id3,where);
        Last_Edge(&id1,&id2,&id3,1);
    }
}




void Extend_BackwardsEx(int face_id, FILE *output, int *ties, int tie, int triangulate, int swaps, int *next_id)
{
    /* We just made a strip, now we are going to see if we can extend backwards from the starting face, which had 2 or more adjacencies to start with. */
  
    int bucket, next_face, num, x, y, z, c, max, f;              
    ListHead *pListFace;
    PF_FACES face;
    P_ADJACENCIES temp;
  
    /* Get the first triangle that we have saved the the strip data structure, so we can see if there are any polygons adjacent to this edge or a neighboring one */
    First_Edge(&x,&y,&z); 
  
    pListFace  = PolFaces[face_id];
    face = (PF_FACES) PeekList(pListFace, LISTHEAD, 0);
  
    num = face->nPolSize;
  
    /* Go through the edges to see if there is an adjacency with a vertex in common to the first triangle that was outputted in the strip. (maybe edge was deleted....) */
    for (c = 0; c < num; c++)
    {
      
        if ((c != (num-1)) && (((*(face->pPolygon + c) == x) && (*(face->pPolygon + c + 1) == y)) || (*(face->pPolygon + c) == y) && (*(face->pPolygon + c + 1) == x)))
        {
            next_face = Find_Face(face_id, x, y, &bucket);                                                          /* Input edge is still there see if there is an adjacency */
            if (next_face == -1) break;                                                                             /* Could not find a face adjacent to the edge */
        
            pListFace = array[bucket];
            max = NumOnList(pListFace);
            for (f = 0; ; f++)
            {
                temp = (P_ADJACENCIES) PeekList(pListFace, LISTHEAD, f);    
                if (temp->face_id == next_face)
                {
                    Last_Edge(&z, &y, &x, 1);
                    Polygon_OutputEx(temp, temp->face_id, bucket, pListFace, output, ties, tie, triangulate, swaps, next_id, 0);
                    return;
                }
          
                if (temp == NULL)
                    exit_msg("Error in the new buckets%d %d %d\n", bucket, max, 0);
            }
      
        }
        else if ((c == (num -1)) && (((*(face->pPolygon) == x) && (*(face->pPolygon + num - 1) == y)) || (*(face->pPolygon) == y) && (*(face->pPolygon + num - 1) == x)))
        {
            next_face = Find_Face(face_id, x, y, &bucket);
            if (next_face == -1) break;                                                                             /* Could not find a face adjacent to the edge */
        
            pListFace = array[bucket];
            max = NumOnList(pListFace);
            for (f=0;;f++)
            {
                temp = (P_ADJACENCIES) PeekList(pListFace, LISTHEAD, f);
                if (temp->face_id == next_face)
                {
                    Last_Edge(&z,&y,&x,1);
                    Polygon_OutputEx(temp, temp->face_id, bucket, pListFace, output, ties, tie, triangulate, swaps, next_id, 0);
                    return;
                }
          
                if (temp == NULL)
                    exit_msg("Error in the new buckets%d %d %d\n", bucket, max, 0);
            }
        }
    }
}

void Polygon_OutputEx(P_ADJACENCIES temp, int face_id, int bucket, ListHead* pListHead, FILE* output, int* ties, int tie, int triangulate, int swaps, int* next_id, int where)
{
    ListHead *pListFace;
    PF_FACES face;
    static bool begin = true;
    int old_face, next_face_id, next_bucket, e1, e2, e3, other1, other2, other3;
    P_ADJACENCIES lpListInfo; 
  
  /* We have a polygon to output, the id is face id, and the number of adjacent polygons to it is bucket. */
  
    Last_Edge(&e1, &e2, &e3, 0);
  
    /*  Get the polygon with id face_id */
    pListFace  = PolFaces[face_id];
    face = (PF_FACES) PeekList(pListFace, LISTHEAD, 0);
  
    if (face->nPolSize == 3)                                                                                                    /* It is already a triangle */
    {
        if (bucket == 0)                                                                                                        /* It is not adjacent to anything so we do not have to worry about the order of the sides or updating adjacencies */
        {
            Last_Edge(&e1, &e2, &e3, 0);
            next_face_id = Different(*(face->pPolygon), *(face->pPolygon + 1), *(face->pPolygon + 2), e1, e2, e3, &other1, &other2);
            
            if ((e2 ==0) && (e3 == 0))                                                                                          /* No input edge, at the start */
            {
                e2 = other1;
                e3 = other2;
            }
      
            Output_TriEx(e2,e3,next_face_id,NULL,begin,where);
            if (face_array[temp->face_id].head == pListHead)
                face_array[temp->face_id].pfNode = NULL;
            RemoveList(pListHead, (PLISTINFO) temp);            
            begin = true;                                                                                                       /* We will be at the beginning of the next strip. */
        }
        /* It is a triangle with adjacencies. This means that we have to:
          1. Update the adjacencies in the list, because we are using this polygon and it will be deleted.
          2. Get the next polygon. */
        else
        {
        /* Return the face_id of the next polygon we will be using, while updating the adjacency list by decrementing the adjacencies of everything adjacent to the current triangle. */
      
        next_face_id = Update_AdjacenciesEx(face_id, &next_bucket, &e1, &e2, ties);
        old_face = next_face_id;
      
        /* Break the tie,  if there was one */
        if (tie != FIRST)
            old_face = Get_Next_Face(tie, face_id, triangulate);
      
        if (next_face_id == -1)
        {
            Polygon_OutputEx(temp, face_id, 0, pListHead, output, ties, tie, triangulate, swaps, next_id, where);
            return;
        }
      
        /*  We are using a different face */
        if ((tie != FIRST) && (old_face != next_face_id) && (swaps == ON))
        {
            next_face_id = old_face;
            /* Get the new output edge, since e1 and e2 are for the original next face that we got. */
            e3 = Get_EdgeEx(&e1, &e2, face->pPolygon, next_face_id, face->nPolSize, 0, 0);
        }
      
        /* Find the other vertex to transmit in the triangle */
        e3 = Return_Other(face->pPolygon, e1, e2);
        Last_Edge(&other1, &other2, &other3, 0);
      
        if ((other1 != 0) && (other2 != 0))
        {
            /* See which vertex in the output edge is not in the input edge */
            if ((e1 != other2) && (e1 != other3))
                e3 = e1;
            else if ((e2 != other2) && (e2 != other3))
                e3 = e2;
            /* can happen with > 2 polys on an edge but won't form a good strip so stop the strip here */
            else
            {
                Polygon_OutputEx(temp, face_id, 0, pListHead, output, ties, tie, triangulate, swaps, next_id, where);
                return;
            }
          
            /*   See which vertex of the input edge is not in the output edge */
            if ((other2 != e1) && (other2 != e2))
            {
                other1 = other2;
                other2 = other3;
            }
            else if ((other3 != e1) && (other3 != e2))
                other1 = other3;
            else
            {
                /* Degenerate triangle just return*/
                Output_TriEx(other1, other2, e3, NULL, begin, where);
                if (face_array[temp->face_id].head == pListHead)
                    face_array[temp->face_id].pfNode = NULL;
                RemoveList(pListHead,(PLISTINFO) temp);
                begin = false;
                return;
            }
        }
        else                                                                                                        /*  There was not an input edge, we are the first triangle in a strip */
        {
            other1 = e3;                                                                                            /* Find the correct order to transmit the triangle, what is the output edge that we want ? */
            e3 = e2;
            other2 = e1;
        }
      
        /* At this point the adjacencies have been updated and we have the next polygon id */
        Output_TriEx(other1, other2, e3, NULL, begin, where);
        if (face_array[temp->face_id].head == pListHead)
            face_array[temp->face_id].pfNode = NULL;
        RemoveList(pListHead,(PLISTINFO) temp);
        begin = false;
      
        if (Done(next_face_id,&next_bucket) == NULL) return;
      
        pListHead = array[next_bucket];
        lpListInfo = face_array[next_face_id].pfNode;
        if (lpListInfo == NULL)
            exit_msg("There is an error finding the next polygon3 %d\n", next_face_id);
        Polygon_OutputEx(lpListInfo, next_face_id, next_bucket, pListHead, output, ties, tie, triangulate, swaps, next_id, where);
      
        }
    }
    else
    {
        if (bucket == 0)                                                                                            /* It is not a triangle, we have to triangulate it.Since it is not adjacent to anything we can triangulate it blindly */
        {            
            Last_Edge(&other1, &other2, &other3, 0);                                                                /* Check to see if there is not an input edge */
            if ((other1 == 0) && (other2 == 0))
                Blind_TriangulateEx(face->nPolSize, face->pPolygon, true, where);
            else
                Blind_TriangulateEx(face->nPolSize, face->pPolygon, false, where);
      
            if (face_array[temp->face_id].head == pListHead)        
                face_array[temp->face_id].pfNode = NULL;
            RemoveList(pListHead,(PLISTINFO) temp);
            begin = true;                                                                                           /* We will be at the beginning of the next strip. */
        }
      
        /* If we have specified PARTIAL triangulation then we will go to special routines that will break the polygon and update the data structure. Else everything
           below will simply triangulate the whole polygon */
      else if (triangulate == PARTIAL)
        {
      
      /*  Return the face_id of the next polygon we will be using,
       */
      next_face_id = Min_Face_AdjEx(face_id,&next_bucket,ties);
      
      
      /* Don't do it partially, because we can go inside and get
         less adjacencies, for a quad we can do the whole thing.
      */
      if ((face_id == next_face_id) && (face->nPolSize == 4) && (swaps == ON))
        {
          next_face_id = Update_AdjacenciesEx(face_id, &next_bucket,
                          &e1,&e2,ties);
          if (next_face_id == -1)
        {
          /*  There is no sequential face to go to, end the strip */
          Polygon_OutputEx(temp,face_id,0,pListHead,output,
                   ties,tie,triangulate,swaps,next_id,where);
          return;
        }
          
          /* Break the tie,  if there was one */
          if (tie != FIRST)
        next_face_id = Get_Next_Face(tie,face_id,triangulate);
          Non_Blind_TriangulateEx(face->nPolSize,face->pPolygon,
                      output,next_face_id,face_id,where);
          
          if (face_array[temp->face_id].head == pListHead)          
        face_array[temp->face_id].pfNode = NULL;
          RemoveList(pListHead,(PLISTINFO) temp);
        }
      
          /*   Was not a quad but we still do not want to do it partially for
           now, since we want to only do one triangle at a time
      */
      else if ((face_id == next_face_id) && (swaps == ON))
        Inside_Polygon(face->nPolSize,face->pPolygon,
               face_id,pListHead,where);
      
      else
        {
          if ((tie != FIRST) && (swaps == ON))
        next_face_id = Get_Next_Face(tie,face_id,triangulate);
          Partial_Triangulate(face->nPolSize,face->pPolygon,
                  output,next_face_id,face_id,next_id,
                  pListHead,temp,where);
          /*    Check the next bucket again, maybe it changed. We calculated one less, but that might not be the case */
        }
      
      if (Done(next_face_id,&next_bucket) == NULL)
        {
          /*  Check to see if there is not an input edge */
          Last_Edge(&other1,&other2,&other3,0);
          if ((other1 == 0) && (other2 ==0))
        Blind_TriangulateEx(face->nPolSize,face->pPolygon, true,where);
          else
        Blind_TriangulateEx(face->nPolSize,face->pPolygon, false,where);
          
          if (Done(face_id, &bucket) != NULL)
        {
          pListHead = array[bucket];
          lpListInfo = face_array[face_id].pfNode;
          if (face_array[temp->face_id].head == pListHead)
            face_array[lpListInfo->face_id].pfNode = NULL;
          RemoveList(pListHead,(PLISTINFO)lpListInfo);
        }
          begin = true;
          return;
        }
      
      begin = false;
      pListHead = array[next_bucket];
      lpListInfo = face_array[next_face_id].pfNode;
      if (lpListInfo == NULL)
        exit_msg("There is an error finding the next polygon1 %d %d\n",next_face_id,next_bucket);
      Polygon_OutputEx(lpListInfo,next_face_id,next_bucket,pListHead, output,ties,tie,triangulate,swaps,next_id,where);
        }
      
      
      else
        {
          /*  WHOLE triangulation. It is not a triangle and has adjacencies. This means that we have to:
            1. TriangulateEx this polygon, not blindly because we have an edge that we want to come out on, that is the edge that is adjacent to a polygon with the
               least number of adjacencies. Also we must come in on the last seen edge.
            2. Update the adjacencies in the list, because we are using this polygon .
            3. Get the next polygon. */
          /* Return the face_id of the next polygon we will be using, while updating the adjacency list by decrementing the adjacencies of everything adjacent to the current polygon. */
          
          next_face_id = Update_AdjacenciesEx(face_id,&next_bucket, &e1,&e2,ties);
          
          if (Done(next_face_id,&next_bucket) == NULL)
            {
          Polygon_OutputEx(temp,face_id,0,pListHead,output,ties,tie, 
                   triangulate,swaps,next_id,where);
          /*    Because maybe there was more than 2 polygons on the edge */
          return;
            }
          
          /*      Break the tie,  if there was one */
          else if (tie != FIRST)
            next_face_id = Get_Next_Face(tie,face_id,triangulate);
          
          Non_Blind_TriangulateEx(face->nPolSize,face->pPolygon, 
                  output,next_face_id,face_id,where);
      
          if (face_array[temp->face_id].head == pListHead)
            face_array[temp->face_id].pfNode = NULL;
          RemoveList(pListHead,(PLISTINFO) temp);
          begin = false;
          pListHead = array[next_bucket];
          lpListInfo = face_array[next_face_id].pfNode;
      
          if (lpListInfo == NULL)
            exit_msg("There is an error finding the next polygon2 %d %d\n", next_face_id,next_bucket);
          Polygon_OutputEx(lpListInfo,next_face_id,next_bucket,pListHead, output,ties,tie,triangulate,swaps,next_id,where);
    }
      
    }
  Last_Edge(&e1,&e2,&e3,0);
}
