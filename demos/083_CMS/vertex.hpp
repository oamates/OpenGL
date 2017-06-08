#ifndef _cms_vertex_included_82437513491235412783561820735610875618274356723427
#define _cms_vertex_included_82437513491235412783561820735610875618274356723427

#include <vector>

#include "vec3.hpp"

namespace cms
{

struct Vertex
{
    std::vector<int> onPath;
    Vec3 m_pos;
    Vec3 m_normal;

    Vertex() : 
        m_pos(Vec3(0, 0, 0)),
        m_normal(Vec3(0, 0, 0))
    {}

    Vertex(Vec3 i_pos, Vec3 i_normal) :
        m_pos(i_pos),
        m_normal(i_normal)
    {}

    void setPos(Vec3 i_pos)
        { m_pos = i_pos; }

    const Vec3& getPos() const
        { return m_pos; }

    void setNormal(Vec3 i_normal)
        { m_normal = i_normal; }

    const Vec3& getNormal() const
        { return m_normal; }

    void print() const
    {
        std::cout << "Point: " << m_pos << std::endl << "Normal: " << m_normal << std::endl;
    }

};

} // namespace cms

#endif // _cms_vertex_included_82437513491235412783561820735610875618274356723427