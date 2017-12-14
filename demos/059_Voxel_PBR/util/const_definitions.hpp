#pragma once

#include <glm/detail/type_vec3.hpp>
#include <glm/matrix.hpp>

struct Vector3
{
    static const glm::vec3 forward;         // (0,0,1)
    static const glm::vec3 right;           // (1,0,0)
    static const glm::vec3 up;              // (0,1,0)
    static const glm::vec3 zero;            // (0,0,0)
    static const glm::vec3 one;             // (1,1,1)
};

struct Matrix
{
    static const glm::mat4x4 identity4;
    static const glm::mat3x3 identity3;
    static const glm::mat2x2 identity2;
};