#include <iostream>
#include <fstream>

#include <glm/gtc/matrix_inverse.hpp>

#include "mesh.hpp"
#include "gl_aux.hpp"
#include "utilities.hpp"

MeshRenderable::MeshRenderable(std::string const& filename, glm::vec4 const& color)
    : Renderable::Renderable(),
      _textured(false),
      _pBuffer(-1), _cBuffer(-1), _nBuffer(-1), _tBuffer(-1), _iBuffer(-1)
{
    read_obj(filename, _positions, _indices, _normals, _texCoords);

    _colors.resize(_positions.size(), color);
    if (color.r < 0.0f)
    {
        for (std::size_t i = 0; i < _colors.size(); ++i)
            _colors[i] = randomColor();
    }

    glGenBuffers(1, &_pBuffer); //vertices
    glGenBuffers(1, &_cBuffer); //colors
    glGenBuffers(1, &_nBuffer); //normals
    glGenBuffers(1, &_tBuffer); //tex
    glGenBuffers(1, &_iBuffer); //indices

    glBindBuffer(GL_ARRAY_BUFFER, _pBuffer);
    glBufferData(GL_ARRAY_BUFFER, _positions.size() * sizeof(glm::vec3), _positions.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, _cBuffer);
    glBufferData(GL_ARRAY_BUFFER, _colors.size() * sizeof(glm::vec4), _colors.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, _nBuffer);
    glBufferData(GL_ARRAY_BUFFER, _normals.size() * sizeof(glm::vec3), _normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, _tBuffer);
    glBufferData(GL_ARRAY_BUFFER, _texCoords.size() * sizeof(glm::vec2), _texCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size()*sizeof(unsigned int), _indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

MeshRenderable::~MeshRenderable()
{
    if (_pBuffer != -1) glDeleteBuffers(1, &_pBuffer);
    if (_cBuffer != -1) glDeleteBuffers(1, &_cBuffer);
    if (_nBuffer != -1) glDeleteBuffers(1, &_nBuffer);
    if (_tBuffer != -1) glDeleteBuffers(1, &_tBuffer);
    if (_iBuffer != -1) glDeleteBuffers(1, &_iBuffer);
}

void MeshRenderable::loadUVTexture(std::string const& filename)
{
    _UVTexture.loadFromFile(filename.c_str());
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

void MeshRenderable::do_draw(const glsl_program_t& program, Camera const& camera) const
{
    glm::mat4 normalMatrix = glm::inverseTranspose(camera.getViewMatrix() * _modelMatrix);

    program["modelMatrix"] = _modelMatrix;
    program["normalMatrix"] = normalMatrix;
    program["textured"] = (int) _textured;
    glBindTexture(GL_TEXTURE_2D, _UVTexture.id);

    GLuint posALoc = getShaderAttributeLoc(program, "vPosition");
    if (posALoc != -1)
    {
        glEnableVertexAttribArray(posALoc);
        glBindBuffer(GL_ARRAY_BUFFER, _pBuffer);
        glVertexAttribPointer(posALoc, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    }

    GLuint colorALoc = getShaderAttributeLoc(program, "vColor");
    if (colorALoc != -1 && !_colors.empty())
    {
        glEnableVertexAttribArray(colorALoc);
        glBindBuffer(GL_ARRAY_BUFFER, _cBuffer);
        glVertexAttribPointer(colorALoc, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

    GLuint texALoc = getShaderAttributeLoc(program, "vTexCoords");
    if (texALoc != -1 && !_texCoords.empty())
    {
        glEnableVertexAttribArray(texALoc);
        glBindBuffer(GL_ARRAY_BUFFER, _tBuffer);
        glVertexAttribPointer(texALoc, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    }

    GLuint normalALoc = getShaderAttributeLoc(program, "vNormal");
    if (normalALoc != -1 && !_normals.empty())
    {
        glEnableVertexAttribArray(normalALoc);
        glBindBuffer(GL_ARRAY_BUFFER, _nBuffer);
        glVertexAttribPointer(normalALoc, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iBuffer);
    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, (void*) 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (posALoc != -1)    glDisableVertexAttribArray(posALoc);
    if (colorALoc != -1)  glDisableVertexAttribArray(colorALoc);
    if (texALoc != -1)    glDisableVertexAttribArray(texALoc);
    if (normalALoc != -1) glDisableVertexAttribArray(normalALoc);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
