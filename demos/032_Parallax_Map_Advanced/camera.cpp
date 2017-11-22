#include "camera.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "gl_aux.hpp"


void Camera::sendToShader(GLuint shaderHandle) const
{
    if (shaderHandle == 0 || shaderHandle == (GLuint)(-1)) // shader not yet loaded
        return;

    /* First retrieve locations */
    GLuint viewProjULoc = -1, projULoc = -1, viewULoc = -1;

    viewProjULoc = getShaderUniformLoc(shaderHandle, "viewProjMatrix", false);
    projULoc = getShaderUniformLoc(shaderHandle, "projMatrix", false);
    viewULoc = getShaderUniformLoc(shaderHandle, "viewMatrix", false);

    if(viewProjULoc != -1)
        glUniformMatrix4fv(viewProjULoc, 1, GL_FALSE, glm::value_ptr(getViewProjMatrix()));

    if(projULoc != -1)
        glUniformMatrix4fv(projULoc, 1, GL_FALSE, glm::value_ptr(getProjectionMatrix()));

    if(viewULoc != -1)
        glUniformMatrix4fv(viewULoc, 1, GL_FALSE, glm::value_ptr(getViewMatrix()));
}

glm::mat4 Camera::getViewProjMatrix() const
{
    return getProjectionMatrix() * getViewMatrix();
}



CameraPerspective::CameraPerspective(float FoV, float aspectRatio, float near, float far):
            Camera::Camera(),
            _FoV(std::min(3.1415f, std::max(0.f, FoV))),
            _aspectRatio(aspectRatio),
            _nearClipping(near),
            _farClipping(far)
{
    computeProjectionMatrix();
}

void CameraPerspective::sendToShader(GLuint shaderHandle) const
{
    Camera::sendToShader(shaderHandle);

    if (shaderHandle == 0 || shaderHandle == (GLuint)(-1)) // shader not yet loaded
        return;

    /* First retrieve locations */
    GLuint eyeULoc = -1;
    eyeULoc = getShaderUniformLoc(shaderHandle, "eyeWorldPos", false);

    if(eyeULoc != -1)
    {
        glm::vec3 eyeWorldPos = getCameraPosition();
        glUniform3f(eyeULoc, eyeWorldPos.x, eyeWorldPos.y, eyeWorldPos.z);
    }
}

void CameraPerspective::setFov (float FoV)
{
    _FoV = std::min(3.1415f, std::max(0.f, FoV));
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
