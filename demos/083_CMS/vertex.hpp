#ifndef _cms_vertex_included_82437513491235412783561820735610875618274356723427
#define _cms_vertex_included_82437513491235412783561820735610875618274356723427

#include <vector>

#include "vec3.hpp"

namespace cms
{

struct vertex_t
{
    Vec3 position;
    Vec3 normal;

    vertex_t() : 
        position(Vec3(0, 0, 0)),
        normal(Vec3(0, 0, 0))
    {}

    vertex_t(Vec3 position, Vec3 normal) :
        position(position),
        normal(normal)
    {}
};

} // namespace cms

#endif // _cms_vertex_included_82437513491235412783561820735610875618274356723427