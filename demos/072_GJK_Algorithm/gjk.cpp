#include "gjk.hpp"

#include <glm/glm.hpp>

// Implementation of the Gilbert, Johnson and Keerthi routine to compute the minimum distance between two convex polyhedra.
// Tables encode the constant topology of simplices in DIM-dimensional space.  The original
static const int cardinality[16] = { 0,  1,  1,  2,  1,  2,  2,  3,  1,  2,  2,  3,  2,  3,  3,  4};
static const int max_element[16] = {-1,  0,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3};
static const int elements[16][4] = 
{
    {  0,  0,  0,  0},  {  0,  0,  0,  0},  {  1,  0,  0,  0},  {  0,  1,  0,  0},  {  2,  0,  0,  0},  {  0,  2,  0,  0},  {  1,  2,  0,  0},  {  0,  1,  2,  0},
    {  3,  0,  0,  0},  {  0,  3,  0,  0},  {  1,  3,  0,  0},  {  0,  1,  3,  0},  {  2,  3,  0,  0},  {  0,  2,  3,  0},  {  1,  2,  3,  0},  {  0,  1,  2,  3}
};
static const int non_elements[16][4] = 
{
    {  0,  1,  2,  3},  {  1,  2,  3,  0},  {  0,  2,  3,  0},  {  2,  3,  0,  0},  {  0,  1,  3,  0},  {  1,  3,  0,  0},  {  0,  3,  0,  0},  {  3,  0,  0,  0},
    {  0,  1,  2,  0},  {  1,  2,  0,  0},  {  0,  2,  0,  0},  {  2,  0,  0,  0},  {  0,  1,  0,  0},  {  1,  0,  0,  0},  {  0,  0,  0,  0},  {  0,  0,  0,  0}
};
static const int predecessor[16][4] = 
{
    {  0,  0,  0,  0},  {  0,  0,  0,  0},  {  0,  0,  0,  0},  {  2,  1,  0,  0},  {  0,  0,  0,  0},  {  4,  1,  0,  0},  {  4,  2,  0,  0},  {  6,  5,  3,  0},
    {  0,  0,  0,  0},  {  8,  1,  0,  0},  {  8,  2,  0,  0},  { 10,  9,  3,  0},  {  8,  4,  0,  0},  { 12,  9,  5,  0},  { 12, 10,  6,  0},  { 14, 13, 11,  7}
};
static const int successor[16][4] = 
{
    {  1,  2,  4,  8},  {  3,  5,  9,  0},  {  3,  6, 10,  0},  {  7, 11,  0,  0},  {  5,  6, 12,  0},  {  7, 13,  0,  0},  {  7, 14,  0,  0},  { 15,  0,  0,  0},
    {  9, 10, 12,  0},  { 11, 13,  0,  0},  { 11, 14,  0,  0},  { 15,  0,  0,  0},  { 13, 14,  0,  0},  { 15,  0,  0,  0},  { 15,  0,  0,  0},  {  0,  0,  0,  0}
};

static double delta_values[16][4];
static double dot_products[4][4];
static int support_function(Object obj, int, double*, double*);
static int support_simple(Object obj, int, double*, double*);
static int support_hill_climbing(Object obj, int, double*, double*);
static int default_distance(struct simplex_point* simplex);
static void backup_distance(struct simplex_point* simplex);
static void reset_simplex(int subset, struct simplex_point* simplex);
static void compute_subterms(struct simplex_point* s);
static void compute_point(double pt[3], int len, double (*vertices)[3], double lambdas[]);
static void add_simplex_vertex(struct simplex_point* s, int pos, Object obj1, int v1, Transform t1, Object obj2, int v2, Transform t2);

void apply_trans(Transform t, double* src, double* tgt)
{
    if (t)
        for (int i = 0; i < 3; i++) tgt[i] = t[i][0] * src[0] + t[i][1] * src[1] + t[i][2] * src[2] + t[i][3];
    else
        for (int i = 0; i < 3; i++) tgt[i] = src[i];
};

