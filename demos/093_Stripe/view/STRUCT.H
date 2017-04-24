/********************************************************************/
/*   Viewobjf: a viewer that uses OpenGL to draw a .obj or .objf data
     file.
     Francine Evans, 1996.
     SUNY @ Stony Brook
     Advisors: Steven Skiena and Amitabh Varshney
*/
/********************************************************************/

/*---------------------------------------------------------------------*/
/*   viewobjf: struct.h                                                  
-----------------------------------------------------------------------*/


struct vert_struct {
	VRDATA	x, y, z;	/* point coordinates */
};

struct face_struct {
	int  tri_strip;
	long			num;	/* number of edges/face */
	struct color_struct	*color;
	struct vert_struct 	**index; /* indexes to the object points */
};

void Start_Vert_Struct();
