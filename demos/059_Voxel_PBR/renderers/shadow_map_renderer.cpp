#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "shadow_map_renderer.hpp"

#include "../scene/light.hpp"
#include "../programs/depth_program.hpp"
#include "../programs/blur_program.hpp"
#include "../scene/scene.hpp"
#include "../scene/texture.hpp"
#include "../scene/material.hpp"
#include "../core/assets_manager.hpp"

#include <oglplus/context.hpp>
#include <oglplus/bound/texture.hpp>

void ShadowMapRenderer::SetMaterialUniforms(const Material &material) const
{
    auto &prog = CurrentProgram<DepthProgram>();
    oglplus::Texture::Active(RawTexture::Diffuse);
    material.BindTexture(RawTexture::Diffuse);
    prog.diffuseMap.Set(RawTexture::Diffuse);
}

void ShadowMapRenderer::SetMatricesUniforms(const Node &node) const
{
    auto &prog = CurrentProgram<DepthProgram>();
    static auto &camera = Camera::Active();
    prog.matrices.modelViewProjection.Set(camera->ViewProjectionMatrix() * node.transform.Matrix());
}

void ShadowMapRenderer::Render()
{
    using namespace oglplus;
    static Context gl;
    static Scene * scenePtr = nullptr;
    static auto &scene = Scene::Active();
    auto camera = Camera::Active().get();

    if (!camera || !scene || !scene->IsLoaded() || !shadowCaster)
    {
        return;
    }

    // initially assign invalid direction
    static auto &changes = Transform::TransformChangedMap();
    static bool updateShadowMap = true;

    // any node transformation happened
    for (auto &c : changes)
    {
        auto &type = typeid(*c.first);

        if (type == typeid(Node))
        {
            updateShadowMap = true; break;
        }
    }

    // shadow caster change
    if (shadowCaster->TransformChanged()) { updateShadowMap = true; }

    // scene change
    if (scenePtr != scene.get())
    {
        scenePtr = scene.get();
        updateShadowMap = true;
    }

    if (!updateShadowMap) { return; }

    updateShadowMap = false;
    // update shadow map
    SetAsActive();
    lightView.SetAsActive();
    shadowFramebuffer.Bind(FramebufferTarget::Draw);
    gl.Viewport(0, 0, shadowMapSize.x, shadowMapSize.y);
    gl.ClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    gl.Clear().DepthBuffer().ColorBuffer();
    // activate geometry pass shader program
    auto &prog = DepthShader();
    CurrentProgram<DepthProgram>(prog);
    // rendering flags
    gl.Disable(Capability::Blend);
    gl.Enable(Capability::DepthTest);
    gl.Disable(Capability::CullFace);
    // unneded since directional light cover the whole scene
    // can be useful for view frustum aware light frustum later
    lightView.DoFrustumCulling(false);
    // scene spatial cues
    auto sceneBB = scene->rootNode->boundaries;
    auto &center = sceneBB.Center();
    auto radius = distance(center, sceneBB.MaxPoint());
    // fix light frustum to fit scene bounding sphere
    lightView.OrthoRect(glm::vec4(-radius, radius, -radius, radius));
    lightView.ClipPlaneNear(-radius);
    lightView.ClipPlaneFar(2.0f * radius);
    lightView.Projection(Camera::ProjectionMode::Orthographic);
    lightView.transform.Position(center + shadowCaster->Direction() * radius);
    lightView.transform.Forward(-shadowCaster->Direction());
    // update lightview matrix
    LightSpaceMatrix();
    // uniforms
    prog.exponents.Set(exponents);
    // draw whole scene tree from root node
    scene->rootNode->DrawList();
    // recover original render camera
    camera->SetAsActive();

    // blur the result evsm map
    if(blurScale > 0)
    {
        BlurShadowMap();
    }

    // recover
    DefaultFramebuffer().Bind(FramebufferTarget::Draw);

    // no trilinear filtering
    if (filtering < 2) return;

    // mip map shadow map
    shadowMap.Bind(TextureTarget::_2D);
    shadowMap.GenerateMipmap(TextureTarget::_2D);
}

