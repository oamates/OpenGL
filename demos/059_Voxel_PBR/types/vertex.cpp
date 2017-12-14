#include <glm/gtx/norm.hpp>
#include <glm/gtx/orthonormalize.hpp>

#include "vertex.hpp"

Vertex::Vertex()
{
    position = normal = tangent = bitangent = uv = glm::vec3(0.0f);
}

void Vertex::Orthonormalize()
{
    normal = normalize(normal);
    this->tangent = orthonormalize(tangent, normal);                    // Gram-Schmidt orthonormalization
    if (dot(cross(normal, tangent), bitangent) < 0.0f)                  // secure handedness
        tangent = -tangent;
}

