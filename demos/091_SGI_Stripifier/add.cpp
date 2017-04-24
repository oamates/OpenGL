//=======================================================================================================================================================================================================================
// STRIPE : add.hpp
// This file contains the procedure code that will add information to our data structures.
//=======================================================================================================================================================================================================================

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "log.hpp"
#include "add.hpp"
#include "global.hpp"

static bool new_vertex(double difference, int id1, int id2, struct vert_struct *n)
{
  /* Is the difference between id1 and id2 (2 normal vertices that mapped to the same vertex) greater than the threshold that was specified? */
    struct vert_struct *pn1, *pn2;
    double dot_product;
    double distance1, distance2,distance;
    char arg1[100];
    char arg2[100];
  
    pn1 = n + id1;
    pn2 = n + id2;
  
    dot_product = ((pn1->x) * (pn2->x)) + ((pn1->y) * (pn2->y)) + ((pn1->z) * (pn2->z));
  
    if (dot_product < 0)                                                                                /* Get the absolute value */
        dot_product = -dot_product;
  
    distance1 = sqrt((pn1->x * pn1->x) + (pn1->y * pn1->y) + (pn1->z * pn1->z));
    distance2 = sqrt((pn2->x * pn2->x) + (pn2->y * pn2->y) + (pn2->z * pn2->z));
    distance = distance1 * distance2;
  
    double rad = acos((double)dot_product / (double)distance);
    rad = (180.0 * rad) / PI;                                                                           /* convert to degrees */
  
    if ( rad <= difference)
        return false;
  
    
    sprintf(arg1,"%.5f", rad);                                                                          /* double checking because of imprecision with floating point acos function */
    sprintf(arg2,"%.5f", difference);
    if (strcmp(arg1, arg2) <= 0) return false;
    return rad > difference;
}

static bool Check_VN(int vertex,int normal, struct vert_added *added)
{
    int n = (added + vertex)->num;                                                                      /* Check to see if we already added this vertex and normal */
    for (int x = 0; x < n; x++)
    {
        if (*((added + vertex)->normal + x) == normal)
            return true;
    }
    return false;
}

bool norm_array(int id, int vertex, double normal_difference, struct vert_struct *n, int num_vert)
{
    static int last;
    static struct vert_added *added;
    register int x;
    static bool first = true;
  
    if (first)
    {
        /* This is the first time that we are in here, so we will allocate a structure that will save the vertices that we added, so that we do not add the same thing twice */
        first = false;
        added = (struct vert_added *) malloc (sizeof (struct vert_added ) * num_vert);
        /* The number of vertices added for each vertex must be initialized to zero */
        for (int x = 0; x < num_vert; x++)
            (added + x)->num = 0;
    }
  
    if (vertex)
    /* Set the pointer to the vertex, we will be calling again with the normal to fill it with */
        last = id;
    else
    {    
        /* Fill the pointer with the id of the normal */
        if (*(vert_norms + last) == 0)
            *(vert_norms + last) = id;
        else if ((*(vert_norms + last) != id) && ((int)normal_difference != 360))
        {
      /*   difference is big enough, we need to create a new vertex */
            if (new_vertex(normal_difference,id,*(vert_norms + last),n))
            {
          /*  First check to see if we added this vertex and normal already */
                if (Check_VN(last, id, added)) return false;
          /*  OK, create the new vertex, and have its id = the number of vertices and its normal what we have here */
                vert_norms = (int*) realloc(vert_norms, sizeof(int) * (num_vert + 1));
                if (!vert_norms)
                    exit_msg("Allocation error - aborting\n");

                *(vert_norms + num_vert) = id;
          /*   We created a new vertex, now put it in our added structure so we do not add the same thing twice
          */
                (added + last)->num = (added + last)->num + 1;
                if ((added + last)->num == 1)
                {
          /*   First time */
                    (added + last)->normal =  (int*) malloc (sizeof (int ) * 1);
                    *((added + last)->normal) =  id;
                }
                else
                {
          /*   Not the first time, reallocate space */
                    (added+last)->normal = (int*) realloc((added+last)->normal, sizeof(int) * (added+last)->num);
                    *((added+last)->normal+((added + last)->num-1)) = id;
                }
                return true;
            }
        }
    }
    return false;
}

void add_texture(int id,bool vertex)
{
    static int last;                                                                                                            /* Save the texture with its vertex for future use when outputting */
    if (vertex)
        last = id;
    else
        *(vert_texture + last) = id;
}

int add_vert_id(int id, int index_count)
{
    for (int x = 1; x < index_count ; x++)                                                                                      /* Test if degenerate, if so do not add degenerate vertex */
        if (ids[x] == id) return 0;
    ids[index_count] = id;
    return 1;
}

void add_norm_id(int id, int index_count)
{
    norms[index_count] = id;
}

