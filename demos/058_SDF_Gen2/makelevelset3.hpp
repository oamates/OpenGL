#ifndef MAKELEVELSET3_H
#define MAKELEVELSET3_H

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "array3d.hpp"

//=======================================================================================================================================================================================================================
// tri is a list of triangles in the mesh, and x is the positions of the vertices
// absolute distances will be nearly correct for triangle soup, but a closed mesh is
// needed for accurate signs. Distances for all grid cells within exact_band cells of
// a triangle should be exact; further away a distance is calculated but it might not
// be to the closest triangle - just one nearby.
//=======================================================================================================================================================================================================================
void make_level_set3(const std::vector<glm::uvec3>& faces, const std::vector<glm::dvec3>& positions,
                     const glm::dvec3& origin, double delta, const glm::ivec3& size,
                     array3d<double>& field, const int exact_band = 1);


//=======================================================================================================================================================================================================================
// find distance x0 is from segment x1-x2
//=======================================================================================================================================================================================================================
double point_segment_distance(const glm::dvec3& P, const glm::dvec3& A, const glm::dvec3& B)
{
    glm::dvec3 AB = B - A;
    glm::dvec3 AP = P - A;

    double m2 = glm::length2(AB);
    double t = glm::dot(AP, AB) / m2;
    t = glm::clamp(t, 0.0, 1.0);

    return glm::length(P - glm::mix(A, B, t));
}

//=======================================================================================================================================================================================================================
// find distance x0 is from triangle x1-x2-x3
//=======================================================================================================================================================================================================================
double point_triangle_distance(const glm::dvec3& P, const glm::dvec3& A, const glm::dvec3& B, const glm::dvec3& C)
{
    //===================================================================================================================================================================================================================
    // first find barycentric coordinates of closest point on infinite plane
    //===================================================================================================================================================================================================================
    glm::dvec3 CA = A - C, 
    	       CB = B - C, 
    	       CP = P - C;

    double m13 = glm::length2(CA), 
           m23 = glm::length2(CB), 
           d = glm::dot(CA, CB);

    double invdet = 1.0 / glm::max(m13 * m23 - d * d, 1e-30);
    double a = glm::dot(CA, CP),
           b = glm::dot(CB, CP);

    //===================================================================================================================================================================================================================
    // the barycentric coordinates themselves
    //===================================================================================================================================================================================================================
    double w23 = invdet * (m23 * a - d * b);
    double w31 = invdet * (m13 * b - d * a);
    double w12 = 1.0 - w23 - w31;

    //===================================================================================================================================================================================================================
    // if we're inside the triangle
    //===================================================================================================================================================================================================================
    if (w23 >= 0 && w31 >= 0 && w12 >= 0)
        return glm::length(P - (w23 * A + w31 * B + w12 * C)); 

    //===================================================================================================================================================================================================================
    // clamping to one of the edges
    //===================================================================================================================================================================================================================
    if(w23 > 0)
        return glm::min(point_segment_distance(P, A, B), point_segment_distance(P, A, C));
    if(w31 > 0)
        return glm::min(point_segment_distance(P, A, B), point_segment_distance(P, B, C));

    return glm::min(point_segment_distance(P, A, C), point_segment_distance(P, B, C));
}

void check_neighbour(const std::vector<glm::uvec3>& tri, const std::vector<glm::dvec3>& x,
                            array3d<double>& phi, array3d<int>& closest_tri,
                            const glm::dvec3& g, int i0, int j0, int k0, int i1, int j1, int k1)
{
    if(closest_tri(i1, j1, k1) >= 0)
    {
        unsigned int p, q, r;
        int tri_idx = closest_tri(i1, j1, k1);
        p = tri[tri_idx].x;
        q = tri[tri_idx].y;
        r = tri[tri_idx].z;
        double d = point_triangle_distance(g, x[p], x[q], x[r]);
        if(d < phi(i0, j0, k0))
        {
            phi(i0, j0, k0) = d;
            closest_tri(i0, j0, k0) = closest_tri(i1, j1, k1);
        }
    }
}

