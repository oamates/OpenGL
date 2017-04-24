//=======================================================================================================================================================================================================================
// STRIPE : local.hpp
// This file contains the code that initializes the data structures for the local algorithm, and starts the local algorithm going.
//=======================================================================================================================================================================================================================

#include <cstdio>
#include <cstdlib>

#include "log.hpp"
#include "global.hpp"
#include "init.hpp"
#include "local.hpp"
#include "outputex.hpp"
#include "polverts.hpp"
#include "queue.hpp"
#include "sgi_triang.hpp"
#include "util.hpp"

static void Find_StripsEx(FILE* output, int* ties, int tie, int triangulate, int swaps, int* next_id)
{
    ListHead *pListHead;                                                                                        /* This routine will peel off the strips from the model */
    P_ADJACENCIES temp = NULL;
    register int max, bucket = 0;
    bool whole_flag = true;
    int dummy = 0;

    Last_Edge(&dummy, &dummy, &dummy, 1);                                                                       /* Set the last known input edge to be null */
    
    while (whole_flag)                                                                                          /* Search for lowest adjacency polygon and output strips */
    {
        bucket = -1;
        
        while (bucket < 59)                                                                                     /* Search for polygons in increasing number of adjacencies */
        {
            bucket++;
            pListHead = array[bucket];
            max = NumOnList(pListHead);
            if (max > 0)
            {
                temp = (P_ADJACENCIES) PeekList(pListHead,LISTHEAD,0);
                if (temp == NULL)
                    exit_msg("Error in the buckets%d %d %d\n", bucket, max, 0);
                Polygon_OutputEx(temp, temp->face_id, bucket, pListHead, output, ties, tie, triangulate, swaps, next_id, 1);
                
                if (bucket >= 2)                                                                                /* Try to extend backwards, if the starting polygon in the strip had 2 or more adjacencies to begin with */
                    Extend_BackwardsEx(temp->face_id, output, ties, tie, triangulate, swaps, next_id);
                break;  
            }
        }
        
        if ((bucket == 59) && (max == 0))                                                                       /* Went through the whole structure, it is empty and we are done. */
            whole_flag = false;
        else                                                                                                    /* We just finished a strip, send dummy data to signal the end of the strip so that we can output it. */
        {
            Output_TriEx(-1, -2, -3, output, -10, 1);
            Last_Edge(&dummy, &dummy, &dummy, 1);
        }
    }
}



void SGI_Strip(int num_faces,FILE *output, int ties, int triangulate)
{
    int next_id = -1, t = 0;
    // We are going to output and find triangle strips according the the method that SGI uses, ie always choosing as the next triangle in our strip the triangle that has the least number of adjacencies. 
    // We do not have all triangles and will be triangulating on the fly those polygons that have more than 3 sides.
    
    Init_Table_SGI(num_faces);                                                                                  // Build a table that has all the polygons sorted by the number of polygons adjacent to it. Initialize it
    Build_SGI_Table(num_faces);                                                                                 // Build it
    InitStripTable();                                                                                           // We will have a structure to hold all the strips, until outputted.
    // Now we have the structure built to find the polygons according to the number of adjacencies. Now use the SGI Method to find strips according to the adjacencies
    Find_StripsEx(output,&t,ties,triangulate,ON,&next_id);
}
