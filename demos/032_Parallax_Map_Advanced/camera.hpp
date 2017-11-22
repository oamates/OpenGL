#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED

#include <GL/glew.h>
#include "glm.hpp"


/* Camera */
class Camera
{
    public:
        Camera () {}
        virtual ~Camera() {}

        /* Assumes the shader is already binded */
        virtual void sendToShader (GLuint shaderHandle) const;

        /*  Matrice projection * viewMatrix */
        glm::mat4 getViewProjMatrix () const;

        virtual glm::mat4 const& getViewMatrix() const = 0;
        virtual glm::mat4 const& getProjectionMatrix() const = 0;
};

class CameraPerspective: public Camera
{
    public:
        CameraPerspective (float FoV=3.141592654f/2.f,
                           float aspectRatio=1.f,
                           float nearClipping=0.3f,
                           float farClipping=5.f);
        virtual ~CameraPerspective() {}

        /* Assumes the shader is already binded */
        virtual void sendToShader (GLuint shaderHandle) const;

        /* In radians */
        void setFov (float Fov);
        float getFov () const;

        void setAspectRatio(float width, float height);
        float getAspectRatio () const;

        void setNearDist (float near);
        float getNearDist () const;

        void setFarDist (float far);
        float getFarDist () const;

        virtual glm::mat4 const& getProjectionMatrix() const;

        /* Position de la cam�ra */
        virtual glm::vec3 getCameraPosition () const = 0;

    private:
        void computeProjectionMatrix();


    private:
        float _FoV;
        float _aspectRatio;
        float _nearClipping;
        float _farClipping;

        glm::mat4 _projectionMatrix;
};

#endif // CAMERA_HPP_INCLUDED
