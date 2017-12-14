#pragma once
#include "../core/interface.hpp"

struct UISceneMaterials : public Interface
{
	void Draw() override;
	UISceneMaterials() = default;
	~UISceneMaterials() = default;
};
