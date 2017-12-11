#pragma once

#include "drawable.hpp"
#include "spheremesh.hpp"
#include "material.hpp"

struct MoonMaterial : public Material
{
	explicit MoonMaterial();

	virtual void bind() override;
	virtual void unbind() override {}

	std::shared_ptr<Texture> MoonTexture;
	std::shared_ptr<Texture> MoonBumpTexture;
};

struct Moon : public Drawable
{
	explicit Moon(glm::vec3 position, float radius, std::shared_ptr<SphereMesh> mesh);

	virtual void draw() override;

	float Radius;

	std::shared_ptr<SphereMesh>		MoonMesh;
	std::shared_ptr<MoonMaterial>	MoonMat;
};