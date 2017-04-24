#include	<stdio.h>
#include	<math.h>
#include	<stdlib.h>
#include	<string.h>
#include        <sys/time.h>

/*------------------ various consts ----------------------------------*/

#define 	X   		0
#define 	Y   		1
#define 	Z   		2

#define		TRUE		1
#define		FALSE		0

/*-------------------- general-purpose macros ------------------------*/

#define START gettimeofday(&tm,&tz);\
                et = (tm.tv_sec)+ (0.000001* (tm.tv_usec));

#define STOP gettimeofday(&tm,&tz);\
                et = (tm.tv_sec)+(0.000001*(tm.tv_usec)) - et;

#define  ALLOCN(PTR,TYPE,N) 					\
	{ PTR = (TYPE *) malloc(((unsigned)(N))*sizeof(TYPE));	\
	  if (PTR == NULL) {    				\
	  printf("malloc failed");				\
	  exit(-1);                                             \
	  }							\
	}

/*---------------------------structs-------------------------------------*/
typedef struct
{ float			radius;
  float			center[3];
  int		type;
} Gp_Atom;

/*---------------------------variables-------------------------------------*/
/*extern*/ Gp_Atom          *atoms;
/*extern*/ int              Num_atoms;
/*extern*/ struct timeval   tm;
/*extern*/ struct timezone  tz;
/*extern*/ double           et;
