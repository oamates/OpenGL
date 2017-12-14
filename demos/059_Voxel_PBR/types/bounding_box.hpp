#pragma once

#include <array>
#include <memory>
#include <glm/detail/type_vec3.hpp>
#include <glm/mat4x4.hpp>

// Describes a volume box which contains anything inside be it a node or a mesh.
struct BoundingBox
{
    std::array<glm::vec3, 2> boundaries;
    std::shared_ptr<BoundingBox> original;
    glm::vec3 center;
    glm::vec3 extent;

    BoundingBox();
    virtual ~BoundingBox();

    void MinPoint(const glm::vec3 &val);            // Sets the minimum point only if value is smaller than the current minPoint
    void MaxPoint(const glm::vec3 &val);            // Sets the maximum point only if value is bigger than the current maxPoint
    void Transform(const glm::mat4x4 &model);       // Updates the boundaries accordly with the given model matrix.
    void Reset();                                   // Resets the boundaries, to be updated again with MaxPoint and MinPoint

    const glm::vec3 &MinPoint(bool transformed = false) const;
    const glm::vec3 &MaxPoint(bool transformed = false) const;
    const glm::vec3 &Center(bool transformed = false) const;
    const glm::vec3 &Extent(bool transformed = false) const;
};
