#pragma once

#include "transform.hpp"

struct Light 
{
	explicit Light(glm::vec3 position);

	Transform transform;
};