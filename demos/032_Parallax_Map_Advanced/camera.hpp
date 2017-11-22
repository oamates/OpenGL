#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "shader.hpp"

struct Camera
{
    Camera () {}
    virtual ~Camera() {}

    /* Assumes the shader is already binded */
    virtual void sendToShader (const glsl_program_t& program) const;

    /*  Matrice projection * viewMatrix */
    glm::mat4 getViewProjMatrix () const;

    virtual glm::mat4 const& getViewMatrix() const = 0;
    virtual glm::mat4 const& getProjectionMatrix() const = 0;
};

struct CameraPerspective: public Camera
{
    CameraPerspective (float FoV = 3.141592654f / 2.0f, float aspectRatio = 1.0f, float nearClipping = 0.3f, float farClipping = 5.0f);
    virtual ~CameraPerspective() {}
    
    virtual void sendToShader (const glsl_program_t& program) const;                          /* Assumes the shader is already bound */
    
    void setFov (float Fov);                                                        /* In radians */
    float getFov () const;

    void setAspectRatio(float width, float height);
    float getAspectRatio () const;

    void setNearDist (float near);
    float getNearDist () const;

    void setFarDist (float far);
    float getFarDist () const;

    virtual glm::mat4 const& getProjectionMatrix() const;
    virtual glm::vec3 getCameraPosition () const = 0;                               /* Position of the camera */

    void computeProjectionMatrix();

    float _FoV;
    float _aspectRatio;
    float _nearClipping;
    float _farClipping;

    glm::mat4 _projectionMatrix;
};

#endif // CAMERA_HPP_INCLUDED
