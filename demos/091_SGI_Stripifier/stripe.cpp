//=======================================================================================================================================================================================================================
// STRIPE: stripe.cpp
// This file contains the main procedure code that will read in the object and then call the routines that produce the triangle strips.
//=======================================================================================================================================================================================================================

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "log.hpp"
#include "add.hpp"
#include "common.hpp"
#include "free.hpp"
#include "global.hpp"
#include "init.hpp"
#include "local.hpp"
#include "newpolve.hpp"
#include "options.hpp"
#include "outputex.hpp"
#include "sgi_triang.hpp"

#define MAX1 60

/*   TIMING for Windows */
#ifdef WIN32
#include <sys/timeb.h>
#include <time.h>
static time_t start;
static time_t finish;

#define START time(&start); 
#define STOP time(&finish);
/*   TIMING for UNIX */
#else
#include <sys/types.h>
#include <sys/param.h>
#include <sys/times.h>
#include        <sys/time.h>
struct timeval   tm;
static struct timezone  tz;
static double           et;
#define START gettimeofday(&tm,&tz);et = (tm.tv_sec)+ (0.000001* (tm.tv_usec));
#define STOP gettimeofday(&tm,&tz);et = (tm.tv_sec)+(0.000001*(tm.tv_usec)) - et;
#endif


static void get_time()
{
  
#ifdef WIN32
    printf("Elapsed time for generating stripes was %lf seconds.\n", difftime(finish,start) );
#else
    printf("Elapsed time for generating stripes was %lf seconds.\n", et);
#endif
}

struct vert_struct *vertices = NULL, *nvertices = NULL, *pvertices = NULL, *pnvertices = NULL;                              // Structures for the object

FILE* OpenOutputFile(const char* fname)
{
    FILE* bands;
    int flength = 0;
    char* newfname;
    
    flength = strlen(fname);                                                                                                // get the length of the fname
    newfname = (char *) malloc(sizeof(char) * (flength + 2));                                                               // make a new string in memory one larger than fname  
    strcpy(newfname, fname);                                                                                                // Copy the fname to the newfname
    newfname[flength] = 'f';                                                                                                // add an 'f' to the end of it
    newfname[flength + 1] = '\0';    
    printf("   Output file : %s\n", newfname); 
        
    bands = fopen(newfname,"w");                                                                                            // File that will contain the triangle strip data
    fprintf(bands,"#%s: a triangle strip representation created by STRIPE.\n#This is a .objf file\n#by Francine Evans\n",fname);    
    return bands;
}

void GetFileDataInfo(const char* fname, const char* file_open, int* num_vert, int* num_nvert, int* num_texture, int* num_faces, int* num_tris, int* num_buffers, int f) 
{
    FILE *file;
    char buff[255];
    int tempi;
    
    if ((file = fopen (fname,file_open)))                                                                                   // File can be in binary for faster reading
    {                                                                                                                       
        while (!feof (file))                                                                                                
        {                                                                                                                   
            if (f == BINARY)                                                                                                // Read a line
                fread (buff, sizeof(char) * 255, 1, file);                                                                  
            else                                                                                                            
                fgets (buff, sizeof(char) * 255, file);                                                                     
            (*num_buffers)++;                                                                                               
            if (*buff == 'v')                                                                                               // At a vertex
            {                                                                                                               
                                                                                                                            
                if (*(buff + 1) == 'n')                                                                                     // At a normal
                    (*num_nvert)++;                                                                                         
                else if (*(buff + 1) == 't')                                                                                
                    (*num_texture)++;                                                                                       
                                                                                                                            
                else                                                                                                        
                    (*num_vert)++;                                                                                          // At a regular vertex
            }                                                                                                               
            else if (*buff == 'f')                                                                                          // At a face
            {  
                (*num_faces)++;
                strtok(buff, " ");
                tempi = 0;
                while (strtok(NULL, " ") != NULL) tempi++;
                *num_tris += tempi - 2;
            }
            buff[0] = EOF;
        }
        fclose (file);
    } 
    else
    {
        print_usage();
        exit_msg("\nError: Could not file specified file!\n");
    }
  
}

