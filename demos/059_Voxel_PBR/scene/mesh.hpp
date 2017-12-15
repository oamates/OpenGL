#pragma once

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <oglplus/context.hpp>
#include <oglplus/buffer.hpp>
#include <oglplus/vertex_array.hpp>
#include <oglplus/vertex_attrib.hpp>

#include "../types/vertex.hpp"
#include "../types/base_object.hpp"
#include "../types/bounding_box.hpp"


struct Material;

// Mesh base class, contains the mesh data
struct Mesh : public BaseObject
{
    Mesh() 
        { name = "default"; }

    ~Mesh() {}

    BoundingBox boundaries;                     // The mesh's boundaries
    std::vector<Vertex> vertices;               // The mesh's vertices
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
        if (loaded)
            return;
    
        using namespace oglplus;
        
        this->vertexArray = std::make_shared<VertexArray>();                    // create vao
        vertexArray->Bind();
        
        vertexBuffer = std::make_shared<Buffer>();                              // create vertex buffer object and upload vertex data
        vertexBuffer->Bind(BufferTarget::Array);
        Buffer::Data(BufferTarget::Array, this->vertices);

        VertexArrayAttrib(VertexAttribSlot(0)).Enable().Pointer(3, DataType::Float, false, sizeof(Vertex), reinterpret_cast<const GLvoid *>( 0));
        VertexArrayAttrib(VertexAttribSlot(1)).Enable().Pointer(3, DataType::Float, false, sizeof(Vertex), reinterpret_cast<const GLvoid *>(12));
        VertexArrayAttrib(VertexAttribSlot(2)).Enable().Pointer(3, DataType::Float, false, sizeof(Vertex), reinterpret_cast<const GLvoid *>(24));
        VertexArrayAttrib(VertexAttribSlot(3)).Enable().Pointer(3, DataType::Float, false, sizeof(Vertex), reinterpret_cast<const GLvoid *>(36));
        VertexArrayAttrib(VertexAttribSlot(4)).Enable().Pointer(3, DataType::Float, false, sizeof(Vertex), reinterpret_cast<const GLvoid *>(48));
        
        elementBuffer = std::make_shared<Buffer>();                             // create element (indices) buffer object and upload data
        elementBuffer->Bind(BufferTarget::ElementArray);
        Buffer::Data(BufferTarget::ElementArray, this->indices);
        
        this->indicesCount = static_cast<unsigned int>(this->indices.size());   // save number of faces and vertices for rendering
        this->vertexCount = static_cast<unsigned int>(this->vertices.size());
        this->vertices.clear();
        this->indices.clear();
        
        NoVertexArray().Bind();                                                 // unbind vao
        loaded = true;
    }

    virtual void DrawElements() const                                           // Binds the vertex array object and makes a draw call for the elements buffer
    {
        static oglplus::Context gl;
        this->vertexArray->Bind(); 
        gl.DrawElements(oglplus::PrimitiveType::Triangles, indicesCount, oglplus::DataType::UnsignedInt);
    }
};