void AddNewFace(int ids[MAX1], int vert_count, int face_id, int norms[MAX1])
{
    PF_FACES pfNode;
    PF_VERTICES pfVertNode;
    int   *pTempInt;
    int *pnorms;
    F_EDGES **pTempVertptr;
    int   *pTempmarked, *pTempwalked;
    register int  y,count = 0; 
  
    /* Add a new face into our face data structure */
  
    pfNode = (PF_FACES) malloc(sizeof(F_FACES) );
    if ( pfNode)
    {
        pfNode->pPolygon = (int*) malloc(sizeof(int) * (vert_count) );
        pfNode->pNorms = (int*) malloc(sizeof(int) * (vert_count) );
        pfNode->VertandId = (F_EDGES**)malloc(sizeof(F_EDGES*) * (vert_count)); 
        pfNode->marked  = (int*)malloc(sizeof(int) * (vert_count));
        pfNode->walked = (int*)malloc(sizeof(int) * (vert_count));
    }
    pTempInt =pfNode->pPolygon;
    pnorms = pfNode->pNorms;
    pTempmarked = pfNode->marked;
    pTempwalked = pfNode->walked;
    pTempVertptr = pfNode->VertandId;
    pfNode->nPolSize = vert_count;
    pfNode->nOrgSize = vert_count;
    pfNode->seen = -1;
    pfNode->seen2 = -1;
    for (y = 1; y <= vert_count; y++)
    {
        *(pTempInt + count) = ids[y];
        *(pnorms + count) = norms[y];
        *(pTempmarked + count) = false;
        *(pTempwalked + count) = -1;
        *(pTempVertptr + count) = NULL;
        count++;
        /* Add this FaceNode to the Vertices Structure */
        pfVertNode = (PF_VERTICES) malloc(sizeof(F_VERTICES));
        pfVertNode->face = pfNode;
        AddHead(Vertices[ids[y]],(PLISTINFO) pfVertNode);          
    }
    AddHead(PolFaces[face_id - 1], (PLISTINFO) pfNode);
}   


void CopyFace(int ids[MAX1], int vert_count, int face_id, int norms[MAX1])
{
    PF_FACES pfNode;
    int* pTempInt;
    int* pnorms;
    F_EDGES **pTempVertptr;
    int *pTempmarked, *pTempwalked;
    register int y,count = 0;
  
    /* Copy a face node into a new node, used after the global algorithm is run, so that we can save whatever is left into a new structure */
  
    pfNode = (PF_FACES) malloc(sizeof(F_FACES) );
    if (pfNode)
    {
        pfNode->pPolygon = (int*) malloc(sizeof(int) * (vert_count) );
        pfNode->pNorms = (int*) malloc(sizeof(int) * (vert_count) );
        pfNode->VertandId = (F_EDGES**)malloc(sizeof(F_EDGES*) * (vert_count)); 
        pfNode->marked  = (int*)malloc(sizeof(int) * (vert_count));
        pfNode->walked = (int*)malloc(sizeof(int) * (vert_count));
    }
    pTempInt =pfNode->pPolygon;
    pnorms = pfNode->pNorms;
    pTempmarked = pfNode->marked;
    pTempwalked = pfNode->walked;
    pTempVertptr = pfNode->VertandId;
    pfNode->nPolSize = vert_count;
    pfNode->nOrgSize = vert_count;
    pfNode->seen = -1;
    pfNode->seen2 = -1;
    for (y = 0; y < vert_count; y++)
    {
        *(pTempInt + count) = ids[y];
        *(pnorms + count) = norms[y];
        *(pTempmarked + count) = false;
        *(pTempwalked + count) =  -1;
        *(pTempVertptr+count) = NULL;
        count++;
    }
    AddHead(PolFaces[face_id-1],(PLISTINFO) pfNode);
}   


void Add_AdjEdge(int v1,int v2,int fnum,int index1 )
{
    PF_EDGES temp  = NULL;
    PF_FACES temp2 = NULL;
    PF_EDGES pfNode;
    ListHead *pListHead;
    ListHead *pListFace;
    bool flag = true;
    register int  count = 0;
    register int  t,v3 = -1;
    
    if (v1 > v2)
    {
        t  = v1;
        v1 = v2;
        v2 = t;
    }

    pListFace  = PolFaces[fnum];
    temp2 = (PF_FACES) PeekList(pListFace,LISTHEAD,0);
    pListHead = PolEdges[v1];
    temp = (PF_EDGES) PeekList(pListHead,LISTHEAD,count);
    if (temp == NULL) flag = false;
    count++;
    while (flag)
    {
        if (v2 == temp->edge[0])
        {
            /* If greater than 2 polygons adjacent to an edge, then we will only save the first 2 that we found. We will have a small performance hit, but this does not happen often. */

            if (temp->edge[2] == -1)
                temp->edge[2] = fnum;
            else
                v3 = temp->edge[2];
            flag = false;
        }
        else
        {
            temp = (PF_EDGES) GetNextNode(temp);                                   
      
            count++;
            if (temp == NULL) flag = false;
        }
    }
  
  /*   Did not find it */
    if (temp == NULL)
    {
        pfNode = (PF_EDGES) malloc(sizeof(F_EDGES));
        if (pfNode)
        {
            pfNode->edge[0] = v2;
            pfNode->edge[1] = fnum;
            pfNode->edge[2] =  v3;
            AddTail( PolEdges[v1], (PLISTINFO) pfNode );
        }
        else
            exit_msg("Out of memory!\n");
      
        *(temp2->VertandId+index1) = pfNode;
    }
    else
        *(temp2->VertandId+index1) =  temp;
}  