void apply_rot_transpose(Transform t, double* src, double* tgt)
{
    if (t)
        for (int i = 0; i < 3; i++) tgt[i] = t[0][i] * src[0] + t[1][i] * src[1] + t[2][i] * src[2];
    else
        for (int i = 0; i < 3; i++) tgt[i] = src[i];
};


double gjk_distance(Object obj1, Transform tr1, Object obj2, Transform tr2, double wpt1[3], double wpt2[3], struct simplex_point* simplex, int use_seed)
{
    int v, p, maxp, minp;
    double minus_minv, maxv, sqrd, g_val;
    double displacementv[3], reverse_displacementv[3];
    double local_fdisp[3], local_rdisp[3], trv[3];
    double *fdisp, *rdisp;
    struct simplex_point local_simplex;
    int d, use_default, first_iteration, max_iterations;
    double oldsqrd;

    use_default = first_iteration = 1;

    fdisp = tr1 ? local_fdisp : displacementv;
    rdisp = tr2 ? local_rdisp : reverse_displacementv;

    if (simplex == 0)
    {
        use_seed = 0;
        simplex = &local_simplex;
    }

    if (use_seed == 0)
    {
        simplex->simplex1[0] = 0;    simplex->simplex2[0] = 0;
        simplex->npts = 1;           simplex->lambdas[0] = 1.0;
        simplex->last_best1 = 0;     simplex->last_best2 = 0;
        add_simplex_vertex(simplex, 0, obj1, 0, tr1, obj2, 0, tr2);
    }
    else
    {
        // If we are being told to use this seed point, there is a good chance that the near point will be on the current simplex. Besides, if we don't confirm
        // that the seed point given satisfies the invariant (that the witness points given are the closest points on the current simplex) things can and will fall down.
        for (v = 0; v < simplex->npts; v++) 
            add_simplex_vertex(simplex, v, obj1, simplex->simplex1[v], tr1, obj2, simplex->simplex2[v], tr2);
    }

    // The main loop. First compute the distance between the current simplicies, the check whether this gives the globally correct answer, construct new simplices and try again if not.
    max_iterations = obj1->numpoints * obj2->numpoints;

    // Counting the iterations in this way should not be necessary; a while(1) should do just as well.
    while ((max_iterations--) > 0)
    {
        if (simplex->npts == 1) simplex->lambdas[0] = 1.0;
        else
        {
            compute_subterms( simplex);
            if (use_default) use_default = default_distance( simplex);
            if (!use_default) backup_distance( simplex);
        }

        compute_point(wpt1, simplex->npts, simplex->coords1, simplex->lambdas);
        compute_point(wpt2, simplex->npts, simplex->coords2, simplex->lambdas);
        for (d = 0; d < 3; d++)
        {
            displacementv[d] = wpt2[d] - wpt1[d];
            reverse_displacementv[d] = - displacementv[d];
        }
        sqrd = displacementv[0] * displacementv[0] + displacementv[1] * displacementv[1] + displacementv[2] * displacementv[2];

        // if we are using a c-space simplex with DIM_PLUS_ONE points, this is interior to the simplex, and indicates
        // that the original hulls overlap, as does the distance  between them being too small.
        if (sqrd < EPSILON)
        {
            simplex->error = EPSILON;
            return sqrd;
        }

        if (tr1) apply_rot_transpose(tr1, displacementv, fdisp);
        if (tr2) apply_rot_transpose(tr2, reverse_displacementv, rdisp);

        // find the point in obj1 that is maximal in the direction displacement, and the point in obj2 that is minimal in direction displacement
        maxp = support_function(obj1, (use_seed ? simplex->last_best1 : -1), &maxv, fdisp);
        minp = support_function(obj2, (use_seed ? simplex->last_best2 : -1), &minus_minv, rdisp);

        g_val = sqrd + maxv + minus_minv;

        if (tr1) g_val += displacementv[0] * tr1[0][3] + displacementv[1] * tr1[1][3] + displacementv[2] * tr1[2][3];
        if (tr2) g_val += reverse_displacementv[0] * tr2[0][3] + reverse_displacementv[1] * tr2[1][3] + reverse_displacementv[2] * tr2[2][3];

        if (g_val < 0.0) g_val = 0; // not sure how, but it happens!
        if (g_val < EPSILON)                                            // then no better points - finish
        {
            simplex->error = g_val;
            return sqrd;
        }

        // check for good calculation above
        if ((first_iteration || (sqrd < oldsqrd)) && (simplex->npts <= 3))
        {
            // Normal case: add the new c-space points to the current simplex, and call simplex_distance()
            simplex->simplex1[ simplex->npts] = simplex->last_best1 = maxp;
            simplex->simplex2[ simplex->npts] = simplex->last_best2 = minp;
            simplex->lambdas[ simplex->npts] = 0.0;
            add_simplex_vertex( simplex, simplex->npts, obj1, maxp, tr1, obj2, minp, tr2);
            simplex->npts++;
            oldsqrd = sqrd;
            first_iteration = 0;
            use_default = 1;
            continue;
        };

        if (use_default) use_default = 0;
        else
        {
            simplex->error = g_val;
            return sqrd;
        };
    };
    return 0.0; // we never actually get here, but it keeps some fussy compilers happy.
};

