#pragma once

#include "drawable.hpp"
#include "spheremesh.hpp"
#include "material.hpp"

struct Sphere : public Drawable
{
	Sphere(
		glm::vec3 position, 
		float radius, 
		std::shared_ptr<SphereMesh> mesh, 
		std::shared_ptr<Material>	material
	);

	virtual void draw() override;

	float radius;
	std::shared_ptr<SphereMesh> Mesh;
	std::shared_ptr<Material>	material;
};