void sweep(const std::vector<glm::uvec3>& tri, const std::vector<glm::dvec3>& x,
                  array3d<double>& phi, array3d<int>& closest_tri, const glm::dvec3& origin, double delta,
                  int di, int dj, int dk)
{
    int i0, i1;
    if(di > 0)
        { i0 = 1; i1 = phi.ni; }
    else
        { i0 = phi.ni - 2; i1 = -1; }

    int j0, j1;
    if(dj > 0)
        { j0 = 1; j1 = phi.nj; }
    else
        { j0 = phi.nj - 2; j1 = -1; }

    int k0, k1;
    if(dk > 0)
        { k0 = 1; k1 = phi.nk; }
    else
        { k0 = phi.nk - 2; k1 = -1; }

    for(int k = k0; k != k1; k += dk)
        for(int j = j0; j != j1; j += dj)
            for(int i = i0; i != i1; i += di)
            {
                glm::dvec3 g = origin + glm::dvec3(i, j, k) * delta;
                check_neighbour(tri, x, phi, closest_tri, g, i, j, k, i - di, j,      k);
                check_neighbour(tri, x, phi, closest_tri, g, i, j, k, i,      j - dj, k);
                check_neighbour(tri, x, phi, closest_tri, g, i, j, k, i - di, j - dj, k);
                check_neighbour(tri, x, phi, closest_tri, g, i, j, k, i,      j,      k - dk);
                check_neighbour(tri, x, phi, closest_tri, g, i, j, k, i - di, j,      k - dk);
                check_neighbour(tri, x, phi, closest_tri, g, i, j, k, i,      j - dj, k - dk);
                check_neighbour(tri, x, phi, closest_tri, g, i, j, k, i - di, j - dj, k - dk);
            }
}

//=======================================================================================================================================================================================================================
// calculate twice signed area of triangle (0,0)-(x1,y1)-(x2,y2)
// return an SOS-determined sign (-1, +1, or 0 only if it's a truly degenerate triangle)
//=======================================================================================================================================================================================================================
int orientation(double x1, double y1, double x2, double y2, double& signed_area)
{
    signed_area = y1 * x2 - x1 * y2;

    if(signed_area > 0.0) return 1;
    if(signed_area < 0.0) return -1;
    if(y2 > y1) return  1;
    if(y2 < y1) return -1;
    if(x1 > x2) return  1;
    if(x1 < x2) return -1;
    return 0;
}

//=======================================================================================================================================================================================================================
// robust test of (x0,y0) in the triangle (x1,y1)-(x2,y2)-(x3,y3)
// if true is returned, the barycentric coordinates are set in a,b,c.
//=======================================================================================================================================================================================================================
static bool point_in_triangle_2d(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3,
                                 glm::dvec3& barycentric)
{
   	x1 -= x0; x2 -= x0; x3 -= x0;
   	y1 -= y0; y2 -= y0; y3 -= y0;
   	int signa = orientation(x2, y2, x3, y3, barycentric.x);
   	if(signa == 0) return false;
   	int signb = orientation(x3, y3, x1, y1, barycentric.y);
   	if(signb != signa) return false;
   	int signc = orientation(x1, y1, x2, y2, barycentric.z);
   	if(signc != signa) return false;
   	double s = 1.0 / (barycentric.x + barycentric.y + barycentric.z);
    barycentric *= s;
   	return true;
}

