#pragma once

#include <glm/gtx/norm.hpp>
#include <glm/gtx/orthonormalize.hpp>
#include <glm/detail/type_vec3.hpp>

// Vertex parameters
struct Vertex
{
    glm::vec3 position;
    glm::vec3 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    Vertex()
        { position = normal = tangent = bitangent = uv = glm::vec3(0.0f); }

    void Orthonormalize()
    {
        normal = normalize(normal);
        this->tangent = orthonormalize(tangent, normal);                            // Gram-Schmidt orthonormalization
        if (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f)                // secure handedness
            tangent = -tangent;
    }
};