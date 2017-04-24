//=======================================================================================================================================================================================================================
//
//  Solid structure methods implementation
//
//=======================================================================================================================================================================================================================

#include <cstdlib>
#include <map>

#include <glm/gtx/string_cast.hpp>
#include <glm/ext.hpp> 

#include "log.hpp"
#include "solid.hpp"
#include "plato.hpp"

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

solid::~solid()
{
	glDeleteBuffers(1, &vbo_id);																				
	glDeleteBuffers(1, &nbo_id);
	glDeleteBuffers(1, &txbo_id);
	glDeleteBuffers(1, &tybo_id);
	glDeleteBuffers(1, &tbo_id);
	glDeleteVertexArrays(1, &vao_id);
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

surface::~surface()
{
	glDeleteBuffers(1, &vbo_id);																				
	glDeleteBuffers(1, &nbo_id);
	glDeleteBuffers(1, &txbo_id);
	glDeleteBuffers(1, &tybo_id);
	glDeleteBuffers(1, &tbo_id);
	glDeleteBuffers(1, &ibo_id);
	glDeleteVertexArrays(1, &vao_id);
};


// ======================================================================================================================================================================================================================
// Implementation of a function graph on the square -1 < x,y < 1 as an indexed mesh
// ======================================================================================================================================================================================================================
graph::graph() {};

void graph::generate_vao(generator_func func, glm::ivec2 size)
{

	GLuint V = (size.x + 1) * (size.y + 1);
	GLuint F = 2 * size.x * size.y;
    triangles = F;

	GLuint VBO_SIZE = V * sizeof(glm::vec3);
	GLuint TBO_SIZE = V * sizeof(glm::vec2);
	GLuint IBO_SIZE = F * sizeof(glm::ivec3);

	glm::vec3* vertices = (glm::vec3*) malloc(VBO_SIZE);
	glm::vec2* uvs = (glm::vec2*) malloc (TBO_SIZE);
	glm::vec3* normals = (glm::vec3*) calloc(VBO_SIZE, 1);
	glm::ivec3* indices = (glm::ivec3*) malloc(IBO_SIZE);

	// ==============================================================================================================================================================================================================
	// vertices and texture coordinates
	// ==============================================================================================================================================================================================================

	glm::vec2 delta = glm::vec2(2.0f / size.x, 2.0f / size.y);
	glm::vec2 argument;
	argument.y = -1.0f;

	unsigned int index = 0;
	for (int v = 0; v <= size.x; ++v)
	{
		argument.x = -1.0f;
		for (int u = 0; u <= size.y; ++u)
		{
			uvs[index] = argument;
			vertices[index++] = glm::vec3(argument, func(argument));
			argument.x += delta.x;
		};
		argument.y += delta.y;	
	};

	// ==============================================================================================================================================================================================================
	// indices and normals
	// ==============================================================================================================================================================================================================

	unsigned int mesh_index = 0;
	index = 0;
	for (int v = 0; v < size.y; ++v)
	{
		for (int u = 0; u < size.x; ++u)
		{
			indices[index] = glm::ivec3(mesh_index, mesh_index + 1, mesh_index + size.x + 2);
			glm::vec3 normal0 = glm::cross(vertices[indices[index].y] - vertices[indices[index].x], vertices[indices[index].z] - vertices[indices[index].x]);
			normals[indices[index].x] += normal0;
			normals[indices[index].y] += normal0;
			normals[indices[index].z] += normal0;
			++index;
			
			indices[index] = glm::ivec3(mesh_index, mesh_index + size.x + 2, mesh_index + size.x + 1);
			glm::vec3 normal1 = glm::cross(vertices[indices[index].y] - vertices[indices[index].x], vertices[indices[index].z] - vertices[indices[index].x]);
			normals[indices[index].x] += normal1;
			normals[indices[index].y] += normal1;
			normals[indices[index].z] += normal1;
			++index;
			++mesh_index;
		};
		++mesh_index;
	};

	for (index = 0; index < V; ++index) normals[index] = glm::normalize(normals[index]);

	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, VBO_SIZE, glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &nbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, nbo_id);
	glBufferData(GL_ARRAY_BUFFER, VBO_SIZE, glm::value_ptr(normals[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &tbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, tbo_id);
	glBufferData(GL_ARRAY_BUFFER, TBO_SIZE, glm::value_ptr(uvs[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &ibo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IBO_SIZE, glm::value_ptr(indices[0]), GL_STATIC_DRAW);

	free(vertices);
	free(normals);
	free(uvs);
	free(indices);
};

void graph::render()
{
	glBindVertexArray(vao_id);
	glDrawElements(GL_TRIANGLES, 3 * triangles, GL_UNSIGNED_INT, 0);
};

graph::~graph()
{
	glDeleteBuffers(1, &vbo_id);																				
	glDeleteBuffers(1, &nbo_id);
	glDeleteBuffers(1, &tbo_id);
	glDeleteBuffers(1, &ibo_id);
	glDeleteVertexArrays(1, &vao_id);
};

// ======================================================================================================================================================================================================================
// Implementation of a function graph on the unit sphere as an indexed mesh
// ======================================================================================================================================================================================================================

// ======================================================================================================================================================================================================================
// Sphere is iteratively subdivided beginning from regular icosahedron.
// V,E,F of any triangular subdivision satisfy : V - E + F = 2, 3F = 2E, F = 2V - 4, E = 3V - 6.
// Minimal possible value of V is 4 = the number of vertices of tetrahedron.
// After n subdivision iterations V(n), E(n), F(n) take values : 
// Fn = 4^n * F, En = 4^n * E, Vn = 4^n * (V - 2) + 2
// ======================================================================================================================================================================================================================

spherical_surface::spherical_surface() {};

void spherical_surface::generate_vao(generator_func func, GLuint level)
{

	GLuint deg4 = 1 << (2 * level);
	GLuint V = deg4 * (plato::icosahedron::V - 2) + 2;
	GLuint E = deg4 * (plato::icosahedron::E);
	GLuint F = deg4 * (plato::icosahedron::F);
    triangles = F;

	debug_msg("V = %u. E = %u. F = %u.", V, E, F);

	GLuint VBO_SIZE = V * sizeof(glm::vec3);
	GLuint IBO_SIZE = F * sizeof(glm::ivec3);
	GLuint v, f;

	debug_msg("VBO_SIZE = %u. IBO_SIZE = %u.", VBO_SIZE, IBO_SIZE);

	glm::vec3* vertices = (glm::vec3*) malloc(VBO_SIZE);
	glm::vec3* normals = (glm::vec3*) calloc(VBO_SIZE, 1);
	glm::vec3* uvs = (glm::vec3*) malloc (VBO_SIZE);
	for(v = 0; v < plato::icosahedron::V; ++v) vertices[v] = func(uvs[v] = glm::normalize(plato::icosahedron::vertex[v]));
		
	glm::ivec3* indices = (glm::ivec3*) malloc(IBO_SIZE);
	for(f = 0; f < plato::icosahedron::F; ++f) indices[f] = plato::icosahedron::triangle[f];

	// ==============================================================================================================================================================================================================
	// vertices and texture coordinates
	// ==============================================================================================================================================================================================================

	for (GLuint l = 0; l < level; ++l)
	{
		GLuint end = f;
		std::map<uvec2_lex, GLuint> center_index;
		for (GLuint triangle = 0; triangle < end; ++triangle)
		{
        	GLuint P = indices[triangle].x;
        	GLuint Q = indices[triangle].y;
        	GLuint R = indices[triangle].z;
			GLuint S, T, U;

			uvec2_lex PQ = (P < Q) ? uvec2_lex(P, Q) : uvec2_lex(Q, P);
			uvec2_lex QR = (Q < R) ? uvec2_lex(Q, R) : uvec2_lex(R, Q);
			uvec2_lex RP = (R < P) ? uvec2_lex(R, P) : uvec2_lex(P, R);

			std::map<uvec2_lex, GLuint>::iterator it = center_index.find(PQ); 
			if (it != center_index.end()) S = it->second;
			else
			{
				S = v++;
				center_index[PQ] = S;
				vertices[S] = func(uvs[S] = glm::normalize(uvs[P] + uvs[Q]));
				
			};
			it = center_index.find(QR); 
			if (it != center_index.end()) T = it->second;
			else
			{
				T = v++;
				center_index[QR] = T;
				vertices[T] = func(uvs[T] = glm::normalize(uvs[Q] + uvs[R]));
				
			};
			it = center_index.find(RP); 
			if (it != center_index.end()) U = it->second;
			else
			{
				U = v++;
				center_index[RP] = U;
				vertices[U] = func(uvs[U] = glm::normalize(uvs[R] + uvs[P]));
			};

            indices[triangle] = glm::ivec3(S, T, U);
            indices[f++]      = glm::ivec3(P, S, U);
            indices[f++]      = glm::ivec3(Q, T, S);
            indices[f++]      = glm::ivec3(R, U, T);
		};
	};

	// ==============================================================================================================================================================================================================
	// vertices and texture coordinates
	// ==============================================================================================================================================================================================================
	for (f = 0; f < F; ++f)
	{
		glm::vec3 normal = glm::cross(vertices[indices[f].y] - vertices[indices[f].x], vertices[indices[f].z] - vertices[indices[f].x]);
		normals[indices[f].x] += normal;
		normals[indices[f].y] += normal;
		normals[indices[f].z] += normal;
	};

	for (v = 0; v < V; ++v) normals[v] = glm::normalize(normals[v]);


	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, VBO_SIZE, glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &nbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, nbo_id);
	glBufferData(GL_ARRAY_BUFFER, VBO_SIZE, glm::value_ptr(normals[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &tbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, tbo_id);
	glBufferData(GL_ARRAY_BUFFER, VBO_SIZE, glm::value_ptr(uvs[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &ibo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IBO_SIZE, glm::value_ptr(indices[0]), GL_STATIC_DRAW);

	free(vertices);
	free(normals);
	free(uvs);
	free(indices);

};

void spherical_surface::render()
{
	glBindVertexArray(vao_id);
	glDrawElements(GL_TRIANGLES, 3 * triangles, GL_UNSIGNED_INT, 0);
};

spherical_surface::~spherical_surface()
{
	glDeleteBuffers(1, &vbo_id);																				
	glDeleteBuffers(1, &nbo_id);
	glDeleteBuffers(1, &ibo_id);
	glDeleteVertexArrays(1, &vao_id);
};

// ======================================================================================================================================================================================================================
// Implementation of a function graph on a torus as an indexed mesh
// ======================================================================================================================================================================================================================

toral_surface::toral_surface() {};

void toral_surface::generate_vao(generator_func func, glm::ivec2 size)
{

	GLuint V = size.x * size.y;
    index_count = (2 * size.x + 3) * size.y - 1;

	debug_msg("V = %u. index_count = %u.", V, index_count);

	GLuint VBO_SIZE = V * sizeof(glm::vec3);
	GLuint TBO_SIZE = V * sizeof(glm::vec2);
	GLuint IBO_SIZE = index_count * sizeof(GLuint);
	GLuint v, f;

	debug_msg("VBO_SIZE = %u. IBO_SIZE = %u.", VBO_SIZE, IBO_SIZE);

	glm::vec3* vertices = (glm::vec3*) malloc(VBO_SIZE);
	glm::vec3* normals = (glm::vec3*) calloc(VBO_SIZE, 1);
	glm::vec2* uvs = (glm::vec2*) malloc (TBO_SIZE);
	GLuint* indices = (GLuint*) malloc(IBO_SIZE);

	// ==============================================================================================================================================================================================================
	// vertices, normals and texture coordinates
	// ==============================================================================================================================================================================================================
	glm::vec2 dx = glm::vec2(0.001f, 0.0f);
	glm::vec2 dy = glm::vec2(0.0f, 0.001f);
	glm::vec2 delta = glm::vec2(1.0f, 1.0f) / glm::vec2(size);
	GLuint index = 0;
	glm::vec2 argument = glm::vec2(0.0f);
	for (GLint q = 0; q < size.y; ++q)
	{
		argument.x = 0.0f;
		for (GLint p = 0; p < size.x; ++p)
		{
            vertices[index] = func(uvs[index] = argument);
			normals[index] = glm::normalize (glm::cross(func(argument + dx) - func(argument - dx), func(argument + dy) - func(argument - dy)));
			argument.x += delta.x;
			++index;
		};
		argument.y += delta.y;
	};

	// ==============================================================================================================================================================================================================
	// indices : toral mesh can naturally be represented as a sequence of triangular strips, -1 is the primitive restart index
	// ==============================================================================================================================================================================================================
	index = 0;
	GLuint a = 0;
	GLuint b = size.x;
	for (GLint q = 0; q < size.y - 1; ++q)
	{
		for (GLint p = 0; p < size.x; ++p)
		{
			indices[index++] = a++;
			indices[index++] = b++;
		};
		indices[index++] = a - size.x;
		indices[index++] = b - size.x;
		indices[index++] = -1;
	};

	b = 0;
	for (GLint p = 0; p < size.x; ++p)
	{
		indices[index++] = a++;
		indices[index++] = b++;
	};
	indices[index++] = a - size.x;
	indices[index++] = b - size.x;


	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, VBO_SIZE, glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &nbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, nbo_id);
	glBufferData(GL_ARRAY_BUFFER, VBO_SIZE, glm::value_ptr(normals[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &tbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, tbo_id);
	glBufferData(GL_ARRAY_BUFFER, TBO_SIZE, glm::value_ptr(uvs[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &ibo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IBO_SIZE, &indices[0], GL_STATIC_DRAW);

	free(vertices);
	free(normals);
	free(uvs);
	free(indices);

};

void toral_surface::render()
{
	glBindVertexArray(vao_id);
	glDrawElements(GL_TRIANGLE_STRIP, index_count, GL_UNSIGNED_INT, 0);
};

toral_surface::~toral_surface()
{
	glDeleteBuffers(1, &vbo_id);																				
	glDeleteBuffers(1, &nbo_id);
	glDeleteBuffers(1, &tbo_id);
	glDeleteBuffers(1, &ibo_id);
	glDeleteVertexArrays(1, &vao_id);
};

