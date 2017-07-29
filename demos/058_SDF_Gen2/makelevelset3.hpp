#ifndef MAKELEVELSET3_H
#define MAKELEVELSET3_H

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "array3d.hpp"

// tri is a list of triangles in the mesh, and x is the positions of the vertices
// absolute distances will be nearly correct for triangle soup, but a closed mesh is
// needed for accurate signs. Distances for all grid cells within exact_band cells of
// a triangle should be exact; further away a distance is calculated but it might not
// be to the closest triangle - just one nearby.
void make_level_set3(const std::vector<glm::uvec3>& tri, const std::vector<glm::vec3>& x,
                     const glm::vec3& origin, float dx, int nx, int ny, int nz,
                     Array3f &phi, const int exact_band=1);


// find distance x0 is from segment x1-x2
static float point_segment_distance(const glm::vec3& x0, const glm::vec3& x1, const glm::vec3& x2)
{
    glm::vec3 dx = x2 - x1;
    double m2 = glm::length2(dx);
    // find parameter value of closest point on segment
    float s12 = glm::dot(x2 - x0, dx) / m2;
    if(s12 < 0)
    {
        s12 = 0;
    }
    else if (s12 > 1)
    {
        s12 = 1;
    }
   	// and find the distance
    return glm::length(x0 - (s12 * x1 + (1 - s12) * x2));
}

// find distance x0 is from triangle x1-x2-x3
static float point_triangle_distance(const glm::vec3& x0, const glm::vec3& x1, const glm::vec3& x2, const glm::vec3& x3)
{
    // first find barycentric coordinates of closest point on infinite plane
    glm::vec3 x13 = x1 - x3, 
    	      x23 = x2 - x3, 
    	      x03 = x0 - x3;

    float m13 = glm::length2(x13), 
          m23 = glm::length2(x23), 
          d = glm::dot(x13, x23);

    float invdet = 1.0f / glm::max(m13 * m23 - d * d, 1e-30f);
    float a = glm::dot(x13, x03),
          b = glm::dot(x23, x03);

    // the barycentric coordinates themselves
    float w23 = invdet * (m23 * a - d * b);
    float w31 = invdet * (m13 * b - d * a);
    float w12 = 1 - w23 - w31;
    if (w23 >= 0 && w31 >= 0 && w12 >= 0)
    {
        // if we're inside the triangle
        return glm::length(x0 - (w23 * x1 + w31 * x2 + w12 * x3)); 
    }
    else
    { // we have to clamp to one of the edges
        if(w23 > 0) // this rules out edge 2-3 for us
            return glm::min(point_segment_distance(x0, x1, x2), point_segment_distance(x0, x1, x3));
        else if(w31 > 0) // this rules out edge 1-3
            return glm::min(point_segment_distance(x0, x1, x2), point_segment_distance(x0, x2, x3));
        else // w12 must be >0, ruling out edge 1-2
            return glm::min(point_segment_distance(x0, x1, x3), point_segment_distance(x0, x2, x3));
    }
}

static void check_neighbour(const std::vector<glm::uvec3>& tri, const std::vector<glm::vec3>& x,
                            Array3f& phi, Array3i& closest_tri,
                            const glm::vec3& gx, int i0, int j0, int k0, int i1, int j1, int k1)
{
    if(closest_tri(i1, j1, k1) >= 0)
    {
        unsigned int p, q, r;
//        assign(tri[closest_tri(i1, j1, k1)], p, q, r);
        p = tri[closest_tri(i1, j1, k1)].x;
        q = tri[closest_tri(i1, j1, k1)].y;
        r = tri[closest_tri(i1, j1, k1)].z;
        float d = point_triangle_distance(gx, x[p], x[q], x[r]);
        if(d < phi(i0, j0, k0))
        {
            phi(i0, j0, k0) = d;
            closest_tri(i0, j0, k0) = closest_tri(i1, j1, k1);
        }
    }
}

/*
template<unsigned int N, class T>
inline void assign(const Vec<N,T> &a, T &a0, T &a1, T &a2)
{ 
   assert(N==3);
   a0=a.v[0]; a1=a.v[1]; a2=a.v[2];
}
*/

