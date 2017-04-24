#ifndef _hull3d_included_253892378467825423854623854288923673426737456342753454
#define _hull3d_included_253892378467825423854623854288923673426737456342753454

#include <vector>
#include <glm/glm.hpp>

enum {DISCARDED, USED, NEEDS_ADJUSTMENT};

struct Triangle
{
    int id, state;
    glm::ivec3 vertices;
    glm::ivec3 edges;
    glm::dvec3 normal;
};

struct R3
{
    int id;
    glm::dvec3 position;
};

inline bool operator < (const R3& lhs, const R3& rhs) 
{
    if (lhs.position.z < rhs.position.z) return true;
    if (lhs.position.z > rhs.position.z) return false;
    if (lhs.position.y < rhs.position.y) return true;
    if (lhs.position.y > rhs.position.y) return false;
    return lhs.position.x < rhs.position.x;
};

struct Snork
{
    int id;
    int a, b;
};

inline bool operator < (const Snork& lhs, const Snork& rhs) 
{
    if (lhs.a < rhs.a) return true; 
    if (lhs.a > rhs.a) return false; 
    return lhs.b < rhs.b;
};

std::vector<Triangle> convex_hull(std::vector<R3>& points);
std::vector<Triangle> hull3d (std::vector<R3>& points);

#endif // _hull3d_included_253892378467825423854623854288923673426737456342753454
