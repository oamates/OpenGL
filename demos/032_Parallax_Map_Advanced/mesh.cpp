#include <iostream>
#include <fstream>

#include <glm/gtc/matrix_inverse.hpp>

#include "image.hpp"
#include "mesh.hpp"
#include "gl_aux.hpp"
#include "utilities.hpp"

MeshRenderable::MeshRenderable(std::string const& filename, glm::vec4 const& color)
    : Renderable::Renderable(),
      _textured(false),
      _pBuffer(-1), _cBuffer(-1), _nBuffer(-1), _tBuffer(-1), _iBuffer(-1),
      vao_id(-1)
{
    read_obj(filename, _positions, _indices, _normals, _texCoords);

    _colors.resize(_positions.size(), color);
    if (color.r < 0.0f)
    {
        for (std::size_t i = 0; i < _colors.size(); ++i)
            _colors[i] = randomColor();
    }

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    glGenBuffers(1, &_pBuffer); //vertices
    glBindBuffer(GL_ARRAY_BUFFER, _pBuffer);
    glBufferData(GL_ARRAY_BUFFER, _positions.size() * sizeof(glm::vec3), _positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    glGenBuffers(1, &_cBuffer); //colors
    glBindBuffer(GL_ARRAY_BUFFER, _cBuffer);
    glBufferData(GL_ARRAY_BUFFER, _colors.size() * sizeof(glm::vec4), _colors.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);

    glGenBuffers(1, &_nBuffer); //normals
    glBindBuffer(GL_ARRAY_BUFFER, _nBuffer);
    glBufferData(GL_ARRAY_BUFFER, _normals.size() * sizeof(glm::vec3), _normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    glGenBuffers(1, &_tBuffer); //tex
    glBindBuffer(GL_ARRAY_BUFFER, _tBuffer);
    glBufferData(GL_ARRAY_BUFFER, _texCoords.size() * sizeof(glm::vec2), _texCoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

    glGenBuffers(1, &_iBuffer); //indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), _indices.data(), GL_STATIC_DRAW);
}

MeshRenderable::~MeshRenderable()
{
    GLuint buffers[] = {_pBuffer, _cBuffer,_nBuffer, _tBuffer, _iBuffer};
    glDeleteBuffers(sizeof(buffers) / sizeof(GLuint), buffers);
}

void MeshRenderable::loadUVTexture(std::string const& filename)
{
    glActiveTexture(GL_TEXTURE4);
    _UVTexture = image::png::texture2d(filename.c_str());
    _textured = true;
}

void MeshRenderable::setTextured(bool textured)
{
    _textured = textured;
}

bool MeshRenderable::isTextured() const
{
    return _textured;
}

glm::mat4 const& MeshRenderable::getModelMatrix() const
{
    return _modelMatrix;
}

void MeshRenderable::setModelMatrix (glm::mat4 const& matrix)
{
    _modelMatrix = matrix;
}

void MeshRenderable::render(const glsl_program_t& program, Camera const& camera) const
{
    glm::mat4 normalMatrix = glm::inverseTranspose(camera.getViewMatrix() * _modelMatrix);

    program["modelMatrix"] = _modelMatrix;
    program["normalMatrix"] = normalMatrix;
    program["textured"] = (int) _textured;

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, _UVTexture);
    glBindVertexArray(vao_id);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iBuffer);
    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
}
