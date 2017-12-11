#include "sphereanimator.hpp"

#include "sphere.hpp"

SphereAnimator::SphereAnimator(std::shared_ptr<Drawable> drawable) :
	Animator(drawable->transform)
{
	this->drawable = drawable;
}

void SphereAnimator::update(float deltaTime)
{
    const float TranslationSpeed = 4.0;

	auto& transform = drawable->transform;

	auto& translation = transform.translation;
	auto& rotation = transform.rotation;

	translation += TranslationSpeed * deltaTime;
	rotation *= glm::quat(RotationSpeed * deltaTime);	
}
