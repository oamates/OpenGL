#pragma once
#include <array>

#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec4.hpp>

struct BoundingBox;
struct BoundingSphere;
struct BoundingBox;

// A frustum formed by six planes
struct Frustum
{
    enum FrustumPlane
    {
        Left, Right,
        Bottom, Top,
        Near, Far,
    };

    std::array<glm::vec4, 6> planes;

    void ExtractPlanes(const glm::mat4x4 &modelView, bool normalize = true);    // Extracts the frustum planes from the model view matrix
    const glm::vec4 &Plane(const FrustumPlane face) const;                      // Returns the plane from the give frustum face
    const std::array<glm::vec4, 6> &Planes() const;                             // Returns the frustum planes
};

// Contains methods for frustum culling with its Frustum parameters.
struct CullingFrustum : public Frustum
{
    CullingFrustum();
    virtual ~CullingFrustum();

    bool InFrustum(const BoundingBox &volume) const;                            // Determines if the given volume is inside the frustum
};
