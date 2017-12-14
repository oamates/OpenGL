#pragma once

#include "../core/interface.hpp"

struct UIVoxelizationOptions : public Interface
{
	void Draw() override;
	UIVoxelizationOptions() {}
	~UIVoxelizationOptions() {}
};