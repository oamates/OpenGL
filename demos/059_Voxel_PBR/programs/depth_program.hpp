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

    DepthProgram() {}
    ~DepthProgram() override {}

    void ExtractUniforms() override
    {
        matrices.modelViewProjection.Assign(program);
        matrices.modelViewProjection.BindTo("matrices.modelViewProjection");
        exponents.Assign(program);
        exponents.BindTo("exponents");
        diffuseMap.Assign(program);
        alphaCutoff.Assign(program);
        diffuseMap.BindTo("diffuseMap");
        alphaCutoff.BindTo("alphaCutoff");
    }
};