// Sets the coordinates of that vector to the coordinates of either the first or second witness point given by the simplex record.

int gjk_extract_point( struct simplex_point *simp, int whichpoint, double vector[])
{
    double (*coords) [3];
    coords = (whichpoint == 1) ? simp->coords1 : simp->coords2;
    compute_point(vector, simp->npts, coords, simp->lambdas);
    return 1;
};

static double delta[16];

// The simplex_distance routine requires the computation of a number of delta terms.  These are computed here.
static void compute_subterms(struct simplex_point* simp)
{
    int size = simp->npts;
    double c_space_points[4][3];
    
    for (int i = 0; i < size; i++)                                                                                              // compute the coordinates of the simplex as C-space obstacle points
        for (int j = 0; j < 3; j++)
            c_space_points[i][j] = simp->coords1[i][j] - simp->coords2[i][j];
    
    for (int i = 0; i < size; i++)                                                                                              // compute the dot product terms
        for (int j = i; j < size; j++)
            dot_products[i][j] = dot_products[j][i] = c_space_points[i][0] * c_space_points[j][0] + c_space_points[i][1] * c_space_points[j][1] + c_space_points[i][2] * c_space_points[j][2];
    
    for (int s = 1; s < 16 && max_element[s] < size; s++)                                                                           // now compute all the delta terms
    {
        if (cardinality[s] <= 1)                                                                                                        // just record delta(s, elts(s, 0))
        {
            delta_values[s][elements[s][0]] = 1.0;
            continue;
        };

        if (cardinality[s] == 2)                                                                                                        // the base case for the recursion 
        {
            delta_values[s][elements[s][0]] = dot_products[elements[s][1]][elements[s][1]] - dot_products[elements[s][1]][elements[s][0]];
            delta_values[s][elements[s][1]] = dot_products[elements[s][0]][elements[s][0]] - dot_products[elements[s][0]][elements[s][1]];
            continue;
        };

        for (int j = 0 ; j < cardinality[s]; j++)                                                                                       // otherwise, card(s) > 2, so use the general case for each element of this subset s, namely elts(s, j)
        {
            int jelt = elements[s][j];
            int jsubset = predecessor[s][j];
            double sum = 0.0;
            for (int i = 0; i < cardinality[jsubset]; i++)                                                                              // for each element of subset jsubset
            {
                int ielt = elements[jsubset][i];
                sum += delta_values[jsubset][ielt] * (dot_products[ielt][elements[jsubset][0]] - dot_products[ielt][jelt]);
            };
            delta_values[s][jelt] = sum;
        };
    };
};

