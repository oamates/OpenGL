#pragma once
#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

struct BlurProgram : public ProgramShader
{
    oglplus::Uniform<glm::vec2> blurDirection;
    oglplus::Uniform<int> blurType;
    oglplus::UniformSampler source;

    BlurProgram() {}
    ~BlurProgram() {}

	void ExtractUniforms() override
    {
        blurDirection.Assign(program);
        blurDirection.BindTo("blurDirection");
        source.Assign(program);
        source.BindTo("source");
        blurType.Assign(program);
        blurType.BindTo("blurType");
    }
};
