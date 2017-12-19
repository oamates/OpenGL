#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "gi_deferred_renderer.hpp"

#include "voxelizer_renderer.hpp"
#include "shadow_map_renderer.hpp"
#include "../scene/camera.hpp"
#include "../scene/scene.hpp"
#include "../scene/material.hpp"
#include "../scene/light.hpp"
#include "../rendering/render_window.hpp"
#include "../core/assets_manager.hpp"
#include "../programs/geometry_program.hpp"
#include "../programs/lighting_program.hpp"

#include <oglplus/bound/texture.hpp>
#include <oglplus/context.hpp>

GIDeferredRenderer::GIDeferredRenderer(render_window_t& window) : Renderer(window)
{
    auto& info = Window().Info();
    
    SetupGeometryBuffer(info.framebufferWidth, info.framebufferHeight);             // create textures and attachments for framebuffer in deferredhandler
    maxTracingDistance = 0.95f;                                                     // initial values
    globalIlluminationStrength = 2.0f;
    ambientOcclusionFalloff = 800.0f;
    ambientOcclusionAlpha = 0.0f;
    samplingFactor = 0.5f;
    coneShadowTolerance = 0.1f;
    coneShadowAperture = 0.03f;
    renderMode = 0;
    fsQuad.Load();
    sampleVoxelShadowVolume = false;
}

GIDeferredRenderer::~GIDeferredRenderer()
{
}

