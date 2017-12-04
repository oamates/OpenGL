// File that allows to manage primitives in parallel with glsl
 
#ifndef PRIMITIVE
#define PRIMITIVE

#include <vector>
#include <iostream>

#include <glm/glm.hpp>

enum TypePrimitive
{
    PRIMITIVE_TRIANGLE = 0,
    PRIMITIVE_PLAN = 1,
    PRIMITIVE_QUADRIQUE = 2
};

struct vec2
{
    vec2()
    {
      u = 0.0;
      v = 0.0;
    }
    vec2(float parU, float parV)
    {
      u = parU;
      v = parV;
    }
	void operator = (const vec2& parVec2)
	{
		u = parVec2.u;
		v = parVec2.v;
	}
    float u;
    float v;
};


struct indexList
{
    int p0;
    int p1;
    int p2;
};

struct Triangle
{
    glm::dvec3 p0;
    vec2 uv0;
    glm::dvec3 p1;
    vec2 uv1;
    glm::dvec3 p2;
    vec2 uv2;
    glm::dvec3 normale;

};

std::ostream& operator << (std::ostream& stream, const Triangle& parTriangle);

struct Plan
{
    glm::dvec3 p0;
    glm::dvec3 p1;
    glm::dvec3 p2;
    glm::dvec3 p3;
    glm::dvec3 normale;
};

struct Quadrique
{
    float A;
    float B;
    float C;
    float D;
    float E;
    float F;
    float G;
    float H;
    float I;
    float J;
};

// Primitive
struct Primitive
{
    int type;
    int index;
    int materiau;
};

std::ostream& operator << (std::ostream& stream, const Primitive& parTriangle);

// Intersection
struct Intersection
{
    bool isValid;
    glm::dvec3 point;
    float distance;
    glm::dvec3 normal;
    Primitive obj;
};

//Materiau
struct Materiau
{
    glm::dvec4 color;
    float coeffReflexion;
    float coeffRefraction;
    float indiceRefraction;
    float diff;
    float spec;
    int texAlbedo;
    int texRough;
    int texSpec;
};

std::ostream& operator << (std::ostream& stream, const Materiau& parMat);

// Lumieres
struct Light
{
    glm::dvec3 position;
    glm::dvec3 color;
    float intensity;
};

std::ostream& operator << (std::ostream& stream, const Light& parLum);

#endif //PRIMITIVE