// default_distance is our equivalent of GJK's distance subalgorithm. It is given a c-space simplex as indices of size (up to DIM_PLUS_ONE) points in the master point list, and computes a pair of witness points for the minimum distance 
// vector between the simplices. This vector is indicated by setting the values lambdas[] in the given array, and returning the number of non-zero values of lambda. 
static int default_distance(struct simplex_point* simplex)
{
    int s, j, k, ok;
    int size = simplex->npts;
    for (s = 1; (s < 16) && (max_element[s] < size); s++)                                                   // for every subset s of the given set of points ...
    {
        // delta[s] will accumulate the sum of the delta expressions for this subset, and ok will remain TRUE whilst this subset can still be thought to be a candidate simplex for the shortest distance.
        delta[s] = 0.0;
        ok = 1;
        // Now the first check is whether the simplex formed by this subset holds the foot of the perpendicular from the origin
        // to the point/line/plane passing through the simplex. This will be the case if all the delta terms for each predecessor subset are (strictly) positive.

        for (j = 0; ok && (j < cardinality[s]); j++)
        {
            if (delta_values[s][elements[s][j]] > 0.0)
                delta[s] += delta_values[s][elements[s][j]];
            else
                ok = 0;
        };

        // If the subset survives the previous test, we still need to check whether the true minimum distance is to a larger piece of geometry, or indeed to another piece of geometry of the same dimensionality. A necessary and sufficient 
        // condition for it to fail at this stage is if the delta term for any successor subset is positive, as this indicates a direction on the appropriate higher dimensional simplex in which the distance gets shorter.
        for (k = 0; ok && (k < size - cardinality[s]); k++)
            if (delta_values[successor[s][k]][non_elements[s][k]] > 0) ok = 0;
            
        if (ok && (delta[s] >= TINY)) break;   /* then we've found a viable subset */
    }

    if (ok) reset_simplex(s, simplex);
    return ok;
}

// A version of GJK's `Backup Procedure'. Note that it requires that the delta[s] entries have been computed for all viable s within simplex_distance.
static void backup_distance(struct simplex_point * simplex)
{
    int i, j, k;
    int size = simplex->npts;
    double distsq_num[16], distsq_den[16];
    
    int bests = 0;                                                                                                                  // for every subset s of the given set of points ...
    for (int s = 1; (s < 16) && (max_element[s] < size); s++)
    {
        if (delta[s] <= 0.0) continue;

        for (i = 0; i < cardinality[s]; i++)
            if (delta_values[s][elements[s][i]] <= 0.0) break;

        if (i < cardinality[s]) continue;                                                                                               // otherwise we've found a viable subset
        
        distsq_num[s] = 0.0;
        for (j = 0; j < cardinality[s]; j++)
            for (k = 0; k < cardinality[s]; k++)
                distsq_num[s] += delta_values[s][elements[s][j]] * delta_values[s][elements[s][k]] * dot_products[elements[s][j]][elements[s][k]];

        distsq_den[s] = delta[s] * delta[s];

        if ((bests < 1) || ((distsq_num[s] * distsq_den[bests]) < (distsq_num[bests] * distsq_den[s]))) bests = s;
    }
    reset_simplex(bests, simplex);
}

static void reset_simplex( int subset, struct simplex_point * simplex)
{
    // compute the lambda values that indicate exactly where the witness points lie.  We also fold back the values stored for the indices into the original point arrays, and the transformed coordinates,
    // so that these are ready for subsequent calls.
    for (int j = 0; j < cardinality[subset]; j++)
    {
        // rely on elts(subset, j) >= j, which is true as they are stored in ascending order.
        int oldpos = elements[subset][j];
        if (oldpos != j)
        {
            simplex->simplex1[j] = simplex->simplex1[oldpos];
            simplex->simplex2[j] = simplex->simplex2[oldpos];
            for (int i = 0 ; i < 3; i++)
            {
                simplex->coords1[j][i] = simplex->coords1[oldpos][i];
                simplex->coords2[j][i] = simplex->coords2[oldpos][i];
            };
        };
        simplex->lambdas[j] = delta_values[subset][elements[subset][j]] / delta[subset];
    };
    simplex->npts = cardinality[subset];
};

