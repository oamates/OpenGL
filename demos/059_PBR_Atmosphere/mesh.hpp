#pragma once

#include "drawable.hpp"

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

struct Triangle
{
	GLuint p0;
	GLuint p1;
	GLuint p2;
};

struct Mesh : public Geometry
{
	virtual void draw() override;
	void initialize(const std::vector<Vertex>& vertices, const std::vector<Triangle>& indices);

	GLuint m_VAO;
	GLuint m_VBO;
	GLuint m_EBO;
	GLuint m_IndicesCount;
};