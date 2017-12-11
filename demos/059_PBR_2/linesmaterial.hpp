#pragma once

#include "material.hpp"

struct LinesMaterial : public Material
{
	explicit LinesMaterial();

	virtual void bind() override;
	virtual void unbind() override {}
};