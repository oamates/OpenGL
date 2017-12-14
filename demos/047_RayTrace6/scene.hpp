// Struct that manages the scene

#ifndef SCENE_COMMON
#define SCENE_COMMON

#include <vector>

#include "defines.hpp"
#include "primitive.hpp"

struct Scene
{
    Scene() {}
    ~Scene() {}
    void AddTriangle(const Triangle &triangle, const Materiau &materiau);
    void AddTriangle(const Triangle &triangle);
    void AddPlane(const Plan &plane, const Materiau &materiau);
    void AddQuadric(const Quadrique &quadric, const Materiau &materiau);
    void AddMateriau(const Materiau &materiau);
    void AddLight(const Light& light);

    std::vector<Plan> m_planes;
    std::vector<Triangle> m_triangles;
    std::vector<Quadrique> m_quadrics;
    std::vector<Materiau> m_materiaux;
    std::vector<Primitive> m_primitives;
    std::vector<Light> m_lights;
};

#endif //SCENE_COMMON