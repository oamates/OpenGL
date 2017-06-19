#ifndef _cms_mesh_included_1872356784325143058634561237512030785203751238546234
#define _cms_mesh_included_1872356784325143058634561237512030785203751238546234

#include <vector>
#include <glm/glm.hpp>

namespace cms
{

struct mesh_t
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;  
    mesh_t() {};
};


} // namespace cms

#endif // _cms_mesh_included_1872356784325143058634561237512030785203751238546234