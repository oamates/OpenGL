#pragma once

#include "../core/renderer.hpp"
#include "../primitives/fullscreen_quad.hpp"

#include "../scene/camera.hpp"

#include <oglplus/texture.hpp>
#include <oglplus/framebuffer.hpp>
#include <oglplus/renderbuffer.hpp>

struct BlurProgram;
struct DepthProgram;
struct Light;

struct ShadowMapRenderer : public Renderer
{
	void SetMaterialUniforms(const Material &material) const override;
	void SetMatricesUniforms(const Node &node) const override;
	void Render() override;
	void Caster(const Light * caster);
	const glm::mat4x4 &LightSpaceMatrix();
	const Light * Caster() const;
	void BindReading(unsigned int unit) const;
	void SetupFramebuffers(const unsigned &w, const unsigned &h);

	void BlurScale(const float &val);
	void BlurQuality(const int &val);
	void Anisotropy(const int &val) const;
	void Filtering(const int &val);
	const float &LightBleedingReduction() const;
	void LightBleedingReduction(const float &val);
	const glm::vec2 &Exponents() const;
	void Exponents(const glm::vec2 &val);

    const Camera &LightCamera() const;
    const oglplus::Texture &ShadowMap() const;
    explicit ShadowMapRenderer(RenderWindow &window);
    ~ShadowMapRenderer();

	static DepthProgram &DepthShader();
    static BlurProgram &BlurShader();
    void BlurShadowMap();
    FullscreenQuad fsQuad;
    oglplus::Framebuffer shadowFramebuffer;
    oglplus::Framebuffer blurFramebuffer;
    oglplus::Renderbuffer depthRender;
    oglplus::Texture shadowMap;
    oglplus::Texture blurShadow;

	glm::uvec2 shadowMapSize;
	glm::vec2 exponents;
	float lightBleedingReduction;
	Camera lightView;
	const Light * shadowCaster;
	glm::mat4x4 lightSpaceMatrix;
	float blurScale;
	int blurQuality;
	int filtering;
};

