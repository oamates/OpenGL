#ifndef LIGHT_HPP_INCLUDED
#define LIGHT_HPP_INCLUDED

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SFML/OpenGL.hpp>

class Light
{
    Light (glm::vec3 const& color = glm::vec3(1.0f), float intensity = 1.0f);
    virtual ~Light() {}
    
    virtual void sendToShader (GLuint shaderHandle) const = 0;              /* Assumes the shader is already bound */

    void setIntensity (float intensity);
    float getIntensity () const;
    void setColor (glm::vec3 const& color);
    glm::vec3 const& getColor () const;

    float _intensity;
    glm::vec3 _color;
};

#endif // LIGHT_HPP_INCLUDED
