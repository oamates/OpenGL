#pragma once

#include <glm/detail/type_vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include "base_object.hpp"
#include <unordered_map>

// Handles all transformation operations such  as scaling, rotating and translating
struct Transform : public BaseObject
{
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 angles;
    glm::mat4x4 transformation;

    Transform();
    virtual ~Transform();

    const glm::vec3 &Position() const;
    const glm::quat &Rotation() const;
    const glm::vec3 &Scale() const;
    const glm::vec3 &Forward() const;
    const glm::vec3 &Right() const;
    const glm::vec3 &Up() const;
    const glm::vec3 &Angles() const;
    const glm::mat4x4 &Matrix() const;

    static std::unordered_map<const Transform*, bool> transformChange;
    static const std::unordered_map<const Transform *, bool> &TransformChangedMap();    // Returns a map of bool associated with a transform that tells if the transform has changed

    static void CleanEventMap();                        // Clears the transform changed map
    void RegisterChange(bool val = true) const;         // Registers a change in the transform parameters onto the transform changed map
    bool TransformChanged() const;                      // Determines whether this transform has changed

    virtual void UpdateTransformMatrix();               // Updates the transform matrix.
 
    void Position(const glm::vec3 &val);                // Sets the transform position
    void Rotation(const glm::quat &val);                // Sets the transform rotation.
    void Rotation(const glm::vec3 &angles);             // Sets the transform rotation using euler angles
 
    void Scale(const glm::vec3 &val);                   // Sets the transform scale
    void Forward(const glm::vec3 &val);                 // Sets the forward direction vector. Warning this does not update the rest of the direction vectors.
    void Right(const glm::vec3 &val);                   // Sets the right direction vector. Warning this does not update the rest of the direction vectors.
    void Up(const glm::vec3 &val);                      // Sets the up direction vector. Warning this does not update the rest of the direction vectors.
 
    void UpdateCoordinates();
};