void ShadowMapRenderer::Caster(const Light * caster)
{
    shadowCaster = caster;
}

const Light * ShadowMapRenderer::Caster() const
{
    return shadowCaster;
}

const glm::mat4x4 &ShadowMapRenderer::LightSpaceMatrix()
{
    static glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0,
                                0.0, 0.5, 0.0, 0.0,
                                0.0, 0.0, 0.5, 0.0,
                                0.5, 0.5, 0.5, 1.0);

    if(lightView.TransformChanged())
    {
        return lightSpaceMatrix = biasMatrix * lightView.ViewProjectionMatrix();
    }

    return lightSpaceMatrix;
}

void ShadowMapRenderer::BindReading(unsigned unit) const
{
    shadowMap.Active(unit);
    shadowMap.Bind(oglplus::TextureTarget::_2D);
}


const Camera &ShadowMapRenderer::LightCamera() const
{
    return lightView;
}

const oglplus::Texture &ShadowMapRenderer::ShadowMap() const
{
    return shadowMap;
}

ShadowMapRenderer::ShadowMapRenderer(RenderWindow &window) : Renderer(window),
    shadowCaster(nullptr)
{
    blurScale = 0.5f;
    blurQuality = 1;
    filtering = 1;
    exponents = glm::vec2(40.0f, 5.0f);
    lightBleedingReduction = 0.0f;
    SetupFramebuffers(1024, 1024);
    fsQuad.Load();
}

ShadowMapRenderer::~ShadowMapRenderer()
{
}

const glm::vec2 &ShadowMapRenderer::Exponents() const
{
    return exponents;
}

void ShadowMapRenderer::Exponents(const glm::vec2 &val)
{
    exponents = val;
}

const float &ShadowMapRenderer::LightBleedingReduction() const
{
    return lightBleedingReduction;
}

void ShadowMapRenderer::LightBleedingReduction(const float &val)
{
    lightBleedingReduction = val;
}

DepthProgram &ShadowMapRenderer::DepthShader()
{
    static auto &assets = AssetsManager::Instance();
    static auto &prog = *static_cast<DepthProgram *>
                        (assets->programs["Depth"].get());
    return prog;
}

BlurProgram &ShadowMapRenderer::BlurShader()
{
    static auto &assets = AssetsManager::Instance();
    static auto &prog = *static_cast<BlurProgram *>
                        (assets->programs["Blur"].get());
    return prog;
}

void ShadowMapRenderer::SetupFramebuffers(const unsigned &w,
        const unsigned &h)
{
    using namespace oglplus;
    static Context gl;
    // save size
    shadowMapSize = glm::uvec2(w, h);
    // setup shadow framebuffer
    shadowFramebuffer.Bind(FramebufferTarget::Draw);
    // create render buffer for depth testing
    depthRender.Bind(RenderbufferTarget::Renderbuffer);
    depthRender.Storage(RenderbufferTarget::Renderbuffer,
                        PixelDataInternalFormat::DepthComponent24, w, h);
    // create variance shadow mapping texture, z and z * z
    gl.Bound(TextureTarget::_2D, shadowMap)
    .Image2D(0, PixelDataInternalFormat::RGBA32F, w, h, 0,
             PixelDataFormat::RGBA, PixelDataType::Float, nullptr);
    Filtering(filtering);
    Anisotropy(8);
    shadowFramebuffer.AttachColorTexture(FramebufferTarget::Draw, 0, shadowMap, 0);
    shadowFramebuffer.AttachRenderbuffer(FramebufferTarget::Draw,
                                         FramebufferAttachment::Depth,
                                         depthRender);
    gl.DrawBuffer(FramebufferColorAttachment::_0);

    // check if success building frame buffer
    if (!Framebuffer::IsComplete(FramebufferTarget::Draw))
    {
        auto status = Framebuffer::Status(FramebufferTarget::Draw);
        Framebuffer::HandleIncompleteError(FramebufferTarget::Draw, status);
    }

    Framebuffer::Bind(Framebuffer::Target::Draw, FramebufferName(0));
    // setup shadow blur framebuffer
    blurFramebuffer.Bind(FramebufferTarget::Draw);
    // create variance shadow mapping texture, z and z * z
    gl.Bound(TextureTarget::_2D, blurShadow)
    .Image2D(0, PixelDataInternalFormat::RGBA32F, w, h, 0,
             PixelDataFormat::RGBA, PixelDataType::Float, nullptr)
    .MinFilter(TextureMinFilter::Linear).MagFilter(TextureMagFilter::Linear)
    .WrapS(TextureWrap::ClampToEdge).WrapT(TextureWrap::ClampToEdge).Anisotropy(8);
    blurFramebuffer.AttachColorTexture(FramebufferTarget::Draw, 0, blurShadow, 0);
    gl.DrawBuffer(FramebufferColorAttachment::_0);

    // check if success building frame buffer
    if (!Framebuffer::IsComplete(FramebufferTarget::Draw))
    {
        auto status = Framebuffer::Status(FramebufferTarget::Draw);
        Framebuffer::HandleIncompleteError(FramebufferTarget::Draw, status);
    }

    Framebuffer::Bind(Framebuffer::Target::Draw, FramebufferName(0));
}

