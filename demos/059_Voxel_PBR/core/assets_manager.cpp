#include "assets_manager.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../scene/texture.hpp"
#include "../scene/scene.hpp"
#include "../primitives/shapes.hpp"

// include program shaders
#include "../programs/lighting_program.hpp"
#include "../programs/geometry_program.hpp"
#include "../programs/voxelization_program.hpp"
#include "../programs/voxel_drawer_program.hpp"
#include "../programs/depth_program.hpp"
#include "../programs/radiance_program.hpp"
#include "../programs/blur_program.hpp"
#include "../programs/mipmapping_program.hpp"
#include "../programs/propagation_program.hpp"
#include "../programs/clear_dynamic_program.hpp"
// include interfaces
#include "../interfaces/scene_loader.hpp"
#include "../interfaces/framerate.hpp"
#include "../interfaces/main_menu.hpp"
#include "../interfaces/scene_cameras.hpp"
#include "../interfaces/scene_lights.hpp"
#include "../interfaces/framebuffer_textures.hpp"
#include "../interfaces/shadowing_options.hpp"
#include "../interfaces/voxelization_options.hpp"
#include "../interfaces/global_illumination_options.hpp"
#include "../interfaces/scene_materials.hpp"
#include "../interfaces/shape_creator.hpp"
// include behaviors
#include "../behaviors/free_camera.hpp"
// include renderers
#include "../renderers/voxelizer_renderer.hpp"
#include "../renderers/gi_deferred_renderer.hpp"
#include "../renderers/shadow_map_renderer.hpp"

std::unique_ptr<AssetsManager> &AssetsManager::Instance()
{
    static std::unique_ptr<AssetsManager> instance = 0;

    if (!instance)
        instance.reset(new AssetsManager());

    return instance;
}

void AssetsManager::Terminate()
{
    delete Instance().release();
}

