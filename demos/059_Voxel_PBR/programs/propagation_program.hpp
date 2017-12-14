#pragma once
#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

struct PropagationProgram : public ProgramShader
{
	oglplus::Uniform<float> maxTracingDistanceGlobal;
	oglplus::Uniform<int> volumeDimension;
	oglplus::Uniform<unsigned int>  checkBoundaries;

    PropagationProgram() {}
    ~PropagationProgram() override {}

	void ExtractUniforms() override
    {
        maxTracingDistanceGlobal.Assign(program);
        maxTracingDistanceGlobal.BindTo("maxTracingDistanceGlobal");
        volumeDimension.Assign(program);
        volumeDimension.BindTo("volumeDimension");
        checkBoundaries.Assign(program);
        checkBoundaries.BindTo("checkBoundaries");
    }
};