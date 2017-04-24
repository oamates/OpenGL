/********************************************************************/
/*   Viewobjf: a viewer that uses OpenGL to draw a .obj or .objf data
     file.
     Francine Evans, 1996.
     SUNY @ Stony Brook
     Advisors: Steven Skiena and Amitabh Varshney
*/
/********************************************************************/

/*---------------------------------------------------------------------*/
/*   viewobjf: normal.c
     This file contains the code that is used to calculate normals.
-----------------------------------------------------------------------*/

#include <math.h>
#include "global.h"
#include "linalg.h"


void initialize_normal(struct vert_struct *normal, int vert)
{
	int x;

	/*	Initialize the normal array */
	for (x = 0; x < vert; x++)
	{
		normal[x].x = 0;
		normal[x].y = 0;
		normal[x].z = 0;
	}
}

void normalize(struct vert_struct *normal, int vert)
{
	/*	Normalize the normal array */
	int x;
	double n[3];
	LINALGINIT;
	
	for (x = 0; x < vert; x++)
	{
		n[0] = normal[x].x;
		n[1] = normal[x].y;
		n[2] = normal[x].z;
		NORMALIZE3(n);
		normal[x].x = n[0];
		normal[x].y = n[1];
		normal[x].z = n[2];
	}
}

/*   Compute the normal for a triangle */
void get_normal(double v1, double v2, double v3,
                double v4, double v5, double v6,
                double v7, double v8, double v9,
                double *x, double *y, double *z)
{
     double vector1[3], vector2[3], cross[3];

     vector1[0] = v1 - v4;
     vector1[1] = v2 - v5;
     vector1[2] = v3 - v6;

     vector2[0] = v4 - v7;
     vector2[1] = v5 - v8;
     vector2[2] = v6 - v9;

     CROSSPROD3(cross, vector1, vector2);

     *x = cross[0];
     *y = cross[1];
     *z =  cross[2];

}

/*   We are given the next vertex in the triangle strip and now we have
     to return the normal for the triangle that is formed.
*/

void normal_triangle (int id, double x, double y, double z,
                      struct vert_struct *normal)
{
     static int last[2];
     static int index;
     double nx,ny,nz;
     static struct coord 
     {
          double x;
          double y;
          double z;
     } c[2];
     int start;

     if (id == -1)
     {
          /*   Start of triangle strip */
          last[0] = -1;
          last[1] = -1;
          index = 0;
          start = 1;
     }
     else if (last[index] == -1)
     {
          /*   Filling in the first 2 vertices of the strip */
          last[index] = id;
          c[index].x = x; c[index].y = y; c[index].z = z;
          index = !index;
     }
     else if (last[!index] == id)
          /*   Degenerate triangle because of a swap */
          index = !index;
     else
     {
          /*   At a real triangle */
          get_normal(c[0].x,c[0].y,c[0].z,c[1].x,c[1].y,c[1].z,x,y,z,&nx,&ny,&nz);
          /*   If start, do the last 2 vertices, since they were the first two in
               the strip 
          */
          if (start)
          {
               normal[last[index]].x += nx; normal[last[index]].y += ny; normal[last[index]].z += nz;
               normal[last[!index]].x += nx; normal[last[!index]].y += ny; normal[last[!index]].z += nz;
               start = !start;
          }
          
          last[index] = id;
          
          c[index].x = x; c[index].y = y; c[index].z = z;
          index = !index;
          normal[id].x += nx; normal[id].y += ny; normal[id].z += nz;
     }
}
