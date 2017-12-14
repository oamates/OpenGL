#pragma once

#include "../util/single_active.hpp"
#include "../types/frustum.hpp"
#include "../types/scene_object.hpp"

/// <summary>
/// Holds parameters and settings for scene cameras.
/// Viewing parameters and projection setup for the camera instance
/// can be all modified here.
/// </summary>
/// <seealso cref="BaseObject" />
/// <seealso cref="SingleActive{Camera}" />
struct Camera : public SceneObject, public SingleActive<Camera>
{
        void UpdateTransformMatrix() override;

        enum struct ProjectionMode
        {
            Perspective,
            Orthographic
        };

        Camera();
        virtual ~Camera();
        /// <summary>
        /// Disables or enables frustum culling for this camera.
        /// if false the method <see cref="InFrustum"/> will return
        /// always true.
        /// </summary>
        /// <param name="val">The value.</param>
        void DoFrustumCulling(const bool val);
        /// <summary>
        /// Camera projection mode
        /// </summary>
        /// <param name="mode">The mode.</param>
        void Projection(const ProjectionMode &mode);
        /// <summary>
        /// Sets the <see cref="clipPlaneFar"> value.
        /// Value is in the range [0.01, inf]
        /// </summary>
        /// <param name="val">The value.</param>
        void ClipPlaneFar(const float &val);
        /// <summary>
        /// Sets the <see cref="clipPlaneNear"> value.
        /// Value is in the range [0.01, inf]
        /// </summary>
        /// <param name="val">The value.</param>
        void ClipPlaneNear(const float &val);
        /// <summary>
        /// Sets the <see cref="fieldOfView"> value.
        /// Value is in the range [1, 179]
        /// </summary>
        /// <param name="val">The value.</param>
        void FieldOfView(const float &val);
        /// <summary>
        /// The camera's aspect ratio.
        /// </summary>
        /// <param name="val">The value.</param>
        void AspectRatio(const float &val);
        /// <summary>
        /// The camera view rectangle on orthographic mode
        /// </summary>
        /// <param name="rect">The rect.</param>
        void OrthoRect(const glm::vec4 &rect);

        const float &ClipPlaneFar() const;
        const float &ClipPlaneNear() const;
        const float &FieldOfView() const;
        const float &AspectRatio() const;
        const glm::vec4 &OrthoRect() const;
        glm::vec3 LookAt() const;

        const glm::mat4x4 &ViewMatrix() const;
        const glm::mat4x4 &ProjectionMatrix() const;
        const glm::mat4x4 &ViewProjectionMatrix() const;
        const glm::mat4x4 &InverseViewMatrix() const;
        const glm::mat4x4 &InverseProjectionMatrix() const;

        /// <summary>
        /// Checks if the bounding volume is inside the
        /// camera frustum <see cref="frustum"/>
        /// </summary>
        /// <param name="volume">The volume.</param>
        /// <returns></returns>
        bool InFrustum(const BoundingBox &volume) const;
        /// <summary>
        /// Returns the camera's frustum pyramid
        /// </summary>
        /// <returns></returns>
        const CullingFrustum &Frustum() const;

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
        CullingFrustum frustum;

        void UpdateProjectionMatrix();
};