// The implementation of the support function.  Given a direction and a hull, this function returns a vertex of the hull that is maximal in that direction, and the value 
// (i.e., dot-product of the maximal vertex and the direction) associated. If there is no topological information given for the hull then an exhaustive search of the vertices is used. 
// Otherwise, hill-climbing is performed. If EAGER_HILL_CLIMBING is defined then the hill-climbing moves to a new vertex as soon as a better  vertex is found, and if it is not defined 
// then every vertex leading from the current vertex is explored before moving to the best one. Initial conclusions are that fewer vertices are explored with EAGER_HILL_CLIMBING defined, 
// but that the code runs slighty slower! This is presumably due to pipeline bubbles and/or cache misses.

static int support_function(Object obj, int start, double * supportval, double * direction)
{
    return (obj->rings) ? support_hill_climbing( obj, start, supportval, direction) : support_simple( obj, start, supportval, direction);
}

// Brute-force routine
static int support_simple(Object obj, int start, double * supportval, double * direction)
{
    int p, maxp;
    double maxv, thisv;
    
    p = maxp = 0;
    maxv = obj->vertices[maxp][0] * direction[0] + obj->vertices[maxp][1] * direction[1] + obj->vertices[maxp][2] * direction[2];
    for (p++; p < obj->numpoints; p++)
    {
        thisv = obj->vertices[p][0] * direction[0] + obj->vertices[p][1] * direction[1] + obj->vertices[p][2] * direction[2];
        if (thisv > maxv)
        {
            maxv = thisv;
            maxp = p;
        }
    }
    *supportval = maxv;
    return maxp;
}

// Hill-climbing routine
static int support_hill_climbing(Object obj, int start, double* supportval, double* direction)
{
    int p, maxp, lastvisited, neighbour;
    int index;
    double maxv, thisv;
    p = lastvisited = -1;
    maxp = (start >= 0) ? start : 0;
    maxv = obj->vertices[maxp][0] * direction[0] + obj->vertices[maxp][1] * direction[1] + obj->vertices[maxp][2] * direction[2];
    while (p != maxp)
    {
        p = maxp;
        for (index = obj->rings[p]; obj->rings[index] >= 0; index++)                                                            // Check each neighbour of the current point.
        {
            neighbour = obj->rings[index];
            // Check that we haven't already visited this one in the last outer iteration.  This is to avoid us calculating the dot-product with vertices we've already looked at.
            if (neighbour == lastvisited) continue;
            thisv = obj->vertices[neighbour][0] * direction[0] + obj->vertices[neighbour][1] * direction[1] + obj->vertices[neighbour][2] * direction[2];
            if (thisv > maxv)
            {
                maxv = thisv;
                maxp = neighbour;
                // break;                                                                                               // This line may be uncommented leaving algorithm valid.
            };
        }
        lastvisited = p;
    }
    *supportval = maxv;
    return p;
}

// Computes the coordinates of a simplex point. Takes an array into which the stuff the result, the number of vertices
// that make up a simplex point, one of the point arrays, the indices of which of the points are used in the for the simplex points, and an array of the lambda values.
static void compute_point(double pt[3], int len, double (* vertices)[3], double lambdas[])
{
    for (int d = 0; d < 3; d++)
    {
        pt[d] = 0.0;
        for (int i = 0; i < len; i++) pt[d] += vertices[i][d] * lambdas[i];
    }
}

static void add_simplex_vertex( struct simplex_point * s, int pos, Object obj1, int v1, Transform t1, Object obj2, int v2, Transform t2)
{
    apply_trans(t1, obj1->vertices[v1], s->coords1[pos]);
    apply_trans(t2, obj2->vertices[v2], s->coords2[pos]);
}