#pragma once

#include "drawable.hpp"
#include "spheremesh.hpp"
#include "material.hpp"

struct EarthMaterial : public Material
{
	explicit EarthMaterial();

	virtual void bind() override;
	virtual void unbind() override;

	std::shared_ptr<Texture> EarthTexture;
	std::shared_ptr<Texture> OceanMaskTexture;
	std::shared_ptr<Texture> OceanIceTexture;
	std::shared_ptr<Texture> EarthNightTexture;
	std::shared_ptr<Texture> EarthTopographyTexture;
	std::shared_ptr<Texture> CloudsTexture;

};

struct AtmosphereMaterial : public Material
{
	explicit AtmosphereMaterial();

	virtual void bind() override;
	virtual void unbind() override;
};

struct Earth : public Drawable
{
	explicit Earth(glm::vec3 position, float radius, std::shared_ptr<SphereMesh> mesh);

	virtual void draw() override;

	float	InnerRadius;
	float	OuterRadius;
	float	RayleighScaleDepth = 0.25f;
	float	ESun = 20.0f;
	float	Kr = 0.0020f;	// Rayleigh scattering constant
	float	Km = 0.0010f;	// Mie scattering constant
	float	g = -0.990f;    // The Mie phase asymmetry factor
	glm::vec3 WaveLength = { 0.650f, 0.570f, 0.475f };

	std::shared_ptr<SphereMesh>			EarthMesh;
	std::shared_ptr<EarthMaterial>		EarthSurfaceMaterial;	
	std::shared_ptr<AtmosphereMaterial> EarthAtmosphereMaterial;
};