void AllocateStruct(int num_faces, int num_vert, int num_nvert, int num_texture)
{
    Start_Face_Struct(num_faces);                                                                                           /* Allocate structures for the information */
    Start_Vertex_Struct(num_vert);
    vertices = (struct vert_struct *) malloc (sizeof (struct vert_struct) * num_vert);

    if (num_nvert > 0) 
    {
        nvertices = (struct vert_struct *) malloc (sizeof (struct vert_struct) * num_nvert);
        vert_norms = (int *) malloc (sizeof (int) * num_vert);
        init_vert_norms(num_vert);                                                                                          /* Initialize entries to zero, in case there are 2 hits to the same vertex we will know it - used for determining the normal difference */
    }
    else 
        nvertices = NULL;
  
    if (num_texture > 0)
    {
        vert_texture = (int *) malloc (sizeof(int) * num_vert);
        init_vert_texture(num_vert);
    }
    pvertices = vertices;                                                                                                   /* Set up the temporary 'p' pointers */
    pnvertices = nvertices;
}

void ReadFile(const char* fname, const char* file_open, FILE *bands, int num_vert,int num_nvert, int num_texture, int num_faces, int num_tris, int num_buffers, int f, int g, int t, int tr, float norm_difference, bool *quads, int *cost) 
{
    FILE *file;
    char *all, *ptr, *ptr2;
    float center[3];
    int num2;
    int face_id = 0;
    bool texture, normal, normal_and_texture;
    int vert_count, loop;
    int vertex;
    int temp[MAX1];  
    int swaps, strips, triangles;
    
    if (file = fopen(fname, file_open))                                                                                     /* File will be put in a list for faster execution if file is in binary */   
    {
        if (f == BINARY)
        {
            all = (char *) malloc (sizeof(char) * 255 * num_buffers);
            fread(all, sizeof(char) * 255 * num_buffers, 1, file);
            ptr = all;
        }
        else
            ptr = (char *) malloc (sizeof(char) * 255 * num_buffers);
    }
    
    while (num_buffers > 0)
    {
        ptr[0] = EOF;
        num_buffers--;
        if (f == ASCII)
            fgets (ptr, sizeof(char) * 255, file);
        else
            ptr = ptr + 255;
        
        if (*ptr == 'v')                                                                                                    /* Load in vertices / normals */
        {
            if (*(ptr + 1) == 'n')
            {
                sscanf (ptr + 3,"%lf%lf%lf", &(pnvertices->x), &(pnvertices->y), &(pnvertices->z));
                fprintf(bands,"vn %lf %lf %lf\n", pnvertices->x, pnvertices->y, pnvertices->z); 
                ++pnvertices;
            }
            else if (*(ptr + 1)=='t')
            {
                sscanf (ptr + 3,"%f%f%f", &center[0], &center[1], &center[2]);
                fprintf(bands,"vt %f %f %f\n", center[0], center[1], center[2]); 
            }
            else
            {
                sscanf (ptr + 2,"%lf%lf%lf", &(pvertices->x), &(pvertices->y), &(pvertices->z));
                fprintf(bands,"v %lf %lf %lf\n", pvertices->x, pvertices->y, pvertices->z); 
                ++pvertices;
            }
        }
        else if (*ptr == 'f')
        {
            num2 = 0;                                                                                                       /* Read in faces */
            face_id++;
            ptr2 = ptr + 1;
            normal = false; 
            texture = false;
            normal_and_texture = false;
            while (*ptr2)
            {
                if (*ptr2 >='0' && *ptr2 <='9')
                {
                    num2++;
                    ++ptr2;
                    while (*ptr2 && (*ptr2!=' ' && *ptr2!='/')) ptr2++;
                    if (*ptr2 == '/')                                                                                       /* There are normals in this line */
                    {
                        if (*(ptr2 + 1) == '/')
                            normal = true;
                        else
                            texture = true;
                    }
                    else if (*ptr2 == ' ')
                    {
                        if ((num2 == 3) && (texture))
                            normal_and_texture = true;
                    }
                }
                else
                    ++ptr2;
            }
      
            ptr2 = ptr + 1;
            vert_count = 0;                                                                                                 /* loop on the number of numbers in this line of face data */
      
            for (loop = 0; loop < num2; loop++)
            {
                while (*ptr2 < '0' || *ptr2 > '9')                                                                          /* skip the whitespace */
                {
                    if (*ptr2 == '-') break;
                    ptr2++;
                }
                vertex = atoi(ptr2) - 1;
                if (vertex < 0)
                {
                    vertex = num_vert + vertex;
                    *ptr2 = ' ';
                    ptr2++;
                }
                if ((normal) && (!normal_and_texture))                                                                      /* If there are either normals or textures with the vertices in this file, the data alternates so we must read it this way */
                {
                    if (loop % 2)
                    {
                        add_norm_id(vertex,vert_count);
                        if (norm_array(vertex, 0, norm_difference, nvertices, num_vert))                                    /* Test here to see if we added a new vertex, since the vertex has more than one normal and the 2 normals are greater than the threshold specified */
                        {
                                                                                                                            /* Add a new vertex and change the id of the vertex that we just read to the id of the new vertex that we just added */
                                                                                                                            /* Put it in the output file, note the added vertices will be after the normals and separated from the rest of the vertices. Will not affect our viewer */
                            fprintf(bands,"v %lf %lf %lf\n", (vertices + temp[vert_count - 1])->x, (vertices + temp[vert_count - 1])->y, (vertices + temp[vert_count - 1])->z); 
                            num_vert++;
                            temp[vert_count - 1] = num_vert - 1;
                            if (!(add_vert_id(num_vert - 1, vert_count))) vert_count--;
                        }
                    }
                    else                                                                                                    /* the vertex */
                    {
                        temp[vert_count] = vertex ;
                        vert_count++;
                        if (!(add_vert_id(vertex,vert_count))) vert_count--;
                        norm_array(vertex, 1, norm_difference, nvertices, num_vert);
                    }
                }
                else if (normal_and_texture)                                                                                /* Else there are vertices and textures with the data */
                {
                    if( !((loop + 1) % 3))
                    {
                        add_norm_id(vertex,vert_count);
                        if (norm_array(vertex,0,norm_difference,nvertices,num_vert))                                        /* Test here to see if we added a new vertex, since the vertex has more than one normal and the 2 normals are greater than the threshold specified */
                        {
                                                                                                                            /* Add a new vertex and change the id of the vertex that we just read to the id of the new vertex that we just added */
                                                                                                                            /* Put it in the output file, note the added vertices will be after the normals and separated from the rest of the vertices. Will not affect our viewer */
                            fprintf(bands,"v %lf %lf %lf\n", (vertices + temp[vert_count - 1])->x, (vertices + temp[vert_count - 1])->y, (vertices + temp[vert_count - 1])->z); 
                            num_vert++;
                            temp[vert_count - 1] = num_vert - 1;
                            if (!(add_vert_id(num_vert - 1, vert_count))) vert_count--;
                        }
                    }
                    else if ((loop == 0) || (*(ptr2-1) == ' '))                                                             /* the vertex */
                    {
                        temp[vert_count] = vertex ;
                        vert_count++;
                        if (vert_count == 4) *quads = true;
                        if (!(add_vert_id(vertex,vert_count))) vert_count--;
                        add_texture(vertex, true);
                        norm_array(vertex, 1, norm_difference, nvertices, num_vert);
                    }
                    else                                                                                                    /* The texture */
                        add_texture(vertex, false);
                }
                else if (texture)
                {
                    if (!(loop % 2))                                                                                        /* the vertex */
                    {
                        temp[vert_count] = vertex ;
                        vert_count++;
                        if (vert_count == 4) *quads = true;
                        add_texture(vertex, true);
                        if (!(add_vert_id(vertex,vert_count))) vert_count--;
                        norm_array(vertex, 1, norm_difference, nvertices, num_vert);
                    }
                    else                                                                                                    /* texture */
                        add_texture(vertex, false);                                                                          
                }
                else
                {
                    temp[vert_count] = vertex;                                                                              /* no nvertices */
                    vert_count++;
                    if (vert_count == 4) *quads = true;
                    if (!(add_vert_id(vertex, vert_count))) vert_count--;
                }

                while (*ptr2 >= '0' && *ptr2 <= '9') ptr2++;
            }                                                                                                               /* Done with the polygon */
      
            if (vert_count >= 3)                                                                                            /* add it to face structure */
                AddNewFace(ids, vert_count, face_id, norms);
            else
                face_id--;
            if (vert_count == 4) *quads = true;
        }
        else if (g && (face_id > 0) && ((*ptr == 'g') || (*ptr  == 's') || (*ptr == 'm') || (*ptr == 'o')))
        {
            Start_Edge_Struct(num_vert);                                                                                    /* The user specified that the strips will be contained in each group from the data file, so we just finished a group and will find the triangle strips in it. */
            Find_Adjacencies(face_id);
            if (*quads)
            {
                Init_Table_SGI(num_faces);
                Build_SGI_Table(face_id);
                Save_Walks(face_id);                                                                                        /* Code for lengths of walks in each direction */
                Find_Bands(face_id, bands, &swaps, &strips, cost, &triangles, num_nvert, vert_norms, num_texture, vert_texture);    /* Code for finding the bands */
              
                if (*cost != 0)                                                                                             /* Remove the faces that we did  so that we can run the strip code on the rest of the faces that are left */
                {
                    printf("Total %d triangles with %d cost\n",triangles,*cost);
                    Save_Rest(&face_id);
                    printf("We saved %d .... now doing the local algorithm\n", face_id);
                    fprintf(bands,"\n#local\n");
                    End_Edge_Struct(num_vert);
                    Start_Edge_Struct(num_vert);
                    Find_Adjacencies(face_id);
                }
            }
            SGI_Strip(face_id, bands, t, tr);
            Output_TriEx(-1, -2, -3, NULL, -20, *cost);                                                                     /* Get the total cost */
            End_Face_Struct(num_faces);
            End_Edge_Struct(num_vert);
            *cost = 0;
            face_id = 0;
            *quads = false;
            Start_Face_Struct(num_faces-face_id);
            num_faces = num_faces - face_id;
            Free_Strips();
        }
    }
  
    num_faces = face_id;                                                                                                    /* Done reading in all the information into data structures */
    fclose (file);
    free(vertices);
    free(nvertices);
}

