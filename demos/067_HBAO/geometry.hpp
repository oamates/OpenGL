#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include <cassert>
#include <string>

#include <glm/glm.hpp>
#include <GL/glew.h>

struct Geometry
{
    typedef struct
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec2 texCoord;
    }sVertex;

    Geometry();
    ~Geometry();

    uint32_t getVertexSize();
    uint32_t getTriangleSize();

    const glm::vec3& getVertexPosition(const uint32_t& vertexIndex) const;
    const glm::vec3& getVertexNormal(const uint32_t& vertexIndex) const;
    const glm::vec2& getVertexTextCoord(const uint32_t& vertexIndex) const;

    const glm::uvec3 &getTriangleIndices(const uint32_t &triangleIndex) const;
    const glm::vec3 &getTrianglePoint(const uint32_t &triangleIndex, const int32_t& point) const;

    const float* getVertexData();
    const uint32_t* getTriangleData();

    void addVertex(const sVertex& vertex);
    void addTriangle(const glm::uvec3& triangle);
    const Geometry& addGeometry(const Geometry& geometry);

    void translate(const glm::vec3 &translation);
    void rotate(const glm::vec3 &rotation);
    void scale(float scale);

    bool createStaticBuffers(int32_t posLoc = 0, int32_t normLoc = 1, int32_t tangLoc = 2, int32_t texLoc = 3);
    bool createDynamicBuffers();

    inline bool existOnGpu()
        { return (bool) glIsVertexArray(vao); }
    bool updateBuffers();

    void destroyBuffers();

    void process();
    void clear();

    void draw();

    void bindVAO();
    void unbindVAO();

    std::string material;

    uint32_t vao;
    uint32_t vbo_vertex;
    uint32_t vbo_triangle;

    std::vector<sVertex> vertices;
    std::vector<glm::uvec3> triangles;
};
    
#endif