void make_level_set3(const std::vector<glm::uvec3>& faces, const std::vector<glm::dvec3>& positions,
                     const glm::dvec3& origin, double delta, const glm::ivec3& size,
                     array3d<double>& field, const int exact_band)
{
    field.resize(size.x, size.y, size.z);
    field.assign((size.x + size.y + size.z) * delta); // upper bound on distance
    array3d<int> closest_tri(size.x, size.y, size.z, -1);
    array3d<int> intersection_count(size.x, size.y, size.z, 0); 

    //===================================================================================================================================================================================================================
    // intersection_count(i,j,k) is # of tri intersections in (i-1,i] x {j} x {k}
   	// we begin by initializing distances near the mesh, and figuring out intersection counts
    //===================================================================================================================================================================================================================
    double inv_delta = 1.0 / delta;

    for(unsigned int t = 0; t < faces.size(); ++t)
    {
        unsigned int p = faces[t].x, q = faces[t].y, r = faces[t].z; 

        //===============================================================================================================================================================================================================
        // coordinates in grid to high precision
        //===============================================================================================================================================================================================================
        glm::dvec3 P = inv_delta * (glm::dvec3(positions[p]) - glm::dvec3(origin));
        glm::dvec3 Q = inv_delta * (glm::dvec3(positions[q]) - glm::dvec3(origin));
        glm::dvec3 R = inv_delta * (glm::dvec3(positions[r]) - glm::dvec3(origin));

        //===============================================================================================================================================================================================================
      	// do distances nearby
        //===============================================================================================================================================================================================================
        glm::dvec3 lower_bound = glm::min(glm::min(P, Q), R);
        glm::dvec3 upper_bound = glm::max(glm::max(P, Q), R);
        glm::ivec3 ilower_bound = glm::clamp(glm::ivec3(glm::floor(lower_bound)) - exact_band, glm::ivec3(0), size - 1);
        glm::ivec3 iupper_bound = glm::clamp(glm::ivec3(glm::ceil(upper_bound))  + exact_band, glm::ivec3(0), size - 1);

        for(int k = ilower_bound.z; k <= iupper_bound.z; ++k)
            for(int j = ilower_bound.y; j <= iupper_bound.y; ++j)
                for(int i = ilower_bound.x; i <= iupper_bound.x; ++i)
                {
                    glm::dvec3 g = origin + glm::dvec3(i, j, k) * delta;
                    double d = point_triangle_distance(g, positions[p], positions[q], positions[r]);
                    if(d < field(i, j, k))
                    {
                        field(i, j, k) = d;
                        closest_tri(i, j, k) = t;
                    }
                }

        //===============================================================================================================================================================================================================
        // and do intersection counts
        //===============================================================================================================================================================================================================
        for(int k = ilower_bound.z; k <= iupper_bound.z; ++k)
            for(int j = ilower_bound.y; j <= iupper_bound.y; ++j)
            {
                glm::dvec3 barycentric;
                if(point_in_triangle_2d(j, k, P.y, P.z, Q.y, Q.z, R.y, R.z, barycentric))
                {
                    double x = barycentric.x * P.x + barycentric.y * Q.x + barycentric.z * R.x;
                    int ix = int(glm::ceil(x));
                    if(ix < 0) 
                        ++intersection_count(0, j, k);
                    else if(ix < size.x) 
                        ++intersection_count(ix, j, k);
                }
            }
    }

    //===================================================================================================================================================================================================================
    // and now we fill in the rest of the distances with fast sweeping
    //===================================================================================================================================================================================================================
    for(unsigned int pass = 0; pass < 2; ++pass)
    {
        sweep(faces, positions, field, closest_tri, origin, delta, +1, +1, +1);
        sweep(faces, positions, field, closest_tri, origin, delta, -1, -1, -1);
        sweep(faces, positions, field, closest_tri, origin, delta, +1, +1, -1);
        sweep(faces, positions, field, closest_tri, origin, delta, -1, -1, +1);
        sweep(faces, positions, field, closest_tri, origin, delta, +1, -1, +1);
        sweep(faces, positions, field, closest_tri, origin, delta, -1, +1, -1);
        sweep(faces, positions, field, closest_tri, origin, delta, +1, -1, -1);
        sweep(faces, positions, field, closest_tri, origin, delta, -1, +1, +1);
    }

    //===================================================================================================================================================================================================================
    // figure out signs (inside/outside) from intersection counts
    //===================================================================================================================================================================================================================
    for(int k = 0; k < size.z; ++k)
        for(int j = 0; j < size.y; ++j)
        {
            int internal = 0;
            for(int i = 0; i < size.x; ++i)
            {
                internal += intersection_count(i, j, k);
                if (internal & 1)
                    field(i, j, k) = -field(i, j, k);
            }
        }
}

#endif