//=======================================================================================================================================================================================================================
// Program entry point. It will start by loading in a .obj file then it will convert the polygonal model into triangle strips.
//=======================================================================================================================================================================================================================

int main (int argc, char *argv[])
{
    FILE* bands;
  
    int num_buffers = 0;
    int swaps, strips, cost = 0, triangles;
    int f, t, tr, g;  
    int num_vert = 0,
        num_nvert    = 0,
        num_texture  = 0,
        num_tris     = 0,
        num_faces    = 0;
  
    bool quads = false;
    
    START
                                                                                                                            /* Scan the file once to find out the number of vertices, normals, and faces so we can set up some memory structures */
    float norm_difference = get_options(argc, argv, &f, &t, &tr, &g, &orient);                                              /* Interpret the options specified */
    const char* fname = argv[argc - 1];    
    const char* file_open = (f == BINARY) ? "rb" : "r";
    
    GetFileDataInfo(fname, file_open, &num_vert, &num_nvert, &num_texture, &num_faces, &num_tris, &num_buffers, f);         /* Read in the number of vert, nverts, textures, faces, and tri */
  
    printf("Stripe v2.0\n   Input File  : %s\n", fname);

    bands = OpenOutputFile(fname);                                                                                          /* Open the output file and load the object into memory */
    ReadFile(fname, file_open, bands, num_vert, num_nvert, num_texture, num_faces, num_tris, num_buffers, f, g, t, tr, norm_difference, &quads, &cost);
  
    Start_Edge_Struct(num_vert);
    Find_Adjacencies(num_faces);
      
    face_array = NULL;                                                                                                      /* Initialize it */
    Init_Table_SGI(num_faces);
      
    Build_SGI_Table(num_faces);                                                                                             /* Build it */
    InitStripTable();
  
    if (quads)
    {        
        Save_Walks(num_faces);                                                                                              /* Code for lengths of walks in each direction */
        Find_Bands(num_faces, bands, &swaps, &strips, &cost, &triangles, num_nvert, vert_norms, num_texture, vert_texture); /* Code for finding the bands */
        Save_Rest(&num_faces);                                                                                              /*  Remove the faces that we did  so that we can run the strip code on the rest of the faces that are left */
        fprintf(bands,"\n#local\n");
        End_Edge_Struct(num_vert);
        Start_Edge_Struct(num_vert);
        Find_Adjacencies(num_faces);
    }
  
    SGI_Strip(num_faces, bands, t, tr);                                                                                     /* Get the total cost */
    
    Output_TriEx(-1, -2, -3, NULL, -20, cost);  
    End_Face_Struct(num_faces);
    End_Edge_Struct(num_vert);
    fclose(bands);
    STOP
    
    get_time();
    return 0;
}