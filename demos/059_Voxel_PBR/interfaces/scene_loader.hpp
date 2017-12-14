#pragma once

#include "../core/interface.hpp"

struct EngineBase;

struct UISceneLoader : public Interface
{
	void Draw() override;
	UISceneLoader();
	~UISceneLoader() override;
};
