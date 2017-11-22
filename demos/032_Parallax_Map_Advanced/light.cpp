#include <glm/gtc/matrix_transform.hpp>

#include "gl_aux.hpp"
#include "light.hpp"

Light::Light(glm::vec3 const& color, float intensity):
    _intensity (intensity),
    _color (color)
{
}

void Light::sendToShader(const glsl_program_t& program) const
{
    program["lightColor"] = _color;
    program["lightIntensity"] = _intensity;
}

void Light::setIntensity (float intensity)
{
    _intensity = intensity;
}

float Light::getIntensity () const
{
    return _intensity;
}

void Light::setColor (glm::vec3 const& color)
{
    _color = color;
}

glm::vec3 const& Light::getColor () const
{
    return _color;
}



SpotLight::SpotLight (glm::vec3 const& focusPoint, float distance, float latitude, float longitude)
    : Renderable::Renderable(),
      TrackballObject::TrackballObject(focusPoint, distance, latitude, longitude),
      Light::Light(),
      _spotSize(0.8f)
{
    _mesh = std::make_shared<MeshRenderable>("res/arrow.obj", glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    updateMesh ();
}

void SpotLight::sendToShader (const glsl_program_t& program) const
{
    Light::sendToShader(program);
    program["normalizedSpotDirection"] = getSpotSize();
    program["spotSize"] = getSpotDir();
    program["lightWorldPos"] = getWorldPosition();

}

glm::vec3 SpotLight::getSpotDir () const
{
    return -getRelativePosition() / glm::length(getRelativePosition());
}

void SpotLight::setSpotSize (float spotSize)
{
    _spotSize = spotSize;
}

float SpotLight::getSpotSize() const
{
    return _spotSize;
}

void SpotLight::positionChanged()
{
    updateMesh();
}

void SpotLight::updateMesh ()
{
    if (!_mesh)
        return;

    glm::mat4 modelMatrix; //identity

    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5));
    modelMatrix = glm::inverse(glm::lookAt(getWorldPosition(), getFocusPoint(), glm::vec3(0, 0, 1))) * modelMatrix;

    _mesh->setModelMatrix (modelMatrix);
}

void SpotLight::do_draw (const glsl_program_t& program, Camera const& camera) const
{
    if (_mesh)
        _mesh->draw (program, camera);
}

