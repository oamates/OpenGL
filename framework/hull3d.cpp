#include <vector>
#include <algorithm>
#include <glm/glm.hpp>

#include "hull3d.hpp"

namespace hull3d {

//=======================================================================================================================================================================================================================
// clears the vector from unused triangular faces
//=======================================================================================================================================================================================================================
std::vector<triangle_t> convex_hull(std::vector<glm::dvec3>& points)
{
    //===================================================================================================================================================================================================================
    // to sort points in a lexicographic order, this structure has to be passed to std::sort call
    // () is supposed to mimic comparison < operator
    //===================================================================================================================================================================================================================
    struct 
    {
        bool operator()(const glm::dvec3& lhs, const glm::dvec3& rhs)
        {   
            if (lhs.z < rhs.z) return true;
            if (lhs.z > rhs.z) return false;
            if (lhs.y < rhs.y) return true;
            if (lhs.y > rhs.y) return false;
            return lhs.x < rhs.x;
        }   
    } dvec3_lex;
    std::sort(points.begin(), points.end(), dvec3_lex);

    //===================================================================================================================================================================================================================
    // the main call
    //===================================================================================================================================================================================================================
    std::vector<triangle_t> hull = hull3d(points);
    int hull_size = hull.size();

    //===================================================================================================================================================================================================================
    // re-index both vertex buffer and triangle_t buffer
    //===================================================================================================================================================================================================================
    int cloud_size = points.size();
    unsigned int* pindex = (unsigned int*) malloc (cloud_size * sizeof(unsigned int));
    unsigned int* tindex = (unsigned int*) malloc (hull_size * sizeof(unsigned int));
    for (int i = 0; i < cloud_size; ++i) pindex[i] = 0;

    int t = 0;
    for(int i = 0; i < hull_size; ++i)
        if(hull[i].state > 0)
        {
            tindex[i] = t++;
            pindex[hull[i].vertices.x] = 1;
            pindex[hull[i].vertices.y] = 1;
            pindex[hull[i].vertices.z] = 1;
        }
        else
            tindex[i] = -1;

    int p = 0;
    for(int i = 0; i < cloud_size; ++i)
        if (pindex[i]) pindex[i] = p++;

    std::vector<triangle_t> chull;
    for(int i = 0; i < hull_size; ++i)
        if(hull[i].state > 0)
        {
            triangle_t triangle;
            triangle.id = tindex[i];
            triangle.state = 1;
            triangle.vertices = glm::ivec3(pindex[hull[i].vertices.x], pindex[hull[i].vertices.y], pindex[hull[i].vertices.z]);
            triangle.edges = glm::ivec3(tindex[hull[i].edges.x], tindex[hull[i].edges.y], tindex[hull[i].edges.z]);
            triangle.normal = hull[i].normal;
            chull.push_back(triangle);
        }

    free(pindex);
    free(tindex);
    return chull;
}

inline void attach_edge(triangle_t& triangle, int p, int q, int id)
{
    if (triangle.vertices.x == p)
        triangle.edges.y = id;
    else if(triangle.vertices.y == p)
        triangle.edges.z = id;
    else // triangle.vertices.z == p
        triangle.edges.x = id;
}

//=======================================================================================================================================================================================================================
// the main working routine : produces a vector of triangular faces of the hull 
// many of them are marked as not used, but are not cleaned out from the vector
// assumes that the input vector is sorted lexicografically
//=======================================================================================================================================================================================================================
std::vector<triangle_t> hull3d(std::vector<glm::dvec3>& points)
{
    std::vector<triangle_t> hull;
    int hull_size = 0;
    int cloud_size = points.size();

    triangle_t triangle;
    triangle.state = USED;
    triangle.vertices = glm::ivec3(0, 1, 2);

    glm::dvec3 mass_center = points[0] + points[1] + points[2];
    glm::dvec3 n = glm::cross(points[1] - points[0], points[2] - points[1]);

    triangle.id = hull_size++;
    triangle.normal = n;
    triangle.edges = glm::ivec3(1);
    hull.push_back(triangle);

    triangle.vertices.y = 2;
    triangle.vertices.z = 1;
    triangle.id = hull_size++;
    triangle.normal = -n;
    triangle.edges = glm::ivec3(0);
    hull.push_back(triangle);

    for(int p = 3; p < cloud_size; ++p)
    {
        glm::dvec3 mass_center_direction = mass_center - double(p) * points[p];
        mass_center += points[p];

        int hsize = hull_size;
        std::vector<int> xlist;
        int xlist_size = 0;

        for(int h = hull_size - 1; h >= 0; --h)
            if(glm::dot(points[p] - points[hull[h].vertices.x], hull[h].normal) > 0.0)
            {
                hull[h].state = DISCARDED;
                xlist.push_back(h);
                xlist_size = 1;
                break;
            }

        for(int index = 0; index < xlist_size; index++)
        {
            int id = xlist[index];
            int bc = hull[id].edges.x;
            int ca = hull[id].edges.y;
            int ab = hull[id].edges.z;

            if (glm::dot(points[p] - points[hull[bc].vertices.x], hull[bc].normal) > 0.0)
            {
                if(hull[bc].state == USED)
                {
                    hull[bc].state = DISCARDED;
                    xlist.push_back(bc);
                    ++xlist_size;
                }
            }
            else
            {
                triangle.id = hull_size++;
                triangle.state = NEEDS_ADJUSTMENT;
                triangle.edges = glm::ivec3(bc, -1, -1);
                int y = hull[id].vertices.y;
                int z = hull[id].vertices.z;
                triangle.normal = glm::cross(points[y] - points[p], points[z] - points[p]);
                if (glm::dot(triangle.normal, mass_center_direction) > 0.0)
                {
                    triangle.normal = -triangle.normal;
                    int q = y; y = z; z = q;
                }
                triangle.vertices = glm::ivec3(p, y, z);
                attach_edge(hull[bc], y, z, triangle.id);
                hull.push_back(triangle);
            }

            if (glm::dot(points[p] - points[hull[ca].vertices.x], hull[ca].normal) > 0.0)
            {
                if(hull[ca].state == USED)
                {
                    hull[ca].state = DISCARDED;
                    xlist.push_back(ca);
                    ++xlist_size;
                }
            }
            else
            {
                triangle.id = hull_size++;
                triangle.state = NEEDS_ADJUSTMENT;
                triangle.edges = glm::ivec3(ca, -1, -1);
                int z = hull[id].vertices.z;
                int x = hull[id].vertices.x;
                triangle.normal = glm::cross(points[z] - points[p], points[x] - points[p]);
                if (glm::dot(triangle.normal, mass_center_direction) > 0.0)
                {
                    triangle.normal = -triangle.normal;
                    int q = z; z = x; x = q;
                }
                triangle.vertices = glm::ivec3(p, z, x);
                attach_edge(hull[ca], z, x, triangle.id);
                hull.push_back(triangle);
            }

            if(glm::dot(points[p]- points[hull[ab].vertices.x], hull[ab].normal) > 0.0)
            {
                if(hull[ab].state == USED)
                {
                    hull[ab].state = DISCARDED;
                    xlist.push_back(ab);
                    ++xlist_size;
                }
            }
            else
            {
                triangle.id = hull_size++;
                triangle.state = NEEDS_ADJUSTMENT;
                triangle.edges = glm::ivec3(ab, -1, -1);
                int x = hull[id].vertices.x;
                int y = hull[id].vertices.y;
                triangle.normal = glm::cross(points[x] - points[p], points[y] - points[p]);
                if (glm::dot(triangle.normal, mass_center_direction) > 0.0)
                {
                    triangle.normal = -triangle.normal;
                    int q = x; x = y; y = q;
                }
                triangle.vertices = glm::ivec3(p, x, y);
                attach_edge(hull[ab], x, y, triangle.id);
                hull.push_back(triangle);
            }
        }

        std::vector<Snork> norks;
        Snork snork;
        for(int q = hull_size - 1; q >= hsize; q--)
        {
            if(hull[q].state == NEEDS_ADJUSTMENT)
            {
                snork.id = q;
                snork.a = hull[q].vertices.y;
                snork.b = 1;
                norks.push_back(snork);
                snork.a = hull[q].vertices.z;
                snork.b = 0;
                norks.push_back(snork);
                hull[q].state = USED;
            }
            std::sort(norks.begin(), norks.end());
            for(int s = 0; s < (int) norks.size() - 1; s++)
                if(norks[s].a == norks[s + 1].a)
                {
                    if(norks[s].b == 1)
                        hull[norks[s].id].edges.z = norks[s + 1].id;
                    else
                        hull[norks[s].id].edges.y = norks[s + 1].id;

                    if(norks[s + 1].b == 1)
                        hull[norks[s + 1].id].edges.z = norks[s].id;
                    else
                        hull[norks[s + 1].id].edges.y = norks[s].id;
                }
        }
    }
    return hull;
}

}