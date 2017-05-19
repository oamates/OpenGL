#ifndef CAMERA_H
#define CAMERA_H

#include "types.hpp"

#define CAMERA_PLANE_LEFT   0
#define CAMERA_PLANE_RIGHT  1
#define CAMERA_PLANE_BOTTOM 2
#define CAMERA_PLANE_TOP    3
#define CAMERA_PLANE_NEAR   4
#define CAMERA_PLANE_FAR    5

class Camera
{
private:
    vec2 nearfar;
    ivec2 screensize;
    f32 fov;

    vec3 position;
    vec3 orientation;
    mat4 viewMat, projMat;
    mat4 invViewMat, invProjMat;

    vec4 plane[6];

    vec3 *lookat;

public:
    void setup();
    void draw();
    Camera() : nearfar(vec2(0.1f,100.0f)), screensize(ivec2(1024,768)), fov(glm::radians(60.f)), lookat(NULL) {}
    void setPosition(const vec3 &pos) { position = pos; }
    void setPosition(f32 x, f32 y, f32 z) { position = vec3(x,y,z);}
    void setOrientation(const vec3 &ori) { orientation = ori; }
    void setOrientation(f32 x, f32 y, f32 z) { orientation = vec3(x,y,z); }

    void lookAt(vec3 *pos){ lookat = pos; }
    void move(const vec3 &vec);
    void translate(const vec3 &vec) { position+=vec; }

    const vec3 &getPosition() { return position; }
    const vec3 &getOrientation() { return orientation; }

    float getFov() { return fov; }
    float getNear() { return nearfar.x; }
    float getFar() { return nearfar.y; }

    const mat4 &getProjMatrix() { return projMat; }
    const mat4 &getViewMatrix() { return viewMat; }
    const mat4 &getInverseViewMatrix() { return invViewMat; }
    const mat4 &getInverseProjMatrix() { return invProjMat; }
};

#endif
