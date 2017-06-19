#ifndef _cms_vertex_included_82437513491235412783561820735610875618274356723427
#define _cms_vertex_included_82437513491235412783561820735610875618274356723427

#include <glm/glm.hpp>

namespace cms
{

struct vertex_t
{
    glm::vec3 position;
    glm::vec3 normal;

    vertex_t()
        : position(glm::vec3(0.0f)), normal(glm::vec3(0.0f))
    {}

    vertex_t(const glm::vec3& position, const glm::vec3& normal)
        : position(position), normal(normal)
    {}
};

} // namespace cms

#endif // _cms_vertex_included_82437513491235412783561820735610875618274356723427