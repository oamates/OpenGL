#pragma once

#include "../types/vertex.hpp"
#include "../types/base_object.hpp"
#include "../types/bounding_box.hpp"

#include <vector>
#include <oglplus/buffer.hpp>
#include <oglplus/vertex_array.hpp>

struct Material;

// Mesh base class, contains the mesh data
struct Mesh : public BaseObject
{
	Mesh();
	~Mesh();
	
	BoundingBox boundaries;						// The mesh's boundaries
	std::vector<Vertex> vertices;				// The mesh's vertices
	std::vector<unsigned int> indices;			// The mesh's indices
	std::shared_ptr<Material> material;			// The mesh's material
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

	MeshDrawer();
	~MeshDrawer();

	void BindArrayBuffer() const;							// Binds the vertex array buffer.
    void BindElementArrayBuffer() const;					// Binds the indices array buffer.
    void BindVertexArrayObject() const;						// Binds the vertex array object.
    // Determines whether the mesh's vertex and element buffers, and the mesh's vertex array object have been initialized
    // Returns true if this mesh is loaded; otherwise, returns false.
    bool IsLoaded() const;
    
    virtual void Load();									// Initializes the mesh's vertex buffer, element buffers and vertex array object with the associated mesh data
    virtual void DrawElements() const;						// Binds the vertex array object and makes a draw call for the elements buffer
};

