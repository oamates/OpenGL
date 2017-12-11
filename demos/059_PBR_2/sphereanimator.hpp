#pragma once

#include "animator.hpp"

#include <glm/glm.hpp>

#include <memory>

struct Drawable;

struct SphereAnimator : public Animator
{
	explicit SphereAnimator(std::shared_ptr<Drawable> drawable);

	virtual void update(float deltaTime) override;	
	glm::vec3 WorldRotationSpeed;

	std::shared_ptr<Drawable> drawable;
};