static void sweep(const std::vector<glm::uvec3> &tri, const std::vector<glm::vec3> &x,
                  Array3f& phi, Array3i& closest_tri, const glm::vec3& origin, float dx,
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
                glm::vec3 gx = glm::vec3(i * dx + origin[0], j * dx + origin[1], k * dx + origin[2]);
                check_neighbour(tri, x, phi, closest_tri, gx, i, j, k, i - di, j,      k);
                check_neighbour(tri, x, phi, closest_tri, gx, i, j, k, i,      j - dj, k);
                check_neighbour(tri, x, phi, closest_tri, gx, i, j, k, i - di, j - dj, k);
                check_neighbour(tri, x, phi, closest_tri, gx, i, j, k, i,      j,      k - dk);
                check_neighbour(tri, x, phi, closest_tri, gx, i, j, k, i - di, j,      k - dk);
                check_neighbour(tri, x, phi, closest_tri, gx, i, j, k, i,      j - dj, k - dk);
                check_neighbour(tri, x, phi, closest_tri, gx, i, j, k, i - di, j - dj, k - dk);
            }
}

// calculate twice signed area of triangle (0,0)-(x1,y1)-(x2,y2)
// return an SOS-determined sign (-1, +1, or 0 only if it's a truly degenerate triangle)
static int orientation(double x1, double y1, double x2, double y2, double& signed_area)
{
    signed_area = y1 * x2 - x1 * y2;

    if(signed_area > 0) return 1;
    if(signed_area < 0) return -1;
    if(y2 > y1) return 1;
    if(y2 < y1) return -1;
    if(x1 > x2) return 1;
    if(x1 < x2) return -1;
    
    return 0; // only true when x1==x2 and y1==y2
}

// robust test of (x0,y0) in the triangle (x1,y1)-(x2,y2)-(x3,y3)
// if true is returned, the barycentric coordinates are set in a,b,c.
static bool point_in_triangle_2d(double x0, double y0, 
                                 double x1, double y1, double x2, double y2, double x3, double y3,
                                 double& a, double& b, double& c)
{
   	x1 -= x0; x2 -= x0; x3 -= x0;
   	y1 -= y0; y2 -= y0; y3 -= y0;
   	int signa = orientation(x2, y2, x3, y3, a);
   	if(signa == 0) return false;
   	int signb = orientation(x3, y3, x1, y1, b);
   	if(signb != signa) return false;
   	int signc = orientation(x1, y1, x2, y2, c);
   	if(signc != signa) return false;
   	double sum = a + b + c;
   	assert(sum != 0); // if the SOS signs match and are nonkero, there's no way all of a, b, and c are zero.
   	a /= sum;
   	b /= sum;
   	c /= sum;
   	return true;
}

