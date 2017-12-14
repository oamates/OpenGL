#pragma once

#include "transform.hpp"

struct SceneObject : public Transform                           // Use for objects that reside within a scene
{
    Transform& transform;                                       // The object's transform

    SceneObject() : transform(*this) {}
    ~SceneObject() {}
    SceneObject(const SceneObject &obj) : transform(*this)
        { name = obj.name; }
};