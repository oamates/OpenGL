#pragma once
#include <array>

#include <glm/gtc/matrix_access.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec4.hpp>
#include <glm/detail/func_geometric.hpp>

#include "bbox.hpp"

struct BoundingSphere;

// A frustum formed by six planes
struct frustum_t
{
    enum FrustumFace { Left = 0, Right = 1, Bottom = 2, Top = 3, Near = 4, Far = 5 };
    std::array<glm::vec4, 6> planes;

    const glm::vec4& Plane(const FrustumFace face) const                        // Returns the plane from the give frustum face
        { return planes[face]; }

    const std::array<glm::vec4, 6>& Planes() const                              // Returns the frustum planes
        { return planes; }

    void ExtractPlanes(const glm::mat4x4& modelView, bool normalize = true)     // Extracts the frustum planes from the model view matrix
    {
        const glm::vec4 mRow[4] =                                               // extract frustum planes from the modelView matrix planes are in format: normal(xyz), offset(w)
        {
            glm::row(modelView, 0), glm::row(modelView, 1),
            glm::row(modelView, 2), glm::row(modelView, 3)
        };
    
        planes[Left]   = mRow[3] + mRow[0];
        planes[Right]  = mRow[3] - mRow[0];
        planes[Bottom] = mRow[3] + mRow[1];
        planes[Top]    = mRow[3] - mRow[1];
        planes[Near]   = mRow[3] + mRow[2];
        planes[Far]    = mRow[3] - mRow[2];
    
        if (normalize)
            for (glm::vec4& p : planes)                                             // normalize them
                p = glm::normalize(p);
    }

    bool InFrustum(const bbox_t& volume) const                                  // Determines if the given volume is inside the frustum
    {
        for (const glm::vec4& p : planes)
        {
            glm::vec3 n = glm::vec3(p);
            float d = glm::dot(volume.Extent(true), glm::abs(n));
            float r = p.w + glm::dot(volume.Center(true), n);
            if (d + r <= 0.0f)
                return false;
        }
        return true;
    }
};