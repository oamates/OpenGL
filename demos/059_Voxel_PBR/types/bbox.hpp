#pragma once

#include <array>
#include <memory>

#include <glm/detail/func_common.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/mat4x4.hpp>

// Describes a volume box which contains anything inside be it a node or a mesh.
struct bbox_t
{
    std::array<glm::vec3, 2> boundaries;

    std::shared_ptr<bbox_t> original;
    glm::vec3 center;
    glm::vec3 extent;

    bbox_t() : original(nullptr)
        { Reset(); }
    virtual ~bbox_t() {}

    void MinPoint(const glm::vec3& val)             // Sets the minimum point only if value is smaller than the current minPoint
    {
        boundaries[0] = glm::min(boundaries[0], val);
        center = 0.5f * (boundaries[1] + boundaries[0]);
        extent = 0.5f * (boundaries[1] - boundaries[0]);
    }

    void MaxPoint(const glm::vec3 &val)             // Sets the maximum point only if value is bigger than the current maxPoint
    {
        boundaries[1] = glm::max(boundaries[1], val);
        center = 0.5f * (boundaries[1] + boundaries[0]);
        extent = 0.5f * (boundaries[1] - boundaries[0]);
    }

    void Transform(const glm::mat4x4& model)        // Updates the boundaries accordly with the given model matrix.
    {
        if (!original)
            original = std::make_shared<bbox_t>(*this);

        glm::vec3 min = original->boundaries[0];
        glm::vec3 max = original->boundaries[1];
        glm::vec3 corners[8] =
        {
            min,
            max,
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(min.x, max.y, min.z),
            glm::vec3(max.x, min.y, min.z),
            glm::vec3(min.x, max.y, max.z),
            glm::vec3(max.x, min.y, max.z),
            glm::vec3(max.x, max.y, min.z),
        };

        for (int i = 0; i < 8; i++)
        {
            glm::vec3 transformed = glm::vec3(model * glm::vec4(corners[i], 1.0f));
            boundaries[0] = glm::min(boundaries[0], transformed);
            boundaries[1] = glm::max(boundaries[1], transformed);
        }

        center = 0.5f * (boundaries[1] + boundaries[0]);
        extent = 0.5f * (boundaries[1] - boundaries[0]);
    }

    void Reset()                                    // Resets the boundaries, to be updated again with MaxPoint and MinPoint
    {
        boundaries[1] = glm::vec3(std::numeric_limits<float>::lowest());
        boundaries[0] = glm::vec3(std::numeric_limits<float>::infinity());
        if (original)
            original.reset();
    }

    const glm::vec3& MinPoint(bool transformed = false) const
    {
        if(transformed)
            return boundaries[0];
        return !original ? boundaries[0] : original->boundaries[0];
    }

    const glm::vec3& MaxPoint(bool transformed = false) const
    {
        if (transformed)
            return boundaries[1];
        return !original ? boundaries[1] : original->boundaries[1];
    }

    const glm::vec3& Center(bool transformed = false) const
    {
        if (transformed)
            return center;
        return !original ? center : original->center;
    }

    const glm::vec3& Extent(bool transformed = false) const
    {
        if (transformed)
            return extent;
        return !original ? extent : original->extent;
    }
};