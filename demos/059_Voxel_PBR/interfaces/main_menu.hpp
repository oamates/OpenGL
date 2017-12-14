#pragma once

#include "../core/interface.hpp"

struct EngineBase;

struct UIMainMenu : public Interface
{
	UIMainMenu() {}
	~UIMainMenu() override {}

	void Draw() override;
	static bool drawSceneLoader;
	static bool drawFramerate;
	static bool drawSceneCameras;
	static bool drawSceneLights;
	static bool drawFramebuffers;
	static bool drawShadowOptions;
	static bool drawVoxelizationOptions;
	static bool drawGIOptions;
	static bool drawSceneMaterials;
	static bool drawSceneNodes;
};
