#pragma once

#include <glm/detail/func_common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../util/single_active.hpp"
#include "../types/bbox.hpp"
#include "../types/frustum.hpp"
#include "../types/scene_object.hpp"

// Holds parameters and settings for scene cameras.
// Viewing parameters and projection setup for the camera instance can be all modified here.
struct Camera : public SceneObject, public SingleActive<Camera>
{
    enum struct ProjectionMode { Perspective, Orthographic };

    bool doFrustumCulling;
    float clipPlaneFar;
    float clipPlaneNear;
    float fieldOfView;
    float aspectRatio;
    glm::vec4 orthoRect;
    ProjectionMode mode;

    glm::mat4x4 viewMatrix;
    glm::mat4x4 projectionMatrix;
    glm::mat4x4 viewProjectionMatrix;
    glm::mat4x4 inverseViewMatrix;
    glm::mat4x4 inverseProjectionMatrix;
    frustum_t frustum;

    Camera()
        : clipPlaneFar(10000.0f), clipPlaneNear(0.3f),
          fieldOfView(glm::radians(60.0f)), aspectRatio(16.0f / 9.0f),
          mode(ProjectionMode::Perspective)
    {
        name = "Default Camera";
        viewMatrix = lookAt(transform.Position(), LookAt(), transform.Up());
        inverseViewMatrix = inverse(viewMatrix);
        doFrustumCulling = true;
        UpdateProjectionMatrix();
    }

    virtual ~Camera() {}

    void UpdateTransformMatrix() override
    {
        viewMatrix = lookAt(transform.Position(), LookAt(), transform.Up());
        inverseViewMatrix = inverse(viewMatrix);
        viewProjectionMatrix = projectionMatrix * viewMatrix;
        frustum.ExtractPlanes(viewProjectionMatrix);
    }

    void DoFrustumCulling(const bool val)               // Disables or enables frustum culling for this camera. if false the method InFrustum will return always true.
        { doFrustumCulling = val; }

    void Projection(const ProjectionMode &mode)         // Camera projection mode
    {
        if (this->mode != mode)
        {
            this->mode = mode;
            UpdateProjectionMatrix();
        }
    }

    void ClipPlaneFar(const float &val)                 // Sets the clipPlaneFar value. Value is in the range [0.01, inf]
    {
        float res = glm::max(val, 0.01f);
        if (res != clipPlaneFar)
        {
            clipPlaneFar = res;
            UpdateProjectionMatrix();
        }
    }

    void ClipPlaneNear(const float &val)                // Sets the clipPlaneNear value. Value is in the range [0.01, inf]
    {
        float res = glm::max(val, 0.01f);
        if (res != clipPlaneNear)
        {
            clipPlaneNear = res;
            UpdateProjectionMatrix();
        }
    }

    void FieldOfView(const float &val)                  // Sets the fieldOfView value. Value is in the range [1, 179]
    {
        float res = glm::clamp(val, 0.0f, glm::pi<float>());
        if (res != fieldOfView)
        {
            fieldOfView = res;
            UpdateProjectionMatrix();
        }
    }

    void AspectRatio(const float &val)                  // The camera's aspect ratio.
    {
        if (val != aspectRatio)
        {
            aspectRatio = val;
            UpdateProjectionMatrix();
        }
    }

    void OrthoRect(const glm::vec4 &rect)               // The camera view rectangle on orthographic mode
    {
        if (rect != orthoRect)
        {
            orthoRect = rect;
            UpdateProjectionMatrix();
        }
    }

    const float &ClipPlaneFar() const
        { return clipPlaneFar; }

    const float &ClipPlaneNear() const
        { return clipPlaneNear; }

    const float &FieldOfView() const
        { return fieldOfView; }

    const float &AspectRatio() const
        { return aspectRatio; }

    const glm::vec4 &OrthoRect() const
        { return orthoRect; }

    glm::vec3 LookAt() const
        { return transform.Position() + transform.Forward(); }

    const glm::mat4x4 &ViewMatrix() const
        { return viewMatrix; }
    const glm::mat4x4 &ProjectionMatrix() const
        { return projectionMatrix; }
    const glm::mat4x4 &ViewProjectionMatrix() const
        { return viewProjectionMatrix; }
    const glm::mat4x4 &InverseViewMatrix() const
        { return inverseViewMatrix; }
    const glm::mat4x4 &InverseProjectionMatrix() const
        { return inverseProjectionMatrix; }
        
    bool InFrustum(const bbox_t& volume) const          // Checks if the bounding volume is inside the camera frustum
    {
        if (!doFrustumCulling)
            return true;
        return frustum.InFrustum(volume);
    }

    const frustum_t& Frustum() const               // Returns the camera's frustum pyramid
        { return frustum; }

    void UpdateProjectionMatrix()
    {
        projectionMatrix = (mode == ProjectionMode::Perspective) ? 
            glm::perspective(fieldOfView, aspectRatio, clipPlaneNear, clipPlaneFar) : 
            glm::ortho(orthoRect.x, orthoRect.y, orthoRect.z, orthoRect.w, clipPlaneNear, clipPlaneFar);

        inverseProjectionMatrix = glm::inverse(projectionMatrix);
        viewProjectionMatrix = projectionMatrix * viewMatrix;
        frustum.ExtractPlanes(viewProjectionMatrix);
    }
};