void make_level_set3(const std::vector<glm::uvec3>& tri, const std::vector<glm::vec3>& x,
                     const glm::vec3& origin, float dx, int ni, int nj, int nk,
                     Array3f& phi, const int exact_band)
{
    phi.resize(ni, nj, nk);
    phi.assign((ni + nj + nk)*dx); // upper bound on distance
    Array3i closest_tri(ni, nj, nk, -1);
    Array3i intersection_count(ni, nj, nk, 0); 
    // intersection_count(i,j,k) is # of tri intersections in (i-1,i]x{j}x{k}
   	// we begin by initializing distances near the mesh, and figuring out intersection counts
    glm::vec3 ijkmin, ijkmax;
    for(unsigned int t = 0; t < tri.size(); ++t)
    {
        unsigned int p = tri[t].x, q = tri[t].z, r = tri[t].z; 

        // coordinates in grid to high precision
        double fip = ((double)x[p][0] - origin[0]) / dx, fjp = ((double)x[p][1] - origin[1]) / dx, fkp = ((double)x[p][2] - origin[2]) / dx;
        double fiq = ((double)x[q][0] - origin[0]) / dx, fjq = ((double)x[q][1] - origin[1]) / dx, fkq = ((double)x[q][2] - origin[2]) / dx;
        double fir = ((double)x[r][0] - origin[0]) / dx, fjr = ((double)x[r][1] - origin[1]) / dx, fkr = ((double)x[r][2] - origin[2]) / dx;

      	// do distances nearby
        int i0 = glm::clamp(int(glm::min(glm::min(fip, fiq), fir)) - exact_band, 0, ni - 1), 
            i1 = glm::clamp(int(glm::max(glm::max(fip, fiq), fir)) + exact_band + 1, 0, ni - 1);
        int j0 = glm::clamp(int(glm::min(glm::min(fjp, fjq), fjr)) - exact_band, 0, nj - 1),
            j1 = glm::clamp(int(glm::max(glm::max(fjp, fjq), fjr)) + exact_band + 1, 0, nj - 1);
        int k0 = glm::clamp(int(glm::min(glm::min(fkp, fkq), fkr)) - exact_band, 0, nk - 1),
            k1 = glm::clamp(int(glm::max(glm::max(fkp, fkq), fkr)) + exact_band + 1, 0, nk - 1);
        for(int k = k0; k <= k1; ++k)
            for(int j = j0; j <= j1; ++j)
                for(int i = i0; i <= i1; ++i)
                {
                    glm::vec3 gx = glm::vec3(i * dx + origin[0], j * dx + origin[1], k * dx + origin[2]);
                    float d = point_triangle_distance(gx, x[p], x[q], x[r]);
                    if(d < phi(i, j, k))
                    {
                        phi(i, j, k) = d;
                        closest_tri(i, j, k) = t;
                    }
                }
        // and do intersection counts
        j0 = glm::clamp((int)std::ceil (glm::min(glm::min(fjp, fjq), fjr)), 0, nj - 1);
        j1 = glm::clamp((int)std::floor(glm::max(glm::max(fjp, fjq), fjr)), 0, nj - 1);
        k0 = glm::clamp((int)std::ceil (glm::min(glm::min(fkp, fkq), fkr)), 0, nk - 1);
        k1 = glm::clamp((int)std::floor(glm::max(glm::max(fkp, fkq), fkr)), 0, nk - 1);
        for(int k = k0; k <= k1; ++k)
            for(int j = j0; j <= j1; ++j)
            {
                double a, b, c;
                if(point_in_triangle_2d(j, k, fjp, fkp, fjq, fkq, fjr, fkr, a, b, c))
                {
                    double fi = a * fip + b * fiq + c * fir; // intersection i coordinate
                    int i_interval = int(std::ceil(fi)); // intersection is in (i_interval-1,i_interval]
                    if(i_interval<0) 
                        ++intersection_count(0, j, k); // we enlarge the first interval to include everything to the -x direction
                    else if(i_interval<ni) 
                        ++intersection_count(i_interval, j, k);
                    // we ignore intersections that are beyond the +x side of the grid
                }
            }
    }

    // and now we fill in the rest of the distances with fast sweeping
    for(unsigned int pass = 0; pass < 2; ++pass)
    {
        sweep(tri, x, phi, closest_tri, origin, dx, +1, +1, +1);
        sweep(tri, x, phi, closest_tri, origin, dx, -1, -1, -1);
        sweep(tri, x, phi, closest_tri, origin, dx, +1, +1, -1);
        sweep(tri, x, phi, closest_tri, origin, dx, -1, -1, +1);
        sweep(tri, x, phi, closest_tri, origin, dx, +1, -1, +1);
        sweep(tri, x, phi, closest_tri, origin, dx, -1, +1, -1);
        sweep(tri, x, phi, closest_tri, origin, dx, +1, -1, -1);
        sweep(tri, x, phi, closest_tri, origin, dx, -1, +1, +1);
    }

    // then figure out signs (inside/outside) from intersection counts
    for(int k = 0; k < nk; ++k)
        for(int j = 0; j < nj; ++j)
        {
            int total_count = 0;
            for(int i = 0; i < ni; ++i)
            {
                total_count += intersection_count(i, j, k);
                if (total_count % 2 == 1)
                {                                   // if parity of intersections so far is odd,
                    phi(i, j, k) = -phi(i, j, k);    // we are inside the mesh
                }
            }
        }
}

#endif
