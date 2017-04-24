/********************************************************************/
/*   Viewobjf: a viewer that uses OpenGL to draw a .obj or .objf data
     file.
     Francine Evans, 1996.
     SUNY @ Stony Brook
     Advisors: Steven Skiena and Amitabh Varshney
*/
/********************************************************************/

/*---------------------------------------------------------------------*/
/*   viewobjf: ogl.c
     This file contains the OpenGL code.
-----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "global.h"

#ifdef WIN32
#include "glos.h" /* Windows specific */
#include <GL/glaux.h>
#include <sys/timeb.h>
#include <time.h>
#define START
#define STOP

#else
#define CALLBACK 
#include "aux.h"
#include "macros.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/times.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <time.h>

#define XYZ_HEIGHT 480
#define XYZ_WIDTH 320
#define XOFF 60
#define YOFF 60
#define PI 3.141592635
#define ROT_FAC    5
#define TRANS_FAC  1

/*   globals with the obj saved */
struct face_struct *f;
int n,nor,c=0;
double angle;
double scale = 1.0 ;
double maxz,minz;
double colorx = 0, colory = 1, colorz = 1;

static double fov = 60.0;
static int leftbutton = FALSE, rightbutton = FALSE;
static int width = 320, height = 480;
static int origx, origy;
static int ox, oy;
GLfloat light_position[] = {0.0, 50.0, 50.0, 0.0};
GLfloat light_position2[] = {0.0, 0, 1.0, 0.0};
GLfloat light_ambient[]  = {0.4, 0.4, 0.4, 0.5};
GLfloat plastic_ambt[] = {0.9, 0.9, 0.9, 1.0};
GLfloat plastic_spec[] = {0.8, 0.8, 0.8, 1.0};
GLfloat plastic_shin[] = { 50.0 };
GLfloat plastic_diff[] = {0.8, 0.75, 0.12, 1.0};


void CALLBACK display(void);

double zscale=1.0;

/*   Globals for movement */
static int   		incpitch=0, incyaw=0;
float 		xtrans = 0, ytrans = 0, ztrans = 0;
double         halfx = 160, halfy = 240;



/*---------------------------------------------------------------------
the following functions define the rotation and translation functions
---------------------------------------------------------------------*/

void CALLBACK incpitch_add( void )
{ 
  incpitch += ROT_FAC;
}

void CALLBACK incpitch_sub( void )
{ 
  incpitch -= ROT_FAC;
}

void CALLBACK incyaw_add( void )
{ 
  incyaw += ROT_FAC;
}

void CALLBACK incyaw_sub( void )
{ 
  incyaw -= ROT_FAC;
}

void CALLBACK xtrans_add( void )
{ 
  xtrans += TRANS_FAC;
}

void CALLBACK xtrans_sub( void )
{ 
  xtrans -= TRANS_FAC;
}

void CALLBACK ytrans_add( void )
{ 
  ytrans += TRANS_FAC;
}

void CALLBACK ytrans_sub( void )
{ 
  ytrans -= TRANS_FAC;
}

void CALLBACK ztrans_add( void )
{ 
  ztrans += TRANS_FAC;
}

void CALLBACK ztrans_sub( void )
{ 
  ztrans -= TRANS_FAC;
}


void CALLBACK idle (void)
{
  if (leftbutton)
  {
    int x, y;

    auxGetMouseLoc (&x, &y);

    if (x != ox  ||  y != oy)
    {
      incpitch += 2 * ((float)(x - ox) * fov) / width;
      incyaw += 2 * ((float)(y - oy) * fov) / height;

      ox = x;
      oy = y;
    }
  }
  else if (rightbutton)
  {
    int x, y;

    auxGetMouseLoc (&x, &y);

    if (x != ox  ||  y != oy)
    {
      fov += ((y - origy) * 90) / height;

      if (fov > 140)
      fov = 140;

      if (fov < 20)
        fov = 20;


      ox = x;
      oy = y;

    }
  }
}

void CALLBACK mousemove (AUX_EVENTREC *ev)
{
  idle();
}


void CALLBACK leftdown (AUX_EVENTREC *ev)
{
  ox = origx = ev->data[AUX_MOUSEX];
  oy = origy = ev->data[AUX_MOUSEY];

  leftbutton = TRUE;
}


void CALLBACK leftup (AUX_EVENTREC *ev)
{
  leftbutton = FALSE;
}

void CALLBACK middledown(AUX_EVENTREC *ev)
{
	/*	Reset display */
	incpitch = 0;
 	incyaw = 0;
}

