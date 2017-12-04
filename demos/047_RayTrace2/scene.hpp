#ifndef SCENE_H
#define SCENE_H

#include <cstdint>
#include <cstdlib>

#include "raytracerconfig.hpp"
#include "vqs.hpp"

typedef Dg::VQS<real> vqs;
typedef Dg::Vector4<real> vec4;

struct IntersectionData
{

};

struct Materials
{
    uint32_t color;
};

struct Sphere
{
    vec4 center;
    real radius;
    Materials materials;
};

struct OBB
{
    vec4 center;
    real x, y, z;
    Materials materials;
};

struct Torus
{
    vec4 center;
    vec4 axis;
    real radius_circle;
    real radius_thick;
    Materials materials;
};

struct Mesh
{
    vec4* vertices;
    int nVertices;
    int* facets[3];
    Materials materials;
};

struct Ray
{
    vec4 origin;
    vec4 direction;
};

template<typename T> struct qArray
{
    unsigned int size;
    T* data;

    qArray() : size(0), data(0) {}

    ~qArray()
        { free(data); }

    void Resize(unsigned a_size) 
    {
        size = a_size;
        if (!a_size)
        {
            free(data);
            data = 0;
            return;
        }
        data = realloc(data, size * sizeof(T));
    }
};

struct Scene
{
    qArray<Sphere> m_spheres;
    vqs m_camera;
};

#endif