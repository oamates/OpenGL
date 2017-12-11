#pragma once

#include "drawable.hpp"
#include "linesmaterial.hpp"
#include "linesgeometry.hpp"

struct AxisGeometry : public LinesGeometry
{
	explicit AxisGeometry(float scale);
};

struct Axis : public Drawable
{
	explicit Axis(float scale = 100.0f);

	virtual void draw() override;

	std::shared_ptr<AxisGeometry> Lines;
	std::shared_ptr<LinesMaterial> AxisMaterial;
};