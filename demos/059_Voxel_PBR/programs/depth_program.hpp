#pragma once
#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

struct DepthProgram : public ProgramShader
{
	struct UniformMatrices
	{
	    oglplus::Uniform<glm::mat4> modelViewProjection;
	};

	UniformMatrices matrices;
    oglplus::Uniform<glm::vec2> exponents;
    oglplus::UniformSampler diffuseMap;
    oglplus::Uniform<float> alphaCutoff;

    void ExtractUniforms() override;
    DepthProgram();
    ~DepthProgram();
};

