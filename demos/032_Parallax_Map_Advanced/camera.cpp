#include "camera.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "gl_aux.hpp"


void Camera::sendToShader(const glsl_program_t& program) const
{
    program["viewProjMatrix"] = getViewProjMatrix();
    program["projMatrix"] = getProjectionMatrix();
    program["viewMatrix"] = getViewMatrix();
}

glm::mat4 Camera::getViewProjMatrix() const
{
    return getProjectionMatrix() * getViewMatrix();
}



CameraPerspective::CameraPerspective(float FoV, float aspectRatio, float near, float far):
    Camera::Camera(),
    _FoV(std::min(3.1415f, std::max(0.0f, FoV))), _aspectRatio(aspectRatio), _nearClipping(near), _farClipping(far)
{
    computeProjectionMatrix();
}

void CameraPerspective::sendToShader(const glsl_program_t& program) const
{
    Camera::sendToShader(program);
    program["eyeWorldPos"] = getCameraPosition();
}

void CameraPerspective::setFov (float FoV)
{
    _FoV = std::min(3.1415f, std::max(0.0f, FoV));
    computeProjectionMatrix();
}
float CameraPerspective::getFov () const
{
    return _FoV;
}

void CameraPerspective::setAspectRatio(float width, float height)
{
    _aspectRatio = width / height;
    computeProjectionMatrix();
}

float CameraPerspective::getAspectRatio () const
{
    return _aspectRatio;
}

void CameraPerspective::setNearDist(float near)
{
    _nearClipping = std::max(0.f, near);
    computeProjectionMatrix();
}

float CameraPerspective::getNearDist() const
{
    return _nearClipping;
}

void CameraPerspective::setFarDist(float far)
{
    _farClipping = std::max(0.f, far);
    computeProjectionMatrix();
}

float CameraPerspective::getFarDist() const
{
    return _farClipping;
}

glm::mat4 const& CameraPerspective::getProjectionMatrix() const
{
    return _projectionMatrix;
}

void CameraPerspective::computeProjectionMatrix()
{
    _projectionMatrix = glm::perspective(_FoV, _aspectRatio, _nearClipping, _farClipping);
}
