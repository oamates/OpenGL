#pragma once

#include "../core/interface.hpp"

struct Scene;
struct Light;

struct UISceneLights : public Interface
{
	void Draw() override;
	UISceneLights();
	~UISceneLights();
};

