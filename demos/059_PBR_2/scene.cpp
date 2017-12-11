#include "scene.hpp"

Scene::Scene()
{
	light = std::make_shared<Light>(glm::vec3{ 0.0f, 0.0f, 3000.0f });
	m_Camera = std::make_shared<Camera>();
}

void Scene::addDrawable(std::shared_ptr<Drawable> drawable)
{
	m_Drawables.push_back(drawable);
}

void Scene::addAnimator(std::shared_ptr<Animator> animator)
{
	m_Animators.push_back(animator);
}

std::shared_ptr<Camera> Scene::camera() const
{
	return m_Camera;
}

void Scene::draw()
{
	auto projectionMatrix = m_Camera->projectionMatrix();
	auto viewMatrix = m_Camera->viewMatrix();
	auto viewProjection = projectionMatrix * viewMatrix;

	glm::vec4 lightViewPos = viewMatrix * glm::vec4{ light->transform.translation, 1.0f };

	for (auto drawable : m_Drawables)
	{
		auto modelMatrix = drawable->modelMatrix();
		auto modelViewMatrix = viewMatrix * modelMatrix;
		auto normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));

		drawable->materialParams.set("Gamma", m_Gamma);
		drawable->materialParams.set("Time", m_CurrentTime);
		drawable->materialParams.set("Model", modelMatrix);
		drawable->materialParams.set("View", viewMatrix);
		drawable->materialParams.set("Projection", projectionMatrix);
		drawable->materialParams.set("ViewProjection", viewProjection);
		drawable->materialParams.set("ModelView", modelViewMatrix);
		drawable->materialParams.set("ModelViewProjection", viewProjection * modelMatrix);
		drawable->materialParams.set("NormalMatrix", normalMatrix);

		drawable->materialParams.set("CameraWorldPos", m_Camera->eye());
		drawable->materialParams.set("fCameraHeight", glm::length(m_Camera->eye()));
		drawable->materialParams.set("fCameraHeight2", glm::length2(m_Camera->eye()));

		auto lightViewDir = viewMatrix * glm::vec4{ light->transform.translation - drawable->transform.translation, 0.0f };
		drawable->materialParams.set("LightPos", glm::vec3{ lightViewPos } / lightViewPos.w);
		drawable->materialParams.set("LightWorldPos", light->transform.translation);
		drawable->materialParams.set("LightDir", glm::normalize(glm::vec3{ lightViewDir }));
		drawable->materialParams.set("LightWorldDir", glm::normalize(light->transform.translation - drawable->transform.translation));

		drawable->draw();
	}
}

void Scene::update(float deltaTime)
{
	m_CurrentTime += deltaTime;

	for (auto animator : m_Animators)
	{
		animator->update(deltaTime);
	}

	m_Camera->update(deltaTime);
}