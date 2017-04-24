/********************************************************************/
/*   Viewobjf: a viewer that uses OpenGL to draw a .obj or .objf data
     file.
     Francine Evans, 1996.
     SUNY @ Stony Brook
     Advisors: Steven Skiena and Amitabh Varshney
*/
/********************************************************************/

/*---------------------------------------------------------------------*/
/*   viewobjf: viewobjf.c
     This file contains the main procedure code 
-----------------------------------------------------------------------*/

  
#include <stdio.h>
#include <stdlib.h>
#include "viewobjf.h"
#include "ogl.h"

#define MAX_BB 100000
#define MIN_BB -100000

void init_bounding_box(double *xmax,double *xmin,double *ymax,double *ymin,
                       double *zmax,double *zmin)
{
       /*   We will save the bounding box of the object, for display purposes */
          
     *xmin = MAX_BB;
     *xmax = MIN_BB;
     *ymin = MAX_BB;
     *ymax = MIN_BB;
     *zmin = MAX_BB;
     *zmax = MIN_BB;
}

void main (int argc, char **argv)
{
     struct face_struct *faces;
     int num_faces,normals,color;
     double xmax,xmin,ymax,ymin,zmax,zmin;
     
     init_bounding_box(&xmax,&xmin,&ymax,&ymin,&zmax,&zmin);
     faces = readobj(argc,argv,&num_faces,&normals,&xmax,&xmin,&ymax,&ymin,&zmax,&zmin);
     init_display(xmax,xmin,ymax,ymin,zmax,zmin);
     draw(faces,num_faces,normals,color);
}


