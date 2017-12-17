#pragma once

#include "../core/renderer.hpp"
#include "../primitives/fullscreen_quad.hpp"

#include <oglplus/vertex_array.hpp>

#include <oglplus/texture.hpp>
#include <oglplus/framebuffer.hpp>
#include <oglplus/renderbuffer.hpp>

struct LightingProgram;
struct GeometryProgram;
struct GeometryBuffer;
struct Material;
struct render_window_t;

// The main renderer for the engine. Uses a deferred rendering path.
// Handles geometry pass and light pass logic, creates the necessary geometry buffer and handles uniform pass.
struct GIDeferredRenderer : public Renderer
{
    explicit GIDeferredRenderer(render_window_t& window);                           // Initializes a new instance of the GIDeferredRenderer class.
    ~GIDeferredRenderer();                                                          // Finalizes an instance of the GIDeferredRenderer class.

    void Render() override;                                                         // Renders a frame using deferred rendering
    void SetMatricesUniforms(const Node &node) const override;                      // Sets the matrices uniforms during geometry pass.
    void SetMaterialUniforms(const Material &material) const override;              // Sets the material uniforms during geometry pass.
    const std::array<oglplus::Texture, 5> &BufferTextures() const;
    const float &MaxTracingDistance() const;
    void MaxTracingDistance(const float &val);
    const float &GlobalIlluminationStrength() const;
    void GlobalIlluminationStrength(const float &val);
    const float &AmbientOclussionFalloff() const;
    void AmbientOclussionFalloff(const float &val);
    const float &AmbientOclussionAlpha() const;
    void AmbientOclussionAlpha(const float &val);
    const unsigned int &RenderMode() const;
    void RenderMode(const unsigned int &mode);
    bool SampleVoxelShadowVolume() const;
    void SampleVoxelShadowVolume(bool val);
    const float &SamplingFactor() const;
    void SamplingFactor(const float &val);
    const float &ConeShadowTolerance() const;
    void ConeShadowTolerance(const float &val);
    float ConeShadowAperture() const;
    void ConeShadowAperture(float val);

    FullscreenQuad fsQuad;
        
    static GeometryProgram &GeometryPass();                                         // The geometry program shader.
    static LightingProgram &LightingPass();                                         // The light pass program shader.
    void SetLightPassUniforms() const;                                              // Sets the light pass uniforms.
    void SetupGeometryBuffer(unsigned int windowWidth, unsigned int windowHeight);  // Setups the geometry buffer, initializes the render target textures and attaches these textures

    oglplus::Framebuffer geometryBuffer;
    oglplus::Renderbuffer depthBuffer;
    std::array<oglplus::Texture, 5> bufferTextures;

    float maxTracingDistance;
    float globalIlluminationStrength;
    float ambientOcclusionFalloff;
    float ambientOcclusionAlpha;
    float samplingFactor;
    float coneShadowTolerance;
    float coneShadowAperture;
    bool sampleVoxelShadowVolume;
    unsigned int renderMode;
};
