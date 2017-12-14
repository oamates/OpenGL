#pragma once
#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <glm/vec3.hpp>

struct MipmappingBaseProgram : public ProgramShader
{
    oglplus::Uniform<int> mipDimension;

    MipmappingBaseProgram() {}
    ~MipmappingBaseProgram() override {}

    void ExtractUniforms() override
    {
        mipDimension.Assign(program);
        mipDimension.BindTo("mipDimension");
    }
};

struct MipmappingVolumeProgram : public ProgramShader
{
    oglplus::Uniform<glm::vec3> mipDimension;
    oglplus::Uniform<int> mipLevel;

    MipmappingVolumeProgram() {}
    ~MipmappingVolumeProgram() override {}

    void ExtractUniforms() override
    {
        mipDimension.Assign(program);
        mipDimension.BindTo("mipDimension");
        mipLevel.Assign(program);
        mipLevel.BindTo("mipLevel");
    }
};