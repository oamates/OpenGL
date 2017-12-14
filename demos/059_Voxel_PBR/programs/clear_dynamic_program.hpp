#pragma once
#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

struct ClearDynamicProgram : public ProgramShader
{
    ClearDynamicProgram() {}
    ~ClearDynamicProgram() {}
    void ExtractUniforms() override {}
};