void CALLBACK rightdown (AUX_EVENTREC *ev)
{
  ox = origx = ev->data[AUX_MOUSEX];
  oy = origy = ev->data[AUX_MOUSEY];

  rightbutton = TRUE;
}

void CALLBACK rightup (AUX_EVENTREC *ev)
{
  rightbutton = FALSE;
}


/*---------------------------------------------------------------------
the following functions determine how much to scale to
---------------------------------------------------------------------*/

void get_scale (double xmax,double xmin,double ymax,
                  double ymin,double zmax,double zmin)
{
     double x, y;
     
     /* larger than the window, scale down */
     if (ymax > (height/4))
          scale = scale * ((height/4)/ymax);
     if (ymin < ((height/4) * -1))
     {
          if (scale > (fabs(ymin)/(height/4)))
               scale = fabs(ymin)/(height/4);
     }
     if (xmax > (width/4))
     {
          if (scale > ((width/4)/xmax))
               scale = (width/4)/xmax;
     }
     if (xmin < ((width/4) * -1))
     {
          if (scale > (fabs(xmin)/(width/4)))
               scale = fabs(xmin)/(width/4);
     }
     if (scale != 1)
          return;

     /*   smaller than the window, scale up */     
     if (fabs(xmax) > fabs(xmin))
          x = fabs(xmax);
     else 
          x = fabs(xmin);

     if (fabs(ymax) > fabs(ymin))
          y = fabs(ymax);
     else
          y = fabs(ymin);
     
     if (x > y)
          scale = (width/4)/x;
     else
          scale = (height/4)/y;

     /*printf ("SCALE:%lf %lf %lf\n",x,y,z);*/
}

/*---------------------------------------------------------------------
the following functions are the drawing functions
---------------------------------------------------------------------*/


void CALLBACK ChangeSize(GLsizei w, GLsizei h)
{
     GLfloat nRange = 100.0f;

     if (h==0)
          h = 1;

     glViewport(0,0,w,h);

     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();

     if (w <= h)
          glOrtho(-nRange,nRange,-nRange*h/w,nRange*h/w,-nRange*2.0f,nRange*2.0f);
     else
          glOrtho(-nRange*w/h,nRange*w/h,-nRange,nRange,-nRange*2.0f,nRange*2.0f);

     glMatrixMode(GL_MODELVIEW);
     glLoadIdentity();
}


void RenderHead(void)
	{
     int q=0,numverts,fn,a;
     struct face_struct *cp_f;

     cp_f = f;
	
	glColor3f(1.0f, 1.0f, 1.7f);
     glScaled(scale,scale,1); 
     glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

	glFrontFace(GL_CW);			

     if (incpitch) 
          glRotatef((GLfloat)incpitch, 1.0,0.0,0.0); /* about X-axis*/
     
     if (incyaw) 
          glRotatef((GLfloat)incyaw, 0.0,1.0,0.0); /* about Y-axis */


     for (a = 0; a < n; a++)
     {

	     if (f->tri_strip == 1)
               glBegin(GL_TRIANGLE_STRIP);

          else if (f->tri_strip == 0)
	           glBegin(GL_POLYGON);
          
          numverts = f->num;
          for (fn = 0; fn< numverts; fn++)
          {
               if (nor) 
               {
                    if (!(fn%2))
                         glNormal3d( (GLdouble) (*(f->index+fn))->x,(GLdouble) (*(f->index+fn))->y,
                         (GLdouble) (*(f->index+fn))->z);
                    else
                         glVertex3d( (GLdouble) (*(f->index+fn))->x,(GLdouble) (*(f->index+fn))->y,
                         (GLdouble) (*(f->index+fn))->z);     
               } 
               else 
                    /*** no normals in the dataset ***/
                    glVertex3d( (GLdouble) (*(f->index+fn))->x,(GLdouble) (*(f->index+fn))->y,
                       (GLdouble) (*(f->index+fn))->z);  

               glColor3f(colorx,colory,colorz);
          }
          f++;
          if (f->tri_strip != 2)
          {
               q=0;
               glEnd();
          }
          else 
               q=1;

     }
     if (q == 1)
          glEnd();

     f = cp_f;
}



