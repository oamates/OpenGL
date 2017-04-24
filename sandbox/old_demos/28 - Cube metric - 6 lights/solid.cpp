//=======================================================================================================================================================================================================================
//  Solid structure methods implementation
//=======================================================================================================================================================================================================================

#include <cstdlib>
#include "log.hpp"
#include "solid.hpp"
#include <glm/gtx/string_cast.hpp>

solid::solid(const glm::vec3* vertices, const glm::vec3* normals, const glm::vec2* uvs, unsigned int mesh_size) : mesh_size(mesh_size)
{
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

	// ==================================================================================================================================================================================================================
	// fill vertex positions and normals buffers
	// ==================================================================================================================================================================================================================

    glEnableVertexAttribArray(0);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, mesh_size * sizeof(glm::vec3), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
	glGenBuffers(1, &nbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, nbo_id);
	glBufferData(GL_ARRAY_BUFFER, mesh_size * sizeof(glm::vec3), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// ==================================================================================================================================================================================================================
	// calculate tangents and fill buffers
	// ==================================================================================================================================================================================================================
	
    glm::vec3 * tangent_x = (glm::vec3 *) malloc(sizeof(glm::vec3) * mesh_size);
    glm::vec3 * tangent_y = (glm::vec3 *) malloc(sizeof(glm::vec3) * mesh_size);

	unsigned int index = 0;
	for (unsigned int i = 0; i < mesh_size; i += 3)
	{

	    glm::mat2x3 tangent_basis = glm::mat2x3(vertices[i + 0] - vertices[i + 1], vertices[i + 0] - vertices[i + 2]);
		glm::mat2 uvs_basis = glm::mat2(uvs[i + 0] - uvs[i + 1], uvs[i + 0] - uvs[i + 2]);
		glm::mat2x3 tangent_xy = tangent_basis * glm::inverse(uvs_basis);
		tangent_x[i + 0] = tangent_xy[0];
		tangent_y[i + 0] = tangent_xy[1];
		tangent_x[i + 1] = tangent_xy[0];
		tangent_y[i + 1] = tangent_xy[1];
		tangent_x[i + 2] = tangent_xy[0];
		tangent_y[i + 2] = tangent_xy[1];
	};

    glEnableVertexAttribArray(2);
	glGenBuffers(1, &txbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, txbo_id);
	glBufferData(GL_ARRAY_BUFFER, mesh_size * sizeof(glm::vec3), tangent_x, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(3);
	glGenBuffers(1, &tybo_id);
	glBindBuffer(GL_ARRAY_BUFFER, tybo_id);
	glBufferData(GL_ARRAY_BUFFER, mesh_size * sizeof(glm::vec3), tangent_y, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

	free(tangent_x);
	free(tangent_y);

	// ==================================================================================================================================================================================================================
	// fill texture coordinates buffer
	// ==================================================================================================================================================================================================================

    glEnableVertexAttribArray(4);
	glGenBuffers(1, &tbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, tbo_id);
	glBufferData(GL_ARRAY_BUFFER, mesh_size * sizeof(glm::vec2), uvs, GL_STATIC_DRAW);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// ==================================================================================================================================================================================================================
	// finally, prepare VAO for rendering to distance texture
	// ==================================================================================================================================================================================================================

    glGenVertexArrays(1, &depth_vao_id);
    glBindVertexArray(depth_vao_id);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

};

void solid::instanced_render(GLsizei primcount)
{
    glActiveTexture(GL_TEXTURE0);																				
	glBindTexture(GL_TEXTURE_2D, texture_id);
    glActiveTexture(GL_TEXTURE1);																				
	glBindTexture(GL_TEXTURE_2D, normal_texture_id);
    glBindVertexArray(vao_id);
    glDrawArraysInstanced(GL_TRIANGLES, 0, mesh_size, primcount);
};

void solid::depth_texture_render(GLsizei primcount)
{
    glBindVertexArray(depth_vao_id);
    glDrawArraysInstanced(GL_TRIANGLES, 0, mesh_size, primcount);
};

solid::~solid()
{
	glDeleteBuffers(1, &vbo_id);																				
	glDeleteBuffers(1, &nbo_id);
	glDeleteBuffers(1, &txbo_id);
	glDeleteBuffers(1, &tybo_id);
	glDeleteBuffers(1, &tbo_id);
	glDeleteVertexArrays(1, &vao_id);
	glDeleteVertexArrays(1, &depth_vao_id);
};


//=======================================================================================================================================================================================================================
//  Smooth solid = surface structure methods implementation
//=======================================================================================================================================================================================================================

surface::surface(surface_func f, unsigned int m, unsigned int n)
{
	static const float eps1 = 1.0f / 1024;
	static const float eps2 = 1.0f / 2048;

	unsigned int grid_size = (m + 1) * (n + 1);

	glm::vec3* vertices  = (glm::vec3 *) malloc(sizeof(glm::vec3) * grid_size); 
	glm::vec3* normals   = (glm::vec3 *) malloc(sizeof(glm::vec3) * grid_size);
	glm::vec3* tangent_x = (glm::vec3 *) malloc(sizeof(glm::vec3) * grid_size);
	glm::vec3* tangent_y = (glm::vec3 *) malloc(sizeof(glm::vec3) * grid_size);
	glm::vec2* uvs       = (glm::vec2 *) malloc(sizeof(glm::vec2) * grid_size);

	unsigned int index = 0;
	glm::vec2 point = glm::vec2(0.0f, 0.0f);
	float delta_x = 1.0f / m;
	float delta_y = 1.0f / n;

	glm::vec2 dx = glm::vec2(delta_x, 0.0f);
	glm::vec2 dy = glm::vec2(0.0f, delta_y);

	for (unsigned int i = 0; i <= n; ++i)
	{
		for (unsigned int j = 0; j <= m; ++j)
		{
			vertices[index] = f(point);
			tangent_x[index] = glm::normalize((f(point + dx) - f(point - dx)) / eps2);
			tangent_y[index] = glm::normalize((f(point + dy) - f(point - dy)) / eps2);
			normals[index] = glm::normalize (glm::cross(tangent_x[index], tangent_y[index]));
			uvs[index] = point;
			point.x += delta_x;
			++index;
		};
		point.y += delta_y;
		point.x = 0.0f;
	};

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

	// ==================================================================================================================================================================================================================
	// fill all the data buffers
	// ==================================================================================================================================================================================================================

    glEnableVertexAttribArray(0);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, grid_size * sizeof(glm::vec3), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
	glGenBuffers(1, &nbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, nbo_id);
	glBufferData(GL_ARRAY_BUFFER, grid_size * sizeof(glm::vec3), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(2);
	glGenBuffers(1, &txbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, txbo_id);
	glBufferData(GL_ARRAY_BUFFER, grid_size * sizeof(glm::vec3), tangent_x, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(3);
	glGenBuffers(1, &tybo_id);
	glBindBuffer(GL_ARRAY_BUFFER, tybo_id);
	glBufferData(GL_ARRAY_BUFFER, grid_size * sizeof(glm::vec3), tangent_y, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(4);
	glGenBuffers(1, &tbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, tbo_id);
	glBufferData(GL_ARRAY_BUFFER, grid_size * sizeof(glm::vec2), uvs, GL_STATIC_DRAW);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, 0);

	free(vertices);
	free(normals);
	free(tangent_x);
	free(tangent_y);
	free(uvs);

	// ==================================================================================================================================================================================================================
	// fill the index buffer
	// ==================================================================================================================================================================================================================

	index = 0;
	mesh_size = 6 * m * n;
	GLushort* elements = (GLushort*) malloc(sizeof(GLushort) * mesh_size); 
	GLushort q = m + 1;
	GLushort p = 0;
	for (unsigned int i = 0; i < n; ++i)
	{
		for (unsigned int j = 0; j < m; ++j)
		{
			elements[index++] = p;
			elements[index++] = p + 1;
			elements[index++] = q + 1;
			elements[index++] = p;
			elements[index++] = q + 1;
			elements[index++] = q;
			++p; ++q;
		};
		++p; ++q;
	};

	glGenBuffers(1, &ibo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_size * sizeof(GLushort), elements, GL_STATIC_DRAW);

	free(elements);

	// ==================================================================================================================================================================================================================
	// finally, prepare VAO for rendering to distance texture
	// ==================================================================================================================================================================================================================

    glGenVertexArrays(1, &depth_vao_id);
    glBindVertexArray(depth_vao_id);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);

};


void surface::instanced_render(GLsizei primcount)
{
    glActiveTexture(GL_TEXTURE0);																				
	glBindTexture(GL_TEXTURE_2D, texture_id);
    glActiveTexture(GL_TEXTURE1);																				
	glBindTexture(GL_TEXTURE_2D, normal_texture_id);
    glBindVertexArray(vao_id);
    glDrawElementsInstanced(GL_TRIANGLES, mesh_size, GL_UNSIGNED_SHORT, 0, primcount);
};

void surface::depth_texture_render(GLsizei primcount)
{
    glBindVertexArray(depth_vao_id);
    glDrawElementsInstanced(GL_TRIANGLES, mesh_size, GL_UNSIGNED_SHORT, 0, primcount);
};

surface::~surface()
{
	glDeleteBuffers(1, &vbo_id);																				
	glDeleteBuffers(1, &nbo_id);
	glDeleteBuffers(1, &txbo_id);
	glDeleteBuffers(1, &tybo_id);
	glDeleteBuffers(1, &tbo_id);
	glDeleteBuffers(1, &ibo_id);
	glDeleteVertexArrays(1, &vao_id);
	glDeleteVertexArrays(1, &depth_vao_id);
};
