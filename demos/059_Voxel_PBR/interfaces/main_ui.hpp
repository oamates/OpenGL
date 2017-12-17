#pragma once

#include "../core/ui.hpp"

struct EngineBase;

struct main_ui_t : public ui_t
{
	main_ui_t() {}
	~main_ui_t() override {}

	void render() override;

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
