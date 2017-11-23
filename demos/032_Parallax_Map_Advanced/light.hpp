#ifndef LIGHT_HPP_INCLUDED
#define LIGHT_HPP_INCLUDED

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "shader.hpp"

#include "mesh.hpp"
#include "trackball_camera.hpp"

struct Light
{
    Light (glm::vec3 const& color = glm::vec3(1.0f), float intensity = 1.0f);
    virtual ~Light();

    virtual void sendToShader (const glsl_program_t& program) const = 0;            /* Assumes the shader is already bound */

    void setIntensity (float intensity);
    float getIntensity () const;
    void setColor (glm::vec3 const& color);
    glm::vec3 const& getColor () const;

    float _intensity;
    glm::vec3 _color;
};

struct SpotLight: public Renderable, public TrackballObject, public Light
{
    SpotLight (glm::vec3 const& focusPoint = glm::vec3(0.0f), float distance = 2.0f, float latitude = 3.1415f / 8.0f, float longitude = -3.1415f / 4.0f);
    virtual ~SpotLight();
        
    virtual void sendToShader (const glsl_program_t& program) const;                /* Assumes the shader is already binded */
    glm::vec3 getSpotDir () const;

    void setSpotSize (float spotSize);                                              /* Maximum value accepted for dot(fromLight, spotDir). For omnidirectional light, set to -2 */
    float getSpotSize() const;

    virtual void positionChanged();
    void updateMesh ();
    
    virtual void render (const glsl_program_t& program, Camera const& camera) const override;         /* Does the actual drawing, once Renderable::draw() checked the shader is ready. */

    float _spotSize;
    MeshRenderablePtr _mesh;
};


typedef std::shared_ptr<SpotLight> SpotLightPtr;

#endif // LIGHT_HPP_INCLUDED
