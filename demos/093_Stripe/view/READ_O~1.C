/********************************************************************/
/*   Viewobjf: a viewer that uses OpenGL to draw a .obj or .objf data
     file.
     Francine Evans, 1996.
     SUNY @ Stony Brook
     Advisors: Steven Skiena and Amitabh Varshney
*/
/********************************************************************/

/*---------------------------------------------------------------------*/
/*   Viewobjf: read_objf.c
     This file contains the code that reads in the data file 
     (.obj or .objf)
-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"
#include "normal.h"

#define LINE 600                         

void save_bounding_box(double x,double y,double z,double *xmax,
                       double *xmin,double *ymax,double *ymin,double *zmax,double *zmin)
{
     /*   At a new vertex, see if we can save it as a max or
          min pt in our bounding box
     */
     if (x < *xmin)
          *xmin = x;
     if (x > *xmax)
          *xmax = x;
     
     if (y < *ymin)
          *ymin = y;
     if (y > *ymax)
          *ymax = y;
     
     if (z < *zmin)
          *zmin = z;
     if (z > *zmax)
          *zmax = z;
}

struct face_struct *readobj (int argc, char **argv, int *num_f, int *norms, double *xmax,
                             double *xmin,double *ymax,double *ymin,double *zmax,double *zmin)
{
     /*   Read the object into our data structure */

     char *fname;
     char	buff[LINE], *ptr;
     FILE	*file;
     int tempi, loop, num;
     double n1,n2,n3;

     struct vert_struct	*vertices	     = NULL,
				     *nvertices	= NULL,
				     *pvertices	= NULL,
				     *pnvertices	= NULL;

     struct color_struct	*colors		= NULL,
				     *pcolors	     = NULL,
				     *c_color	     = NULL;

     struct face_struct	*faces		= NULL,
				     *pfaces		= NULL;

     struct vert_struct	**tvert		= NULL;
     struct vert_struct	**index,**cp_index;

     struct dset_struct	*dset     = NULL;

	long	tcolor=0.0;

	int  num_vert	 = 0,
	     num_faces	 = 0,
	     num_nvert	 = 0,
	     num_edges	 = 0,
          old_tris   = -1,
          num_tris   = 0;
     
     int vertex,normal,texture,normal_and_texture;
     /*
	** Scan the file once to find out the number of vertices,
	** vertice normals, and faces so we can set up some memory
	** structures 
	*/

	fname = argv[argc-1];
	/*printf ("File: %s\n",fname);

	printf ("Scanning...");*/

    	if (file = fopen (fname,"r")) 
	{
		while (!feof (file)) 
		{
			fgets (buff, 254, file);
			/*   Vertex */
               if (*buff == 'v') 
			{
				/*   Normal Vertex */
                    if (*(buff+1)=='n') 
					num_nvert++;
				/*   Texture ignore it */
                    if (*(buff+1) =='t')
                         ;
                    /*   Vertex */
                    else 
					num_vert++;
			} 
               /*   Face or triangle strip */
               else if ((*buff == 'f') || (*buff == 't') || (*buff == 'q'))
			{  
                    num_faces++;
			     strtok(buff, " ");
			     tempi = 0;
			     while (strtok(NULL, " ") != NULL) tempi++;
               }

		}
		fclose (file);
	}     

	else
	{
		printf("Error in the file name\n");
		exit(0);
	}
	
	
	/*** Allocate structures for the information ***/

	vertices = (struct vert_struct *)
			malloc (sizeof (struct vert_struct) * num_vert);
	faces = (struct face_struct *)
			malloc (sizeof (struct face_struct) * (num_faces+1));

     if (num_nvert > 0) 
	    nvertices = (struct vert_struct *)
			      malloc (sizeof (struct vert_struct) * num_nvert);
	else 
	    nvertices = (struct vert_struct *)
                    malloc (sizeof (struct vert_struct) * num_vert);
	
     /*   We will be calculating the normals if there are not any, for shading
          purposes.
     */
     if (num_nvert == 0)
		initialize_normal(nvertices,num_vert);

      
	/*** Set up the temporary 'p' pointers  ***/
	pvertices  = vertices;
	pnvertices = nvertices;
	pfaces = faces;

	/*** Load the object into memory ***/
     /*printf (" Loading...");*/
	
	if (file = fopen (fname,"r")) 
	{
		while (!feof (file)) 
		{
			fgets (buff, 600, file);

			/*** Load in vertices/normals ***/

			if (*buff == 'v') 
			{
				if (*(buff+1)=='n') 
				{
					sscanf (buff+3,"%lf%lf%lf",
						&(pnvertices->x),
						&(pnvertices->y),
						&(pnvertices->z));
					++pnvertices;
				} 
				else if (*(buff+1)=='t')
                         ; /* ignoring the texture */
                    else 
				{
					sscanf (buff+2,"%lf%lf%lf",
						&(pvertices->x),
						&(pvertices->y),
						&(pvertices->z));
                         /*   At a vertex, see if we can save in bounding box */
                         save_bounding_box(pvertices->x,pvertices->y,pvertices->z,
                              xmax,xmin,ymax,ymin,zmax,zmin);
                         ++pvertices;
				}

			} 
			
			else if ((*buff == 'f') || (*buff == 't') || (*buff == 'q'))
			{
				/*	We are at the start of a new face, a new strip or are in
                         the middle of a triangle strip.
				*/
                    normal = 0; texture = 0; normal_and_texture = 0;
		          if (*buff == 't') 
                    {
                         /*   Signalling the start of a new tstrip */
                         normal_triangle (-1,0,0,0,&n1,&n2,&n3);
                         num_tris = num_tris - 2;
                         faces->tri_strip = 1;
                         old_tris = -2;
                    }
                    else if (*buff == 'q')
                         faces->tri_strip = 2;
                    else
                    {
                         faces->tri_strip = 0;
                         old_tris = num_tris;
			          normal_triangle (-1,0,0,0,&n1,&n2,&n3);
                    }
                    
                    num = 0;
				ptr = buff+1;
				while (*ptr) 
				{
				   if (*ptr >='0' && *ptr <='9') 
				   {
					num++; 
					++ptr;
					while (*ptr && (*ptr!=' ' && *ptr!='/')) 
    						ptr++;
                         /*   There are normals in this line */
                         if (*ptr == '/')
                         {
                              if (*(ptr+1) == '/')
                                   normal = 1;
                              else
                                   texture = 1;
			          }
                         else if (*ptr == ' ')
                         {
                              if ((num == 3) && (texture))
                                   normal_and_texture = 1;
                         }

				   } 
				   else 
				   	++ptr;		
                    
                    }

				/*** Allocate structure big enough to hold
				     vertice pointers and normal pointers ***/

				ptr = buff+1;
			     faces->index = index = (struct vert_struct **)
			              malloc (sizeof (struct vert_struct *) * num * 2);
                    cp_index = index;
                    if ((texture) && (!(num_nvert)) && (!normal_and_texture))
                         faces->num = num/2;
                    else if (normal_and_texture)
                         faces->num = num - (num/3);
			     else if ((num_nvert) && (!normal) && (!texture))
                    {
                         faces->num = num * 2;
			          index = realloc(index,(sizeof (struct vert_struct *)) * (num * 2));
                         faces->index = index;
                    }
                    else if (num_nvert == 0)
                         faces->num = num * 2;
                    else 
                         faces->num = num;

                    /*** loop on the number of numbers in this
					line of strip data ***/
						
				for (loop=0;loop<num;loop++) 
				{
					/*** skip the whitespace ***/
					while (*ptr<'0' || *ptr>'9') 
                         {
						if (*ptr == '-')
                                   break;
                              ptr++;  
                         }
  					/*** If there are nvertices in this
						file, the data alternates so
						we must read it this way ***/

                         vertex = atoi(ptr) - 1;
                         if (vertex < 0)
                         {
                              vertex = num_vert + vertex;
                              *ptr = ' ';
                              ptr++;
                         }
                         if ((normal) && (!normal_and_texture))
					{
					   	if (loop%2)
							*(index-1)=&nvertices[vertex];
                              else
                              {
                                   *(index+1)=&vertices[vertex];
                                   num_tris++;
                              }
					} 
                         else if (normal_and_texture)
                         {
                              if( !((loop+1)%3))
							*(index-1)=&nvertices[vertex];
                              /*   the vertex */
                              else if ((loop == 0) || (*(ptr-1) == ' '))
                              {
                                   *(index+1)=&vertices[vertex];
                                   num_tris++;
                              }
                              else 
                                   index--;
                         }
                         else if (texture)
                         {
                              if (!(loop%2))
                              {
                                   if (num_nvert)
                                        *(index+1)=&vertices[vertex];
                                   else
                                        *(index) = &vertices[vertex];
                                   num_tris++;
                              }
                              else if (num_nvert)
                                   /*   fill normal data with dummy data  */
                    			*(index-1)=&nvertices[0];
                              else 
                                   index--;
                         }
					else 
                         {
					    	if (num_nvert == 0)
                              {
                                   /*** no normals in the dataset ***/
                                   normal_triangle (vertex,vertices[vertex].x,vertices[vertex].y,vertices[vertex].z,nvertices);
						     *(index) = &nvertices[vertex];
                                   *(index+1) = &vertices[vertex];
                                   num_tris++;
                                   ++index;
                              }
                              else
                              {
                                   /*   Fill with dummy */
							*(index)=&nvertices[0];
                                   *(index+1)=&vertices[vertex];
                                   num_tris++;
                                   ++index;
                              }
                         }
                        
					while (*ptr>='0' && *ptr<='9') 
						ptr++;
					++index;
				}
                    /*   So that the count is right of the number of faces if it is
                         not in triangle strips
                    */
                    if (old_tris >= 0)
                    {
                         num_tris = old_tris + 1;
                         old_tris = -1;
                    }
                    faces++;
			}
		}	
	fclose (file);
	if ((num_nvert == 0) && (!texture))
		normalize(nvertices,num_vert);
     if (num_tris == -2)
          num_tris = num_tris - 2;

	/*printf(" Done.\n\n");
	printf ("Vertices:	%d\nNormals:	%d\nFaces:		%d\n",num_vert,num_nvert,num_tris);*/
     *num_f = num_faces;
     if ((num_nvert == 0) && (!texture))
          *norms = num_nvert+1;
     else
          *norms = num_nvert;
     return pfaces;
}
}


