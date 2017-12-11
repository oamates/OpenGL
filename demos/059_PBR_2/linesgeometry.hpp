#pragma once

#include "geometry.hpp"

#include <glm/glm.hpp>
#include <GL/glew.h>

#include <vector>

struct LineVertex
{
	glm::vec3 Position;
	glm::vec3 Color;
};

struct LinesGeometry : public Geometry
{
	virtual void draw() override;

	void Initialize(const std::vector<LineVertex>& Lines);

	GLuint LinesCount;
	GLuint VAO;
	GLuint VBO;
};