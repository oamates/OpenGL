#pragma once
#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

struct PropagationProgram : public ProgramShader
{
	PropagationProgram();
	~PropagationProgram();

	oglplus::Uniform<float> maxTracingDistanceGlobal;
	oglplus::Uniform<int> volumeDimension;
	oglplus::Uniform<unsigned int>  checkBoundaries;
	void ExtractUniforms() override;
};

