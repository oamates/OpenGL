//========================================================================================================================================================================================================================
// DEMO 083 : CMS algorithm
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <iostream>
#include <vector>
#include <math.h>

#include "isosurface_t.hpp"
#include "algcms.hpp"
#include "vec3.hpp"
#include "mesh.hpp"

struct ExampleClass
{
    // Sphere function
    float sphereFunction(float x, float y, float z) const
    {
        return x * x + y * y + z * z - 1.0f;
    }

    // Cube function
    float cubeFunction(float x, float y, float z) const
    {
        return std::max(fabs(x) - 1.0f, std::max((fabs(y) - 1.0f), fabs(z) - 1.0f));
    }

    // Cone function
    float coneFunction(float x, float y, float z) const
    {
        return 10.0f * (x * x + y * y) - (z - 1.0f) * (z - 1.0f);
    }

    // Anti-tank-like function
    float antiTankFunction(float x, float y, float z) const
    {
        return x * x * y * y + x * x * z * z + y * y * z * z - 0.01f;
    }

    // Heart function
    float heartFunction(float x, float y, float z) const
    {
        return pow(x * x + y * y + 2.0f * z * z - 0.5f, 3.0) - y * y * y * (x * x - 0.01f * z * z);
    }

    // Torus function
    float torusFunction(float x, float y, float z) const
    {
        float R = 0.45f;
        float r = 0.2f;
        float x0 = x - 0.25f;
        float q = x0 * x0 + y * y + z * z + R * R - r * r;

        return q * q - 4.0f * R * R * (z * z + x0 * x0);
    }

    // Double torus function
    float doubleTorusFunction(float x, float y, float z) const
    {
        float x2 = x * x;
        float x3 = x2 * x;
        float x4 = x2 * x2;
        float y2 = y * y;
        return -(0.01f - x4 + 2.0f * x3 * x3 - x4 * x4 + 2.0f * x2 * y2 - 2.0f * x4 * y2 - y2 * y2 - z * z);
    }

    // Interlinked torii function
    float linkedToriiFunction(float x, float y, float z) const
    {
        float R = 0.45f;
        float r = 0.2f;
        float x0 = x - 0.25f;
        float x1 = x + 0.25f;
        float q0 = x0 * x0 + y * y + z * z + R * R - r * r;
        float q1 = x1 * x1 + y * y + z * z + R * R - r * r;
        return (q0 * q0 - 4.0f * R * R * (x0 * x0 + z * z)) * (q1 * q1 - 4.0f * R * R * (x1 * x1 + y * y));
    }

    // ============================================================================================================================================================================================================================
    float operator()(float x, float y, float z) const
    {
        return torusFunction(x, y, z);
    }

};


static float BBOX_SIZE                  = 2.0f;
static int MIN_OCTREE_RES               = 2;
static int MAX_OCTREE_RES               = 8;
static float COMPLEX_SURFACE_THRESHOLD  = 0.85f;
static const char * OBJ_NAME            = "test.obj";

int ADDRESS_SIZE = MAX_OCTREE_RES; // To be used by some of the classes

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    ExampleClass t;

    cms::Isosurface_t<ExampleClass> iso(&t);

    float halfSize = 0.5f * BBOX_SIZE;

    cms::Range container[3] = 
    {
        cms::Range(-halfSize, halfSize),
        cms::Range(-halfSize, halfSize),
        cms::Range(-halfSize, halfSize)
    };

    cms::AlgCMS cmsAlg(&iso, container, MIN_OCTREE_RES, MAX_OCTREE_RES);

    cms::Mesh mesh;

    // Set the complex surface threshold
    cmsAlg.setComplexSurfThresh(COMPLEX_SURFACE_THRESHOLD);

    // Proceed to extract the surface <runs the algorithm>
    cmsAlg.extractSurface(mesh);

    // Export the created mesh as an .OBJ file
    mesh.exportOBJ(OBJ_NAME);

    return 0;
}
