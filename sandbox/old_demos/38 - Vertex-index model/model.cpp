
#include <cstdio>
#include <vector>
#include <glm/glm.hpp>														                                             
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

#include "model.hpp"
#include "log.hpp"

bool model::load_vi(const char* file_name)
{
	debug_msg("Loading %s model", file_name);

	std::vector<glm::vec3> vertices;
	std::vector<glm::ivec3> indices;

	// ======================================================================================================================================================================================================
	// first check that the file exists
	// ======================================================================================================================================================================================================
    FILE * file = fopen(file_name, "rb");
    if (!file)
    {
    	debug_msg("Cannot open object file %s.", file_name);
    	return false;
    };

	char buf[2048];
	while(fgets (buf, sizeof(buf), file))
	{
	    char token = buf[0];
		// ==================================================================================================================================================================================================
		// skip any line that does not begin with 'v' or 'f'
		// ==================================================================================================================================================================================================
	    if (('v' != token) && ('f' != token)) continue;

		// ==================================================================================================================================================================================================
		// is it a new face? 
		// ==================================================================================================================================================================================================
		if ('v' == token)
		{
			glm::vec3 vertex;
			if (3 != sscanf(&buf[2], "%f %f %f", &vertex[0], &vertex[1], &vertex[2])) continue;
			vertices.push_back(vertex);
			continue;
		};

		// ==================================================================================================================================================================================================
		// if not, then it is a new vertex
		// ==================================================================================================================================================================================================
		glm::ivec3 triangle;
		const glm::ivec3 one = glm::ivec3(1, 1, 1);
		if (3 != sscanf(&buf[2], "%i %i %i", &triangle[0], &triangle[1], &triangle[2])) continue;
		indices.push_back(triangle - one);
	};

	std::vector<glm::vec3> normals (vertices.size(), glm::vec3(0.0f));
	triangles = indices.size();

	debug_msg("File parsed : %u vertices, %u triangles. Calculating normals ... ", vertices.size(), indices.size());
	for (unsigned int i = 0; i < triangles; ++i)
	{
		glm::vec3 normal = glm::cross(vertices[indices[i].y] - vertices[indices[i].x], vertices[indices[i].z] - vertices[indices[i].x]);
		normals[indices[i].x] += normal;	
		normals[indices[i].y] += normal;	
		normals[indices[i].z] += normal;	
	};

	for (unsigned int i = 0; i < normals.size(); ++i) 
	{
		normals[i] = glm::normalize(normals[i]);
//		debug_msg("normals[%d] = %s", i, glm::to_string(normals[i]).c_str());
	};

	debug_msg("Normals calculated");

	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
	glGenBuffers(1, &nbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, nbo_id);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), glm::value_ptr(normals[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
	glGenBuffers(1, &ibo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::ivec3), glm::value_ptr(indices[0]), GL_STATIC_DRAW);
	
	fclose(file);

	debug_msg("VAO created : id = %u", vao_id);

	return true;	
};

bool load_vnti(const char* file_name)
{
	return true;

};


void model::render ()
{
	glBindVertexArray(vao_id);

	glDrawElements(GL_TRIANGLES, 3 * triangles, GL_UNSIGNED_INT, 0);
//	glDrawElements(GL_TRIANGLES, 3000, GL_UNSIGNED_INT, 0);
};
