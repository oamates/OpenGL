#pragma once
#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

struct VoxelizationProgram : public ProgramShader
{
    struct UniformMaterial
    {
        oglplus::Uniform<glm::vec3> diffuse;
        oglplus::Uniform<glm::vec3> emissive;
    };
    struct UniformMatrices
    {
        oglplus::Uniform<glm::mat4> model;
        oglplus::Uniform<glm::mat4> normal;
    };

    UniformMatrices matrices;
    UniformMaterial material;
    oglplus::Uniform<unsigned int> volumeDimension;

    std::array<oglplus::Uniform<glm::mat4>, 3> viewProjections;
    std::array<oglplus::Uniform<glm::mat4>, 3> viewProjectionsI;

    oglplus::Uniform<float> voxelScale;
    oglplus::Uniform<glm::vec3> worldMinPoint;
    oglplus::Uniform<unsigned int> flagStaticVoxels;

    void ExtractUniforms() override;

    VoxelizationProgram() = default;
    virtual ~VoxelizationProgram() {}
};

