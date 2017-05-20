
#include "geometry.hpp"
#include "log.hpp"

Geometry::Geometry()
{
    vao = 0;
    vbo_vertex = 0;
    vbo_triangle = 0;
}

Geometry::~Geometry()
{
    destroyBuffers();
}

GLuint Geometry::getVertexSize() { return vertices.size(); }
GLuint Geometry::getTriangleSize() { return triangles.size(); }

const glm::vec3 &Geometry::getVertexPosition(const GLuint &vertexIndex) const
{
    assert(vertexIndex < vertices.size());
    return vertices[vertexIndex].position;
}

const glm::vec3 &Geometry::getVertexNormal(const GLuint &vertexIndex) const
{
    assert(vertexIndex < vertices.size());
    return vertices[vertexIndex].normal;
}

const glm::vec2 &Geometry::getVertexTextCoord(const GLuint &vertexIndex) const
{
    assert(vertexIndex < vertices.size());
    return vertices[vertexIndex].texCoord;
}

const glm::uvec3 &Geometry::getTriangleIndices(const GLuint &triangleIndex) const
{
    assert(triangleIndex < triangles.size());
    return triangles[triangleIndex];
}

const glm::vec3 &Geometry::getTrianglePoint(const GLuint &triangleIndex, const GLint &point) const
{
    assert(triangleIndex < triangles.size());
    assert(0 < point && point < 3);
    return vertices[ triangles[triangleIndex][point] ].position;
}

const float* Geometry::getVertexData()
{
    if(vertices.size() > 0)
        return &vertices[0].position[0];

    return 0;
}

const GLuint* Geometry::getTriangleData()
{
    if(triangles.size() > 0)
        return &triangles[0][0];

    return 0;
}

void Geometry::addVertex(const sVertex &vertex)
{
    vertices.push_back(vertex);
}

void Geometry::addTriangle(const glm::uvec3 &triangle)
{
    triangles.push_back(triangle);
}

const Geometry& Geometry::addGeometry(const Geometry &geom)
{
    GLuint vertexOffset = vertices.size();
    glm::uvec3 tri;

    for(GLuint i = 0; i < geom.vertices.size(); ++i)
        vertices.push_back(geom.vertices[i]);

    for(GLuint i = 0; i < geom.triangles.size(); ++i)
    {
        tri = geom.triangles[i];
        tri[0] += vertexOffset;
        tri[1] += vertexOffset;
        tri[2] += vertexOffset;
        triangles.push_back(tri);
    }

    return *this;
}

void Geometry::translate(const glm::vec3 &translation)
{
    for(unsigned int i = 0; i < vertices.size(); i++)
        vertices[i].position += translation;
}

void Geometry::rotate(const glm::vec3 &rotation)
{
}

void Geometry::scale(float scale)
{
}

void Geometry::clear()
{
    vertices.clear();
    triangles.clear();
}

glm::vec3 generateTangent(glm::vec3 v1, glm::vec3 v2, glm::vec2 st1, glm::vec2 st2)
{    
    float coef = 1.0f / (st1.x * st2.y - st2.x * st1.y);
    glm::vec3 tangent;

    tangent.x = coef * ((v1.x * st2.y)  + (v2.x * -st1.y));
    tangent.y = coef * ((v1.y * st2.y)  + (v2.y * -st1.y));
    tangent.z = coef * ((v1.z * st2.y)  + (v2.z * -st1.y));
    
    return tangent;
}

