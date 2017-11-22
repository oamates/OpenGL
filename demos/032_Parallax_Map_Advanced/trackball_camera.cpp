#include "trackball_camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <algorithm>

#include "utilities.hpp"

static const float F_PI = 3.141592654f;


TrackballObject::TrackballObject (glm::vec3 const& focusPoint, float distance, float latitude, float longitude):
            _focusPoint (focusPoint),
            _distance (std::max(0.f, distance)),
            _latitude (clamp(-3.1415f/2.f, 3.1415f/2.f, latitude)),
            _longitude (longitude)
{
}

void TrackballObject::setFocusPoint (glm::vec3 focusPoint)
{
    _focusPoint = focusPoint;
    positionChanged();
}
glm::vec3 const& TrackballObject::getFocusPoint () const
{
    return _focusPoint;
}

void TrackballObject::setDistance (float distance)
{
    _distance = std::max(0.f, distance);
    positionChanged();
}
float TrackballObject::getDistance () const
{
    return _distance;
}

void TrackballObject::setLatitude (float latitude)
{
    _latitude = clamp(-3.1415f/2.f, 3.1415f/2.f, latitude);
    positionChanged();
}
float TrackballObject::getLatitude () const
{
    return _latitude;
}

void TrackballObject::setLongitude (float longitude)
{
    _longitude = longitude;
    positionChanged();
}
float TrackballObject::getLongitude () const
{
    return _longitude;
}

glm::vec3 TrackballObject::getRelativePosition() const
{
    return getDistance() * glm::vec3(std::cos(getLongitude()) * std::cos(getLatitude()),
                                    std::sin(getLongitude()) * std::cos(getLatitude()),
                                    std::sin(getLatitude()));
}

glm::vec3 TrackballObject::getWorldPosition() const
{
    return getFocusPoint() + getRelativePosition();
}




TrackballCamera::TrackballCamera (glm::vec3 const& focusPoint, float distance, float latitude, float longitude):
            CameraPerspective::CameraPerspective(),
            TrackballObject::TrackballObject(focusPoint, distance, latitude, longitude),
            _cameraUp(glm::vec3(0,0,1))
{
    computeViewMatrix();
}

void TrackballCamera::positionChanged()
{
    computeViewMatrix();
}

glm::vec3 TrackballCamera::getCameraPosition() const
{
    return TrackballObject::getWorldPosition();
}

glm::mat4 const& TrackballCamera::getViewMatrix() const
{
    return _viewMatrix;
}

void TrackballCamera::computeViewMatrix()
{
    _viewMatrix = glm::lookAt(getCameraPosition(), getFocusPoint(), _cameraUp);
}
