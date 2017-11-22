#ifndef TRACKBALLCAMERA_HPP_INCLUDED
#define TRACKBALLCAMERA_HPP_INCLUDED

#include <glm/glm.hpp>
#include "camera.hpp"

// Trackball object base class.
// Position is defined by a latitude, a longitutde and a distance from focusPoint.

class TrackballObject
{
    public:
        TrackballObject (glm::vec3 const& focusPoint,
                         float distance,
                         float latitude,
                         float longitude);
        virtual ~TrackballObject() {}

        void setFocusPoint (glm::vec3 focusPoint);
        glm::vec3 const& getFocusPoint () const;

        /* Positive, clamped to 0 otherwise */
        void setDistance (float distance);
        float getDistance () const;

        /* In radians */
        void setLatitude (float latitude);
        float getLatitude () const;

        /* In radians, between [-PI/2, PI/2], clamped otherwise*/
        void setLongitude (float longitude);
        float getLongitude () const;

        glm::vec3 getRelativePosition () const;
        glm::vec3 getWorldPosition () const;

    protected:
        virtual void positionChanged () {}


    private:
        glm::vec3 _focusPoint;
        float _distance;
        float _latitude;
        float _longitude;
};



// Trackball camera class, computing viewMatrix annd projectionMatrix.
// Position is defined by a latitude, a longitutde and a distance from focusPoint.

class TrackballCamera: public CameraPerspective, public TrackballObject
{
    public:
        TrackballCamera (glm::vec3 const& focusPoint=glm::vec3(0,0,0),
                         float distance=2.f,
                         float latitude=3.1415f/8.f,
                         float longitude=-3.1415f/4.f);

        virtual glm::mat4 const& getViewMatrix() const;

        /* Position de la caméra */
        virtual glm::vec3 getCameraPosition () const;

    protected:
        virtual void positionChanged();

    private:
        void computeViewMatrix();


    private:
        const glm::vec3 _cameraUp;
        glm::mat4 _viewMatrix;
};

#endif // TRACKBALLCAMERA_HPP_INCLUDED
