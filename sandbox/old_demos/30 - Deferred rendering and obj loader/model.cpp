#include <iostream>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "model.hpp"
#include "util.hpp"
#include "log.hpp"

model::model(const char * file_name, GLuint program, GLuint shadowProgram) : vao_id(0), vbo_id(0), ibo_id(0), elements(0), program(program), shadowProgram(shadowProgram), matUnif(-1), shadowMatUnif(-1)
{
	load(file_name);
};

model::~model()
{
	glDeleteBuffers(1, &vbo_id);
	glDeleteBuffers(1, &ibo_id);
	glDeleteVertexArrays(1, &vao_id);
	glDeleteProgram(program);
	if (shadowProgram) glDeleteProgram(shadowProgram);
};

void model::bind()
{
	glUseProgram(program);
	glBindVertexArray(vao_id);
};

void model::bindShadow()
{
	glUseProgram(shadowProgram);
	glBindVertexArray(vao_id);
};

void model::setShadowVP(const glm::mat4 &vp)
{
	if (!shadowProgram) return;
	glUseProgram(shadowProgram);
	GLuint vpUnif = glGetUniformLocation(shadowProgram, "view_proj");
	glUniformMatrix4fv(vpUnif, 1, GL_FALSE, glm::value_ptr(vp));
};

void model::translate(const glm::vec3 &vec)
{
	if (matUnif == -1) return;
	translation = glm::translate<GLfloat>(vec) * translation;
	updateMatrix();
};

void model::rotate(const glm::mat4 &rot)
{
	if (matUnif == -1) return;
	rotation = rot * rotation;
	updateMatrix();
};

void model::scale(const glm::vec3 &scale)
{
	if (matUnif == -1) return;
	scaling = glm::scale<GLfloat>(scale) * scaling;
	updateMatrix();
};

void model::load(const char * file_name)
{
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);
	glGenBuffers(1, &vbo_id);
	glGenBuffers(1, &ibo_id);

	if (!util::loadOBJ(file_name, vbo_id, vao_id, elements))
	{
		debug_msg("Failed to load model : %s", file_name);
		return;
	};

	//Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), 0);
	//Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
	//UVs
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void *)(6 * sizeof(GLfloat)));
	//Send the identity for the initial model matrix, if that uniform is available
	glUseProgram(program);
	matUnif = glGetUniformLocation(program, "model");

	if (shadowProgram)
	{
		glUseProgram(shadowProgram);
		shadowMatUnif = glGetUniformLocation(shadowProgram, "model");
	};

	if (matUnif == -1) return;
	translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
	rotation = glm::rotate(0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	scaling = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	updateMatrix();
};

void model::updateMatrix()
{
	glUseProgram(program);
	glm::mat4 model = translation * rotation * scaling;
	glUniformMatrix4fv(matUnif, 1, GL_FALSE, glm::value_ptr(model));
	if (!shadowProgram) return;
	glUseProgram(shadowProgram);
	glUniformMatrix4fv(shadowMatUnif, 1, GL_FALSE, glm::value_ptr(model));
};

