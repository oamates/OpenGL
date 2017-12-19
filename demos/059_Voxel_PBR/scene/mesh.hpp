#pragma once

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <oglplus/context.hpp>
#include <oglplus/buffer.hpp>
#include <oglplus/vertex_array.hpp>
#include <oglplus/vertex_attrib.hpp>

#include "../types/base_object.hpp"
#include "../types/bbox.hpp"

#include "vertex.hpp"


struct Material;

// Mesh base class, contains the mesh data
struct Mesh : public BaseObject
{
    Mesh() 
        { name = "default"; }

    ~Mesh() {}

    bbox_t boundaries;                          // The mesh's boundaries
    std::vector<vertex_pft3_t> vertices;               // The mesh's vertices
    std::vector<unsigned int> indices;          // The mesh's indices
    std::shared_ptr<Material> material;         // The mesh's material
};

// Mesh drawer class contains the neccesary buffers to draw the associated mesh data
struct MeshDrawer : public Mesh
{
    std::shared_ptr<oglplus::Buffer> vertexBuffer;
    std::shared_ptr<oglplus::Buffer> elementBuffer;
    std::shared_ptr<oglplus::VertexArray> vertexArray;

    bool loaded;
    unsigned int indicesCount;
    unsigned int vertexCount;

    MeshDrawer() : loaded(false), indicesCount(0), vertexCount(0) {}
    ~MeshDrawer() {}

    void BindArrayBuffer() const                                                // Binds the vertex array buffer.
        { this->vertexBuffer->Bind(oglplus::Buffer::Target::Array); }

    void BindElementArrayBuffer() const                                         // Binds the indices array buffer.
        { this->elementBuffer->Bind(oglplus::Buffer::Target::ElementArray); }

    void BindVertexArrayObject() const                                          // Binds the vertex array object.
        { this->vertexArray->Bind(); }

    bool IsLoaded() const                                                       // Returns true if this mesh is loaded; otherwise, returns false.
        { return loaded; }
    
    virtual void Load()                                                         // Initializes the mesh's vertex buffer, element buffers and vertex array object with the associated mesh data
    {
        if (loaded) return;
    
        this->vertexArray = std::make_shared<oglplus::VertexArray>();                    // create vao
        vertexArray->Bind();
        
        vertexBuffer = std::make_shared<oglplus::Buffer>();                              // create vertex buffer object and upload vertex data
        vertexBuffer->Bind(oglplus::BufferTarget::Array);
        oglplus::Buffer::Data(oglplus::BufferTarget::Array, this->vertices);

        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pft3_t), (const GLvoid *) offsetof(vertex_pft3_t, position));
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pft3_t), (const GLvoid *) offsetof(vertex_pft3_t, uvw));
        glEnableVertexAttribArray(2); glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pft3_t), (const GLvoid *) offsetof(vertex_pft3_t, normal));
        glEnableVertexAttribArray(3); glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pft3_t), (const GLvoid *) offsetof(vertex_pft3_t, tangent_x));
        glEnableVertexAttribArray(4); glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pft3_t), (const GLvoid *) offsetof(vertex_pft3_t, tangent_y));

        elementBuffer = std::make_shared<oglplus::Buffer>();                             // create element (indices) buffer object and upload data
        elementBuffer->Bind(oglplus::BufferTarget::ElementArray);
        oglplus::Buffer::Data(oglplus::BufferTarget::ElementArray, this->indices);
        
        this->indicesCount = static_cast<unsigned int>(this->indices.size());   // save number of faces and vertices for rendering
        this->vertexCount = static_cast<unsigned int>(this->vertices.size());
        this->vertices.clear();
        this->indices.clear();
        
        glBindVertexArray(0);
        loaded = true;
    }

    virtual void DrawElements() const                                           // Binds the vertex array object and makes a draw call for the elements buffer
    {
        this->vertexArray->Bind();
        glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
    }
};