void ShadowMapRenderer::BlurScale(const float &val)
{
    blurScale = val;
}

void ShadowMapRenderer::BlurQuality(const int &val)
{
    blurQuality = glm::clamp(val, 1, 3);
}

void ShadowMapRenderer::Anisotropy(const int &val) const
{
    using namespace oglplus;
    static Context gl;
    gl.Bound(TextureTarget::_2D, shadowMap)
    .Anisotropy(static_cast<float>(val));
}

void ShadowMapRenderer::Filtering(const int &val)
{
    using namespace oglplus;
    static Context gl;
    filtering = glm::clamp(val, 0, 2);

    if(filtering == 0)
        gl.Bound(TextureTarget::_2D, shadowMap)
        .MagFilter(TextureMagFilter::Nearest)
        .MinFilter(TextureMinFilter::Nearest);

    if(filtering == 1)
        gl.Bound(TextureTarget::_2D, shadowMap)
        .MagFilter(TextureMagFilter::Linear)
        .MinFilter(TextureMinFilter::Linear);

    if(filtering == 2)
        gl.Bound(TextureTarget::_2D, shadowMap)
        .MagFilter(TextureMagFilter::Linear)
        .MinFilter(TextureMinFilter::LinearMipmapLinear)
        .GenerateMipmap();
}

void ShadowMapRenderer::BlurShadowMap()
{
    using namespace oglplus;
    static Context gl;
    auto &prog = BlurShader();
    CurrentProgram<BlurProgram>(prog);
    // horizontal blur
    blurFramebuffer.Bind(FramebufferTarget::Draw);
    gl.Disable(Capability::DepthTest);
    // active shadow to be read
    shadowMap.Active(0);
    shadowMap.Bind(TextureTarget::_2D);
    // update uniform
    prog.source.Set(0);
    prog.blurDirection.Set(glm::vec2(1.0f / shadowMapSize.x * blurScale, 0.0f));
    prog.blurType.Set(blurQuality);
    gl.Clear().DepthBuffer().ColorBuffer();
    fsQuad.DrawElements();
    // blur vertically
    shadowFramebuffer.Bind(FramebufferTarget::Draw);
    // active shadow to be read
    blurShadow.Bind(TextureTarget::_2D);
    // update uniform
    prog.source.Set(0);
    prog.blurDirection.Set(glm::vec2(0.0f, 1.0f / shadowMapSize.y * blurScale));
    prog.blurType.Set(blurQuality);
    gl.Clear().DepthBuffer().ColorBuffer();
    fsQuad.DrawElements();
    gl.Enable(Capability::DepthTest);
}
