#ifndef GLHELPER_HPP_INCLUDED
#define GLHELPER_HPP_INCLUDED

#include <string>
#include <vector>
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "shader.hpp"

GLuint getShaderAttributeLoc (const glsl_program_t& program, std::string const& name);

/* Only computes the 4 lateral sides of a cube. */
void computeCube (std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals);

bool read_obj(const std::string& filename,
    std::vector<glm::vec3>& positions,
    std::vector<unsigned int>& triangles,
    std::vector<glm::vec3>& normals,
    std::vector<glm::vec2>& texcoords);

#endif // GLHELPER_HPP_INCLUDED
