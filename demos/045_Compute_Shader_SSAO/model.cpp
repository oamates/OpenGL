
#include <cstdio>
#include <vector>
#include <map>

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

	debug_msg("File parsed : %u vertices, %u triangles. Calculating normals ... ", (unsigned int) vertices.size(), (unsigned int) indices.size());
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

bool model::load_vnti(const char * file_name)
{

	debug_msg("Loading %s model", file_name);

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

	std::vector<glm::vec3> vertices_t, normals_t;
	std::vector<glm::vec2> uvs_t;
	std::vector<glm::vec3> vertices, normals;
	std::vector<glm::vec2> uvs;
	std::vector<glm::ivec3> indices;
	std::map<uvec3_lex, GLuint> vertex_indices_map;

	while(fgets (buf, sizeof(buf), file))
	{
	//	debug_msg("Working on %s", buf);
	    char token = buf[0];
		// ==================================================================================================================================================================================================
		// skip any line that does not begin with 'v' or 'f'
		// ==================================================================================================================================================================================================
	    if (('v' != token) && ('f' != token)) continue;

		// ==================================================================================================================================================================================================
		// is it a new vertex data ? 
		// ==================================================================================================================================================================================================
		if ('v' == token)
		{
			char vtype = buf[1];
			if (' ' == vtype)
			{
				// we got vertex position
				glm::vec3 vertex;
				sscanf(&buf[2], "%f%f%f", &vertex.x, &vertex.y, &vertex.z);
				vertices_t.push_back(vertex);
				continue;
			};
			if ('t' == vtype)
			{
				glm::vec2 uv;
				sscanf(&buf[2], "%f%f", &uv.x, &uv.y);
				uvs_t.push_back(uv);
				continue;
			};
			if ('n' != vtype) continue;
			glm::vec3 normal;
			sscanf(&buf[2], "%f%f%f", &normal.x, &normal.y, &normal.z);
			normals_t.push_back(normal);
			continue;
		};

		// ==================================================================================================================================================================================================
		// if not, then it is a new polygon
		// ==================================================================================================================================================================================================
		std::vector<GLuint> polygon_indices;
		unsigned int char_index = 2; 
    
		while(true)
		{
			uvec3_lex vnt;
			unsigned int bytes_read;
			if(3 != sscanf(&buf[char_index], "%u/%u/%u%n", &vnt.x, &vnt.y, &vnt.z, &bytes_read)) break;
            char_index += bytes_read;
			std::map<uvec3_lex, GLuint>::iterator vertex_index = vertex_indices_map.find(vnt);
			if (vertex_index != vertex_indices_map.end())
			{
				polygon_indices.push_back(vertex_index->second);
				continue;
			};
			GLuint index = vertices.size();
			vertices.push_back(vertices_t[vnt[0] - 1]);
			normals.push_back(normals_t[vnt[1] - 1]);
			uvs.push_back(uvs_t[vnt[2] - 1]);
			polygon_indices.push_back(index);
			vertex_indices_map[vnt] = index;
		}; 

		for (unsigned int t = 0; t < polygon_indices.size() - 2; ++t)
			indices.push_back(glm::ivec3(polygon_indices[0], polygon_indices[t + 1], polygon_indices[t + 2]));

	};
	triangles = indices.size();

	debug_msg("File parsed : %u vertices, %u triangles.", (unsigned int) vertices.size(), (unsigned int) indices.size());

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
    
	glGenBuffers(1, &tbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, tbo_id);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec3), glm::value_ptr(uvs[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
	glGenBuffers(1, &ibo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles * sizeof(glm::ivec3), glm::value_ptr(indices[0]), GL_STATIC_DRAW);

	debug_msg("VAO created : id = %u", vao_id);
	
	return true;
};

void model::render ()
{
	glBindVertexArray(vao_id);
	glDrawElements(GL_TRIANGLES, 3 * triangles, GL_UNSIGNED_INT, 0);
};