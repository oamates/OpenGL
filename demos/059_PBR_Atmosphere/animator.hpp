#pragma once

#include "drawable.hpp"

struct Animator
{
	explicit Animator(Transform& transform);

	virtual void update(float deltaTime);

	glm::vec3 RotationSpeed;
	glm::vec3 WorldRotationSpeed;

	Transform& transform;
};