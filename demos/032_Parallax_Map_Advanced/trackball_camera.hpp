#ifndef TRACKBALLCAMERA_HPP_INCLUDED
#define TRACKBALLCAMERA_HPP_INCLUDED

#include <glm/glm.hpp>
#include "camera.hpp"

// Trackball object base class.
// Position is defined by a latitude, a longitutde and a distance from focusPoint.

struct TrackballObject
{
    TrackballObject (glm::vec3 const& focusPoint, float distance, float latitude, float longitude);
    virtual ~TrackballObject() {}

    void setFocusPoint (glm::vec3 focusPoint);
    glm::vec3 const& getFocusPoint () const;
    void setDistance (float distance);                              /* Positive, clamped to 0 otherwise */
    float getDistance () const;
    void setLatitude (float latitude);                              /* In radians */
    float getLatitude () const;
    void setLongitude (float longitude);                            /* In radians, between [-PI/2, PI/2], clamped otherwise*/
    float getLongitude () const;

    glm::vec3 getRelativePosition () const;
    glm::vec3 getWorldPosition () const;

    virtual void positionChanged () {}

    glm::vec3 _focusPoint;
    float _distance;
    float _latitude;
    float _longitude;
};


// Trackball camera class, computing viewMatrix annd projectionMatrix.
// Position is defined by a latitude, a longitutde and a distance from focusPoint.

class TrackballCamera: public CameraPerspective, public TrackballObject
{
    TrackballCamera (glm::vec3 const& focusPoint = glm::vec3(0.0f), float distance = 2.0f, float latitude = 3.1415f / 8.0f, float longitude = -3.1415f / 4.0f);

    virtual glm::mat4 const& getViewMatrix() const;
    virtual glm::vec3 getCameraPosition () const;               /* Position of the camera */
    virtual void positionChanged();
    void computeViewMatrix();

    const glm::vec3 _cameraUp;
    glm::mat4 _viewMatrix;
};

#endif // TRACKBALLCAMERA_HPP_INCLUDED
