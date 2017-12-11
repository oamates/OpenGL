#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "drawable.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "animator.hpp"

struct Scene
{
	explicit Scene();

	void addDrawable(std::shared_ptr<Drawable> drawable);
	void addAnimator(std::shared_ptr<Animator> animator);

	std::shared_ptr<Camera> camera() const;

	void draw();
	void update(float deltaTime);

	std::shared_ptr<Light> light;

	std::vector<std::shared_ptr<Drawable>> m_Drawables;	
	std::shared_ptr<Camera>                m_Camera;
	std::vector<std::shared_ptr<Animator>> m_Animators;
	float m_CurrentTime	= 0.0f;
	float m_Gamma			= 1.2f;
};