/* This function does any needed initialization on the rendering
   context.  Here it sets up and initializes the lighting for
   the scene.
*/
void SetupRC()
	{
	/* Light values and coordinates */
	GLfloat  ambientLight[] = {0.4f, 0.4f, 0.4f, 1.0f };
	GLfloat  diffuseLight[] = {0.7f, 0.7f, 0.7f, 1.0f };
	GLfloat  specular[] = { 0.9f, 0.9f, 0.9f, 1.0f};
	GLfloat	 lightPos[] = { -50.0f, 200.0f, 200.0f, 1.0f };
	GLfloat  specref[] =  { 0.6f, 0.6f, 0.6f, 1.0f };

	/* Enable lighting */
	glEnable(GL_LIGHTING);

	/* Setup light */
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambientLight);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambientLight);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuseLight);
	glLightfv(GL_LIGHT0,GL_SPECULAR,specular);

	/* Position and turn on the light */
	glLightfv(GL_LIGHT0,GL_POSITION,lightPos);
	glEnable(GL_LIGHT0);

	/* Enable color tracking */
	glEnable(GL_COLOR_MATERIAL);
	
	/* Set Material properties to follow glColor values */
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	/* All materials hereafter have full specular reflectivity
	   with a moderate shine
     */
	glMaterialfv(GL_FRONT, GL_SPECULAR,specref);
	glMateriali(GL_FRONT,GL_SHININESS,64);

	/* Black background */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

     auxKeyFunc(AUX_LEFT, incyaw_sub);
     auxKeyFunc(AUX_RIGHT, incyaw_add);
     auxKeyFunc(AUX_UP, incpitch_sub);
     auxKeyFunc(AUX_DOWN, incpitch_add);
     auxKeyFunc(AUX_j, ztrans_add);
     auxKeyFunc(AUX_k, ztrans_sub);
     auxKeyFunc(AUX_i, ytrans_add);
     auxKeyFunc(AUX_m, ytrans_sub);
     auxKeyFunc(AUX_h, xtrans_add);
     auxKeyFunc(AUX_l, xtrans_sub);  
	
	auxMouseFunc (AUX_LEFTBUTTON, AUX_MOUSEDOWN, leftdown);
  	auxMouseFunc (AUX_LEFTBUTTON, AUX_MOUSEUP, leftup);

  	auxMouseFunc (AUX_RIGHTBUTTON, AUX_MOUSEDOWN, rightdown);
  	auxMouseFunc (AUX_RIGHTBUTTON, AUX_MOUSEUP, rightup);

	auxMouseFunc(AUX_MIDDLEBUTTON, AUX_MOUSEDOWN, middledown);

     #ifdef WIN32
  	auxMouseFunc (AUX_LEFTBUTTON, AUX_MOUSELOC, mousemove);
  	auxMouseFunc (AUX_RIGHTBUTTON, AUX_MOUSELOC, mousemove);
  	auxMouseFunc (AUX_MIDDLEBUTTON, AUX_MOUSELOC, mousemove);
	#else
  	auxIdleFunc (idle);
	#endif

	}

/* Called to draw the model */
void CALLBACK RenderScene(void)
	{
          #ifdef WIN32
          struct _timeb timebuffer;
          char *timeline;
          #endif
          
#ifdef WIN32
          _ftime( &timebuffer );
          timeline = ctime( & ( timebuffer.time ) );
          printf( "The time is %.19s.%hu %s", timeline, timebuffer.millitm, &timeline[20] );
#endif


START

	/* Clear the window with current clearing color */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Save the matrix state and do the rotations */
	glMatrixMode(GL_MODELVIEW);

	/* Rotate and translate, then render the bolt head */
	glPushMatrix();
		glRotatef(0, 1.0f, 0.0f, 0.0f);
		glRotatef(incyaw, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, 0.0f, 55.0f);
		RenderHead();
	glPopMatrix();

	glFlush();


#ifdef WIN32
     _ftime( &timebuffer );
     timeline = ctime( & ( timebuffer.time ) );
     printf( "The time is %.19s.%hu %s", timeline, timebuffer.millitm, &timeline[20] );
#else
     STOP
     printf("Time for last frame  = %lf seconds\n", et);
#endif

}


void init_display(double xmax,double xmin,double ymax,
                  double ymin,double zmax,double zmin)
{

	/*   Determine the scaling function */
     get_scale(xmax,xmin,ymax,ymin,zmax,zmin);
     #ifdef WIN32
     auxInitDisplayMode(AUX_SINGLE | AUX_RGBA | AUX_DEPTH);
     #else
     auxInitDisplayMode(AUX_DOUBLE | AUX_RGBA | AUX_DEPTH);
     #endif
     auxInitPosition(XOFF,YOFF,XYZ_WIDTH,XYZ_HEIGHT);
     auxInitWindow("Viewobjf");
     SetupRC();
}


void draw(struct face_struct *faces, int num_faces, int normals, int color)
{
     /*   Start the drawing */
     f = faces;
     c = color;
     n = num_faces;
     nor = normals;

     auxReshapeFunc(ChangeSize);
     auxMainLoop(RenderScene);

}


