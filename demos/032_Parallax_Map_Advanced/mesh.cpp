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
    _textured  = _textured || _UVTexture.loadFromFile(filename);
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

void MeshRenderable::do_draw(GLuint shaderHandle, Camera const& camera) const
{
    glm::mat4 normalMatrix = glm::inverseTranspose(camera.getViewMatrix() * _modelMatrix);

    GLuint posALoc = -1, colorALoc = -1, normalALoc = -1, texALoc = -1;                         /* First retrieve locations */
    GLuint modelMatrixULoc = -1, normalMatrixULoc = -1, texturedULoc = -1, UVULoc = -1;

    modelMatrixULoc = getShaderUniformLoc(shaderHandle, "modelMatrix", false);
    normalMatrixULoc = getShaderUniformLoc(shaderHandle, "normalMatrix", false);
    texturedULoc = getShaderUniformLoc(shaderHandle, "textured", false);
    UVULoc = getShaderUniformLoc(shaderHandle, "UVTexture", false);

    posALoc = getShaderAttributeLoc(shaderHandle, "vPosition", false);
    colorALoc = getShaderAttributeLoc(shaderHandle, "vColor", false);
    normalALoc = getShaderAttributeLoc(shaderHandle, "vNormal", false);
    texALoc = getShaderAttributeLoc(shaderHandle, "vTexCoords", false);

    if (modelMatrixULoc != -1)
        glUniformMatrix4fv(modelMatrixULoc, 1, GL_FALSE, glm::value_ptr(_modelMatrix));
    if (normalMatrixULoc != -1)
        glUniformMatrix4fv(normalMatrixULoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    if (texturedULoc != -1)
        glUniform1ui(texturedULoc, _textured);
    if (UVULoc != -1)
        glBindTexture(GL_TEXTURE_2D, _UVTexture.getNativeHandle());

    if (posALoc != -1)
    {
        glEnableVertexAttribArray(posALoc);
        glBindBuffer(GL_ARRAY_BUFFER, _pBuffer);
        glVertexAttribPointer(posALoc, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    }

    if(colorALoc != -1 && !_colors.empty())
    {
        GLCHECK(glEnableVertexAttribArray(colorALoc));
        GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, _cBuffer));
        GLCHECK(glVertexAttribPointer(colorALoc, 4, GL_FLOAT, GL_FALSE, 0, (void*)0));
    }

    if(texALoc != -1 && !_texCoords.empty())
    {
        glEnableVertexAttribArray(texALoc);
        glBindBuffer(GL_ARRAY_BUFFER, _tBuffer);
        glVertexAttribPointer(texALoc, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    }

    if(normalALoc != -1 && !_normals.empty())
    {
        glEnableVertexAttribArray(normalALoc);
        glBindBuffer(GL_ARRAY_BUFFER, _nBuffer);
        glVertexAttribPointer(normalALoc, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iBuffer);
    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, (void*) 0);

    if (UVULoc != -1)
        glBindTexture(GL_TEXTURE_2D, 0);

    if (posALoc != -1)
        glDisableVertexAttribArray(posALoc);

    if (colorALoc != -1)
        glDisableVertexAttribArray(colorALoc);

    if (texALoc != -1)
        glDisableVertexAttribArray(texALoc);

    if (normalALoc != -1)
        glDisableVertexAttribArray(normalALoc);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