void GIDeferredRenderer::Render()
{
    static oglplus::Context gl;
    static auto &camera = Camera::Active();
    static auto &scene = Scene::Active();
    static auto &info = Window().Info();

    if (!camera || !scene || !scene->IsLoaded() || VoxelizerRenderer::ShowVoxels)
        return;

    SetAsActive();
    
    geometryBuffer.Bind(oglplus::FramebufferTarget::Draw);                                   // bind g buffer for writing
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, info.framebufferWidth, info.framebufferHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    CurrentProgram<GeometryProgram>(GeometryPass());                                // activate geometry pass shader program
    
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    camera->DoFrustumCulling(true);
    scene->rootNode->DrawList();                                                    // draw whole scene tree from root node

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glViewport(0, 0, info.framebufferWidth, info.framebufferHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CurrentProgram<LightingProgram>(LightingPass());
    SetLightPassUniforms();                                                         // pass light info and texture locations for final light pass    
    fsQuad.DrawElements();                                                          // draw the result onto a fullscreen quad
}

void GIDeferredRenderer::SetMatricesUniforms(const Node &node) const
{
    auto &prog = CurrentProgram<GeometryProgram>();
    static auto &camera = Camera::Active();
    prog.matrices.normal.Set(node.InverseTranspose());
    prog.matrices.modelViewProjection.Set(camera->ViewProjectionMatrix() * node.transform.Matrix());
}

void GIDeferredRenderer::SetMaterialUniforms(const Material &material) const
{
    using namespace oglplus;
    auto &prog = CurrentProgram<GeometryProgram>();
    prog.material.diffuse.Set(material.diffuse);
    prog.material.specular.Set(material.specular);
    prog.material.emissive.Set(material.emissive);
    // shininess curve, a bit smoother than linear
    prog.material.shininess.Set(glm::sin(glm::pow(material.shininess_exponent, 3.0f) * glm::half_pi<float>()));
    prog.material.useNormalsMap.Set(material.HasTexture(RawTexture::Normals));
    // set textures
    glActiveTexture(GL_TEXTURE0);
    material.BindTexture(RawTexture::Diffuse);
    glActiveTexture(GL_TEXTURE1);
    material.BindTexture(RawTexture::Specular);
    glActiveTexture(GL_TEXTURE2);
    material.BindTexture(RawTexture::Normals);
    glActiveTexture(GL_TEXTURE3);
    material.BindTexture(RawTexture::Opacity);
    glActiveTexture(GL_TEXTURE4);
    material.BindTexture(RawTexture::Diffuse);
}

const std::array<oglplus::Texture, 5> &GIDeferredRenderer::BufferTextures() const
    { return bufferTextures; }

const float &GIDeferredRenderer::MaxTracingDistance() const
    { return maxTracingDistance; }

void GIDeferredRenderer::MaxTracingDistance(const float &val)
    { maxTracingDistance = val; }

const float &GIDeferredRenderer::GlobalIlluminationStrength() const
    { return globalIlluminationStrength; }

void GIDeferredRenderer::GlobalIlluminationStrength(const float &val)
    { globalIlluminationStrength = val; }

const float &GIDeferredRenderer::AmbientOclussionFalloff() const
    { return ambientOcclusionFalloff; }

void GIDeferredRenderer::AmbientOclussionFalloff(const float &val)
    { ambientOcclusionFalloff = val; }

const float &GIDeferredRenderer::AmbientOclussionAlpha() const
    { return ambientOcclusionAlpha; }

void GIDeferredRenderer::AmbientOclussionAlpha(const float &val)
    { ambientOcclusionAlpha = val; }

const unsigned &GIDeferredRenderer::RenderMode() const
    { return renderMode; }

void GIDeferredRenderer::RenderMode(const unsigned &mode)
    { renderMode = mode; }

bool GIDeferredRenderer::SampleVoxelShadowVolume() const
    { return sampleVoxelShadowVolume; }

void GIDeferredRenderer::SampleVoxelShadowVolume(bool val)
    { sampleVoxelShadowVolume = val; }

void GIDeferredRenderer::SamplingFactor(const float &val)
    { samplingFactor = val; }

const float &GIDeferredRenderer::SamplingFactor() const
    { return samplingFactor; }

const float &GIDeferredRenderer::ConeShadowTolerance() const
    { return coneShadowTolerance; }

void GIDeferredRenderer::ConeShadowTolerance(const float &val)
    { coneShadowTolerance = val; }

float GIDeferredRenderer::ConeShadowAperture() const
    { return coneShadowAperture; }

void GIDeferredRenderer::ConeShadowAperture(float val)
    { coneShadowAperture = val; }

void GIDeferredRenderer::SetLightPassUniforms() const
{
    static auto &camera = Camera::Active();
    static auto &scene = Scene::Active();
    auto &prog = CurrentProgram<LightingProgram>();;
    prog.cameraPosition.Set(camera->Position());
    prog.inverseProjectionView.Set(camera->InverseViewMatrix() *
                                   camera->InverseProjectionMatrix());

    for (int i = 0; i < bufferTextures.size(); i++)
    {
        bufferTextures[i].Active(i);
        bufferTextures[i].Bind(oglplus::TextureTarget::_2D);
    }

    // uniform arrays of lights
    auto &uDirectionals = prog.directionalLight;
    auto &uPoints = prog.pointLight;
    auto &uSpots = prog.spotLight;
    auto &lights = scene->lights;
    // index of directional-point-spot lights
    auto typeIndex = glm::uvec3(0);
    // pass number of lights per type
    prog.lightTypeCount[0].Set(static_cast<const unsigned int> (Light::Directionals().size()));
    prog.lightTypeCount[1].Set(static_cast<const unsigned int> (Light::Points().size()));
    prog.lightTypeCount[2].Set(static_cast<const unsigned int> (Light::Spots().size()));

    for (int i = 0; i < lights.size(); ++i)
    {
        auto &light = lights[i];
        auto &factor = light->Intensities();
        auto shadowingMethod = light->mode[0].to_ulong();
        shadowingMethod = shadowingMethod == 2 && sampleVoxelShadowVolume
                          ? 3 : shadowingMethod;
        // current light uniform
        auto &uLight = light->Type() == Light::Directional
                       ? uDirectionals[typeIndex.x++]
                       : light->Type() == Light::Point
                       ? uPoints[typeIndex.y++]
                       : uSpots[typeIndex.z++];
        // shared uniforms between types
        uLight.shadowingMethod.Set(shadowingMethod);
        uLight.ambient.Set(light->Ambient() * factor.x);
        uLight.diffuse.Set(light->Diffuse() * factor.y);
        uLight.specular.Set(light->Specular() * factor.z);

        if (light->Type() == Light::Spot || light->Type() == Light::Point)
        {
            uLight.position.Set(light->Position());
        }

        if (light->Type() == Light::Spot || light->Type() == Light::Directional)
        {
            uLight.direction.Set(light->Direction());
        }

        if (light->Type() == Light::Spot || light->Type() == Light::Point)
        {
            uLight.attenuation.constant.Set(light->attenuation.Constant());
            uLight.attenuation.linear.Set(light->attenuation.Linear());
            uLight.attenuation.quadratic.Set(light->attenuation.Quadratic());
        }

        if(light->Type() == Light::Spot)
        {
            uLight.angleInnerCone.Set(cos(light->AngleInnerCone()));
            uLight.angleOuterCone.Set(cos(light->AngleOuterCone()));
        }
    }

    // pass shadowing parameters
    static auto &shadowing = *static_cast<ShadowMapRenderer *> (AssetsManager::Instance()->renderers["Shadowmapping"].get());

    if(shadowing.Caster() != nullptr)
    {
        prog.lightViewProjection.Set(shadowing.LightSpaceMatrix());
        shadowing.BindReading(5);
        prog.exponents.Set(shadowing.Exponents());
        prog.lightBleedingReduction.Set(shadowing.LightBleedingReduction());
    }

    static auto &voxel = *static_cast<VoxelizerRenderer *>(AssetsManager::Instance()->renderers["Voxelizer"].get());
    prog.volumeDimension.Set(voxel.VolumeDimension());
    prog.voxelScale.Set(1.0f / voxel.VolumeGridSize());
    prog.worldMinPoint.Set(scene->rootNode->boundaries.MinPoint());
    prog.worldMaxPoint.Set(scene->rootNode->boundaries.MaxPoint());

    if(sampleVoxelShadowVolume)
    {
        voxel.VoxelNormalVisibility().Active(6);
        voxel.VoxelNormalVisibility().Bind(oglplus::TextureTarget::_3D);
    }

    voxel.VoxelRadiance().Active(7);
    voxel.VoxelRadiance().Bind(oglplus::TextureTarget::_3D);
    auto &mips = voxel.VoxelTextureMipmap();

    for (auto i = 0; i < mips.size(); i++)
    {
        mips[0].Active(8 + i);
        mips[0].Bind(oglplus::TextureTarget::_3D);
    }

    // global illum setup
    prog.maxTracingDistanceGlobal.Set(maxTracingDistance);
    prog.bounceStrength.Set(globalIlluminationStrength);
    prog.aoFalloff.Set(ambientOcclusionFalloff);
    prog.aoAlpha.Set(ambientOcclusionAlpha);
    prog.mode.Set(renderMode);
    prog.samplingFactor.Set(samplingFactor);
    prog.coneShadowTolerance.Set(coneShadowTolerance);
    prog.coneShadowAperture.Set(coneShadowAperture);
}

GeometryProgram &GIDeferredRenderer::GeometryPass()
{
    static auto &assets = AssetsManager::Instance();
    static auto &prog = *static_cast<GeometryProgram *> (assets->programs["Geometry"].get());
    return prog;
}

LightingProgram &GIDeferredRenderer::LightingPass()
{
    static auto &assets = AssetsManager::Instance();
    static auto &prog = *static_cast<LightingProgram *> (assets->programs["Lighting"].get());
    return prog;
}

void GIDeferredRenderer::SetupGeometryBuffer(unsigned windowWidth, unsigned windowHeight)
{
    using namespace oglplus;
    static Context gl;
    
    geometryBuffer.Bind(FramebufferTarget::Draw);       // initialize geometry buffer
    // build textures -- normal
    gl.Bound(TextureTarget::_2D, bufferTextures[0]).Image2D(0, PixelDataInternalFormat::RGBA16F, windowWidth, windowHeight, 0, PixelDataFormat::RGB, PixelDataType::Float, nullptr).MinFilter(TextureMinFilter::Nearest).MagFilter(TextureMagFilter::Nearest);
    geometryBuffer.AttachColorTexture(FramebufferTarget::Draw, 0, bufferTextures[0], 0);
    // build textures -- albedo
    gl.Bound(TextureTarget::_2D, bufferTextures[1]).Image2D(0, PixelDataInternalFormat::RGB8, windowWidth, windowHeight, 0, PixelDataFormat::RGB, PixelDataType::UnsignedByte, nullptr).MinFilter(TextureMinFilter::Nearest).MagFilter(TextureMagFilter::Nearest);
    geometryBuffer.AttachColorTexture(FramebufferTarget::Draw, 1, bufferTextures[1], 0);
    // build textures -- specular color and shininess
    gl.Bound(TextureTarget::_2D, bufferTextures[2]).Image2D(0, PixelDataInternalFormat::RGBA8, windowWidth, windowHeight, 0, PixelDataFormat::RGBA, PixelDataType::UnsignedByte, nullptr).MinFilter(TextureMinFilter::Nearest).MagFilter(TextureMagFilter::Nearest);
    geometryBuffer.AttachColorTexture(FramebufferTarget::Draw, 2, bufferTextures[2], 0);
    // emissivenes
    gl.Bound(TextureTarget::_2D, bufferTextures[3]).Image2D(0, PixelDataInternalFormat::RGB8, windowWidth, windowHeight, 0, PixelDataFormat::RGB, PixelDataType::UnsignedByte, nullptr).MinFilter(TextureMinFilter::Nearest).MagFilter(TextureMagFilter::Nearest);
    geometryBuffer.AttachColorTexture(FramebufferTarget::Draw, 3, bufferTextures[3], 0);
    // attach depth texture for depth testing
    gl.Bound(TextureTarget::_2D, bufferTextures[4]).Image2D(0, PixelDataInternalFormat::DepthComponent24, windowWidth, windowHeight, 0, PixelDataFormat::DepthComponent, PixelDataType::Float, nullptr).MinFilter(TextureMinFilter::Nearest).MagFilter(TextureMagFilter::Nearest);
    geometryBuffer.AttachTexture(FramebufferTarget::Draw, FramebufferAttachment::Depth, bufferTextures[4], 0);
    
    auto attachments = std::vector<Context::ColorBuffer>        // color textures
        { FramebufferColorAttachment::_0, FramebufferColorAttachment::_1, FramebufferColorAttachment::_2, FramebufferColorAttachment::_3 };
    gl.DrawBuffers(attachments);                                // set draw buffers
    if (!Framebuffer::IsComplete(FramebufferTarget::Draw))      // check if success building frame buffer
    {
        auto status = Framebuffer::Status(FramebufferTarget::Draw);
        Framebuffer::HandleIncompleteError(FramebufferTarget::Draw, status);
    }
    Framebuffer::Bind(Framebuffer::Target::Draw, FramebufferName(0));
}
