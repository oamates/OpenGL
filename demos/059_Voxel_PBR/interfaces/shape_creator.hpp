#pragma once
#include "../core/interface.hpp"

struct UIShapeCreator : public Interface
{
	void Draw() override;
	UIShapeCreator() = default;
	~UIShapeCreator() = default;
};