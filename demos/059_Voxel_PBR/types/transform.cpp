#include <glm/gtx/transform.hpp>
#include <glm/gtx/orthonormalize.inl>

#include "transform.hpp"

const glm::vec3 X = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 Y = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 Z = glm::vec3(0.0f, 0.0f, 1.0f);

// The transform change contains a unique set of transforms, on change bool is set to true
std::unordered_map<const Transform *, bool> Transform::transformChange;

Transform::Transform()
{
    position = glm::vec3(0.0f);
    scale = glm::vec3(1.0f);
    rotation = glm::quat(glm::vec3(0.0f));
    forward = Z;
    up = Y;
    right = X;
    transformation = translate(position) * mat4_cast(rotation) * glm::scale(scale);
    UpdateCoordinates();
    RegisterChange(true);
}

Transform::~Transform() { }

void Transform::Position(const glm::vec3 &val)
{
    if(position != val)
    {
        position = val;
        UpdateTransformMatrix();
        transformChange[this] = true;
    }
}

void Transform::Rotation(const glm::quat &val)
{
    if(rotation != val)
    {
        this->angles = eulerAngles(val);
        rotation = val;
        UpdateCoordinates();
        UpdateTransformMatrix();
        transformChange[this] = true;
    }
}

void Transform::Rotation(const glm::vec3 &angles)
{
    if(this->angles != angles)
    {
        this->angles = angles;
        auto rotationX = angleAxis(angles.x, X);
        auto rotationY = angleAxis(angles.y, Y);
        auto rotationZ = angleAxis(angles.z, Z);
        rotation = normalize(rotationZ * rotationX * rotationY);                // final composite rotation
        UpdateCoordinates();                                                    // rotate direction vectors
        UpdateTransformMatrix();
        transformChange[this] = true;
    }
}

void Transform::Scale(const glm::vec3 &val)
{
    if(scale != val)
    {
        scale = val;
        UpdateTransformMatrix();
        transformChange[this] = true;
    }
}

const glm::vec3 &Transform::Position() const
    { return position; }

const glm::quat &Transform::Rotation() const
    { return rotation; }

void Transform::UpdateCoordinates()
{
    up = normalize(Y * rotation);
    right = normalize(X * rotation);
    forward = normalize(Z * rotation);
}

void Transform::UpdateTransformMatrix()
    { transformation = translate(position) * mat4_cast(rotation) * glm::scale(scale); }

bool Transform::TransformChanged() const
{
    auto it = transformChange.find(this);
    if (it != transformChange.end())
        return it->second;
    return false;
}

const std::unordered_map<const Transform *, bool>& Transform::TransformChangedMap()
    { return transformChange; }

void Transform::CleanEventMap()
    { transformChange.clear(); }

void Transform::RegisterChange(bool val) const
    { transformChange[this] = val; }

const glm::vec3 &Transform::Scale() const
    { return scale; }

const glm::vec3 &Transform::Forward() const
    { return forward; }

const glm::vec3 &Transform::Right() const
    { return right; }

const glm::vec3 &Transform::Up() const
    { return up; }

void Transform::Forward(const glm::vec3 &val)
{
    if (forward != val)
    {
        forward = val;
        UpdateTransformMatrix();
        transformChange[this] = true;
    }
}

void Transform::Right(const glm::vec3 &val)
{
    if (right != val)
    {
        right = val;
        UpdateTransformMatrix();
        transformChange[this] = true;
    }
}

void Transform::Up(const glm::vec3 &val)
{
    if (up != val)
    {
        up = val;
        UpdateTransformMatrix();
        transformChange[this] = true;
    }
}

const glm::vec3 &Transform::Angles() const
    { return angles; }

const glm::mat4x4 &Transform::Matrix() const
    { return transformation; }