AssetsManager::AssetsManager()
{
    auto &window = EngineBase::Instance()->Window();
    // instantiate scenes with their paths
    scenes["Crytek Sponza"]           = std::make_shared<Scene> ("models/crytek-sponza/sponza.obj");
    scenes["Crytek Sponza (No Flag)"] = std::make_shared<Scene> ("models/crytek-sponza-noflag/sponza.obj");
    scenes["Sibenik (Open Windows)"]  = std::make_shared<Scene> ("models/sibenik/sibenik.obj");
    scenes["Sibenik (Original)"]      = std::make_shared<Scene> ("models/sibenik_original/sibenik.obj");
    scenes["Cornellbox (Original)"]   = std::make_shared<Scene> ("models/cornell-box/cornell_box.obj");
    scenes["Cornellbox (Empty)"]      = std::make_shared<Scene> ("models/cornell-box/cornell_box_EMPTY.obj");
    scenes["Conference Room"]         = std::make_shared<Scene> ("models/conference/conference.obj");
    scenes["King's Treasure"]         = std::make_shared<Scene> ("models/KingsTreasure/KingsTreasure.obj");
    scenes["Mad Science"]             = std::make_shared<Scene> ("models/science/madScience.obj");
    scenes["Lightbox"]                = std::make_shared<Scene> ("models/lightbox/LightBox01.obj");
    scenes["Test Plane"]              = std::make_shared<Scene> ("models/lightbox/TestPlane.obj");

	// instantiate implemented interfaces
    interfaces["SceneLoader"] = std::make_shared<UISceneLoader>();
    interfaces["Framerate"] = std::make_shared<UIFramerate>();
    interfaces["MainMenu"] = std::make_shared<UIMainMenu>();
    interfaces["Cameras"] = std::make_shared<UISceneCameras>();
    interfaces["Lights"] = std::make_shared<UISceneLights>();
    interfaces["Materials"] = std::make_shared<UISceneMaterials>();
    interfaces["Shapes"] = std::make_shared<UIShapeCreator>();
    interfaces["Framebuffers"] = std::make_shared<UIFramebuffers>();
    interfaces["Shadowing"] = std::make_shared<UIShadowingOptions>();
    interfaces["Voxelization"] = std::make_shared<UIVoxelizationOptions>();
    interfaces["GIOptions"] = std::make_shared<UIGlobalIllumination>();

	// instantiate implemented behaviors
    behaviors["FreeCamera"] = std::make_shared<FreeCamera>();

	// instantiate implemented programs
    programs["Geometry"] = std::make_shared<GeometryProgram>();
    programs["Lighting"] = std::make_shared<LightingProgram>();
    programs["Voxelization"] = std::make_shared<VoxelizationProgram>();
    programs["VoxelDrawer"] = std::make_shared<VoxelDrawerProgram>();
    programs["Depth"] = std::make_shared<DepthProgram>();
    programs["InjectRadiance"] = std::make_shared<InjectRadianceProgram>();
    programs["InjectPropagation"] = std::make_shared<PropagationProgram>();
    programs["MipmappingBase"] = std::make_shared<MipmappingBaseProgram>();
    programs["MipmappingVolume"] = std::make_shared<MipmappingVolumeProgram>();
    programs["Blur"] = std::make_shared<BlurProgram>();
    programs["ClearDynamic"] = std::make_shared<ClearDynamicProgram>();
    // instantiate impleted renderers
    renderers["Shadowmapping"] = std::make_shared<ShadowMapRenderer>(window);
    renderers["Voxelizer"]     = std::make_shared<VoxelizerRenderer>(window);
    renderers["Deferred"]      = std::make_shared<GIDeferredRenderer>(window);
    // attach shaders, ej: programs[index]->AttachShader();
    programs["Geometry"]->AttachShader         (oglplus::ShaderType::Vertex,   "glsl/geometry_pass.vs");
    programs["Geometry"]->AttachShader         (oglplus::ShaderType::Fragment, "glsl/geometry_pass.fs");
    programs["Lighting"]->AttachShader         (oglplus::ShaderType::Vertex,   "glsl/fs_quad.vs");
    programs["Lighting"]->AttachShader         (oglplus::ShaderType::Fragment, "glsl/light_pass.fs");
    programs["Voxelization"]->AttachShader     (oglplus::ShaderType::Vertex,   "glsl/voxelization.vs");
    programs["Voxelization"]->AttachShader     (oglplus::ShaderType::Geometry, "glsl/voxelization.gs");
    programs["Voxelization"]->AttachShader     (oglplus::ShaderType::Fragment, "glsl/voxelization.fs");
    programs["VoxelDrawer"]->AttachShader      (oglplus::ShaderType::Vertex,   "glsl/draw_voxels.vs");
    programs["VoxelDrawer"]->AttachShader      (oglplus::ShaderType::Geometry, "glsl/draw_voxels.gs");
    programs["VoxelDrawer"]->AttachShader      (oglplus::ShaderType::Fragment, "glsl/draw_voxels.fs");
    programs["Depth"]->AttachShader            (oglplus::ShaderType::Vertex,   "glsl/depth_texture.vs");
    programs["Depth"]->AttachShader            (oglplus::ShaderType::Fragment, "glsl/depth_texture.fs");
    programs["InjectRadiance"]->AttachShader   (oglplus::ShaderType::Compute,  "glsl/inject_radiance.cs");
    programs["InjectPropagation"]->AttachShader(oglplus::ShaderType::Compute,  "glsl/inject_propagation.cs");
    programs["MipmappingBase"]->AttachShader   (oglplus::ShaderType::Compute,  "glsl/aniso_mipmapbase.cs");
    programs["MipmappingVolume"]->AttachShader (oglplus::ShaderType::Compute,  "glsl/aniso_mipmapvolume.cs");
    programs["Blur"]->AttachShader             (oglplus::ShaderType::Vertex,   "glsl/fs_quad.vs");
    programs["Blur"]->AttachShader             (oglplus::ShaderType::Fragment, "glsl/blur.fs");
    programs["ClearDynamic"]->AttachShader     (oglplus::ShaderType::Compute,  "glsl/clear_dynamic.cs");

    // link and extract uniforms from shaders
    for (auto &prog : programs)
    {
        prog.second->Link();
        prog.second->ExtractUniforms();
    }

    // utility default assets
    Texture2D::GetDefaultTexture();

    // shapes instancer
    Shapes::Load();
}


AssetsManager::~AssetsManager()
{
    // early owner-ship release for static scope (no gl context)
    auto dTexture = Texture2D::GetDefaultTexture().release();

    if (dTexture != nullptr) { delete dTexture; }

    // shape creator unload
    Shapes::UnloadShapes();
}
