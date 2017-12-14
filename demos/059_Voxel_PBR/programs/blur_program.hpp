#pragma once
#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

struct BlurProgram : public ProgramShader
{
	oglplus::Uniform<glm::vec2> blurDirection;
	oglplus::Uniform<int> blurType;
	oglplus::UniformSampler source;
	void ExtractUniforms() override;
	BlurProgram();
	~BlurProgram();
};

