#pragma once

#include <memory>

#include "geometry.hpp"
#include "materialparams.hpp"
#include "transform.hpp"

struct Drawable
{
	explicit Drawable(std::shared_ptr<Geometry> mesh);	
	explicit Drawable();

	virtual glm::mat4 modelMatrix() const;

	virtual void draw() = 0;	

	Transform				  transform;	
	MaterialParams			  materialParams;
	std::shared_ptr<Geometry> geometry;
};
