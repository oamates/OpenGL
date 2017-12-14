#pragma once

#include "../core/interface.hpp"

struct UIShadowingOptions : public Interface
{
	void Draw() override;
	UIShadowingOptions() {}
	~UIShadowingOptions() {}
};