void Geometry::process()
{
    glm::vec3 a,b,n,t;
    glm::vec2 sta, stb;

    std::vector<glm::vec3> tempNormal, tempTangent;
    tempNormal.resize(vertices.size(), glm::vec3(0.0f));
    tempTangent.resize(vertices.size(), glm::vec3(0.0f));

    assert(vertices.size() > 0);
    assert(triangles.size() > 0);

    std::vector<GLint> sharedFaces;
    sharedFaces.resize(vertices.size(), 0);

    debug_msg("vertices.size() = %d \n", (int) vertices.size());
    debug_msg("triangle.size() = %d \n", (int) triangles.size());

    glm::vec3 center = glm::vec3(0.0);

    // Find geometric center
    for (size_t i = 0; i < vertices.size(); ++i)
        center += vertices[i].position;

    center /= (float) vertices.size();

    for (size_t i = 0; i < vertices.size(); ++i)
        vertices[i].position -= center;

    for (size_t i=0; i < triangles.size(); ++i)
    {
        assert(i < triangles.size());

        a = vertices[triangles[i][1]].position - vertices[triangles[i][0]].position;
        b = vertices[triangles[i][2]].position - vertices[triangles[i][0]].position;

        n = glm::normalize(glm::cross(a,b));

        sta = vertices[triangles[i][1]].texCoord - vertices[triangles[i][0]].texCoord;
        stb = vertices[triangles[i][2]].texCoord - vertices[triangles[i][0]].texCoord;

        t = generateTangent(a, b, sta, stb);

        for(uint32_t u = 0; u < 3; ++u)
        {
            tempNormal[triangles[i][u]] += n;
            tempTangent[triangles[i][u]] += t;
            sharedFaces[triangles[i][u]]++;
        }
    }
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        if(sharedFaces[i] > 0)
        {
            tempNormal[i] /= (float) sharedFaces[i];
            tempNormal[i] = glm::normalize(tempNormal[i]);

            tempTangent[i] /= (float) sharedFaces[i];
            tempTangent[i] = glm::normalize(tempTangent[i]);
        }
        if(glm::dot(vertices[i].normal, vertices[i].normal) == 0.0f)
        {
            vertices[i].normal = tempNormal[i];
        }

        const glm::vec3& t = tempTangent[i];
        const glm::vec3& n = tempNormal[i];

        vertices[i].tangent = glm::normalize(t - n * glm::dot(n, t));
    }
    debug_msg("Done processing");
}

bool Geometry::createStaticBuffers(GLint posLoc, GLint normLoc, GLint tangLoc, GLint texLoc)
{
    destroyBuffers();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo_vertex);
    glGenBuffers(1, &vbo_triangle);

    // bind buffer for vertices and copy data into buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(sVertex), &vertices[0].position[0], GL_STATIC_DRAW);

    // Enable specific pointer for Vertex, for compability-mode and attributepointer for shader
    //glEnableClientState(GL_VERTEX_ARRAY);
    //glVertexPointer(3, GL_FLOAT, sizeof(sVertex), (char*)NULL);

    if(posLoc > -1)
    {
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(sVertex), 0);
        glEnableVertexAttribArray(posLoc);
    }
    else
        debug_msg("posLoc in createStaticBuffer was undefined");

    if(normLoc > -1)
    {
        glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(sVertex), (char*) NULL + 3 * sizeof(float));
        glEnableVertexAttribArray(normLoc);
    }
    else
        debug_msg("normLoc in createStaticBuffer was undefined");

    if(tangLoc > -1)
    {
        glVertexAttribPointer(tangLoc, 3, GL_FLOAT, GL_FALSE, sizeof(sVertex), (char*)NULL + 6 * sizeof(float));
        glEnableVertexAttribArray(tangLoc);
    }
    else
        debug_msg("tangLoc in createStaticBuffer was undefined");

    if(texLoc > -1)
    {
        glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(sVertex), (char*)NULL + 9 * sizeof(float));
        glEnableVertexAttribArray(texLoc);
    }
    else
        debug_msg("texLoc in createStaticBuffer was undefined");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_triangle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(glm::uvec3), &triangles[0][0], GL_STATIC_DRAW);
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    debug_msg("createdStaticBuffers: vertices %d, triangles %d", (int) vertices.size(), (int) triangles.size());

    return true;
}

void Geometry::destroyBuffers()
{
    if(glIsBuffer(vbo_vertex))
        glDeleteBuffers(1, &vbo_vertex);

    if(glIsBuffer(vbo_triangle))
        glDeleteBuffers(1, &vbo_triangle);

    if(glIsVertexArray(vao))
        glDeleteVertexArrays(1, &vao);

    vao = vbo_vertex = vbo_triangle = 0;
}

void Geometry::draw()
{
    if(vao)
    {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 3 * triangles.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void Geometry::bindVAO()
{
    if(vao)
        glBindVertexArray(vao);
}

void Geometry::unbindVAO()
{
    glBindVertexArray(0);
}

glm::vec3 calculateTangent(glm::vec3 v1, glm::vec3 v2, glm::vec2 st1, glm::vec2 st2)
{
    glm::vec3 tangent;
    float coef = 1.0 / (st1.x * st2.y - st2.x * st1.y);
    return coef * ((v1 * st2.y) + (v2 * -st1.y));
}
