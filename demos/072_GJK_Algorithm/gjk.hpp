#include <cstdio>
#include <cstdlib>

struct Object_structure 
{
    int numpoints;
    double (* vertices)[3];
    int* rings;
};

typedef struct Object_structure* Object;
typedef double (* Transform)[4];

struct simplex_point
{
    int npts;                                                             	// number of points in this simplex
    int simplex1[4];                                           				// simplex1 and simplex2 are two arrays of indices into the point arrays, given by the user.
    int simplex2[4];
    double lambdas[4];                                              		// and lambdas gives the values of lambda associated with those points.
    double coords1[4][3];
    double coords2[4][3];
    int last_best1, last_best2;                                      		// calculated coordinates from the last iteration last maximal vertices, used for hill-climbing
    double error;                                                         	// indication of maximum error in the return value
};

#define EPSILON ((double) 1.0e-8)
#define TINY	((double) 1.0e-20)
#define MAX_RING_SIZE_MULTIPLIER	8

int gjk_extract_point( struct simplex_point *simp, int whichpoint, double vector[]);
double gjk_distance(Object obj1, double (* tr1)[4], Object obj2, double (* tr2)[4], double wpt1[3], double wpt2[3], struct simplex_point * simplex, int use_seed);



struct difference_simplex
{
    int dimension;
    glm::ivec4 A, B;
    glm::dvec4 coeff;
};


double 
