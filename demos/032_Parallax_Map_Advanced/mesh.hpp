#ifndef MESH_RENDERABLE_HPP
#define MESH_RENDERABLE_HPP

#include "Renderable.hpp"

#include <SFML/Graphics/Texture.hpp>

#include <string>
#include <vector>
#include <memory>


struct MeshRenderable : public Renderable
{
    
    MeshRenderable(std::string const& filename, glm::vec4 const& color = glm::vec4(-1.0f));
    virtual ~MeshRenderable();

    void loadUVTexture (std::string const& filename);

    void setTextured(bool textured);
    bool isTextured() const;

    glm::mat4 const& getModelMatrix() const;
    void setModelMatrix (glm::mat4 const& matrix);

    virtual void do_draw (GLuint shaderHandle, Camera const& camera) const;

    bool _textured;
    sf::Texture _UVTexture;

    glm::mat4 _modelMatrix;

    std::vector< glm::vec3 > _positions;
    std::vector< glm::vec3 > _normals;
    std::vector< glm::vec4 > _colors;
    std::vector<glm::vec2> _texCoords;
    std::vector< unsigned int > _indices;

    GLuint _pBuffer;
    GLuint _cBuffer;
    GLuint _nBuffer;
    GLuint _tBuffer;
    GLuint _iBuffer;
};

typedef std::shared_ptr<MeshRenderable> MeshRenderablePtr;

#endif
