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

struct Snork
{
    int id;
    int a, b;
};


inline bool operator < (const glm::dvec3& lhs, const glm::dvec3& rhs) 
{
	if (lhs.z < rhs.z) return true;
    if (lhs.z > rhs.z) return false;
	if (lhs.y < rhs.y) return true;
    if (lhs.y > rhs.y) return false;
	return lhs.x < rhs.x;
};

inline bool operator < (const Snork& lhs, const Snork& rhs) 
{
    if (lhs.a < rhs.a) return true; 
    if (lhs.a > rhs.a) return false; 
    return lhs.b < rhs.b;
};

std::vector<Triangle> convex_hull(std::vector<glm::dvec3>& points);
std::vector<Triangle> hull3d(std::vector<glm::dvec3>& points);

#endif // _hull3d_included_253892378467825423854623854288923673426737456342753454
