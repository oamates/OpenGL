#pragma once

#include "../core/interface.hpp"

struct UIGlobalIllumination : public Interface
{
	void Draw() override;
	UIGlobalIllumination();
	~UIGlobalIllumination();
};

