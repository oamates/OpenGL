#pragma once

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

#include "../types/program_shader.hpp"
#include "../scene/light.hpp"

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

    VoxelizationProgram() {}
    ~VoxelizationProgram() override {}

    void ExtractUniforms() override
    {
        matrices.model.Assign(program);                                     // program owner
        matrices.normal.Assign(program);
        material.diffuse.Assign(program);
        material.emissive.Assign(program);
        volumeDimension.Assign(program);
        viewProjections[0].Assign(program);
        viewProjections[1].Assign(program);
        viewProjections[2].Assign(program);
        viewProjectionsI[0].Assign(program);
        viewProjectionsI[1].Assign(program);
        viewProjectionsI[2].Assign(program);
        matrices.model.BindTo("matrices.model");                            // binding point
        matrices.normal.BindTo("matrices.normal");
        material.diffuse.BindTo("material.diffuse");
        material.emissive.BindTo("material.emissive");
        volumeDimension.BindTo("volumeDimension");
        viewProjections[0].BindTo("viewProjections[0]");
        viewProjections[1].BindTo("viewProjections[1]");
        viewProjections[2].BindTo("viewProjections[2]");
        viewProjectionsI[0].BindTo("viewProjectionsI[0]");
        viewProjectionsI[1].BindTo("viewProjectionsI[1]");
        viewProjectionsI[2].BindTo("viewProjectionsI[2]");
        worldMinPoint.Assign(program);                                      // from world to voxel space
        worldMinPoint.BindTo("worldMinPoint");
        voxelScale.Assign(program);
        voxelScale.BindTo("voxelScale");
        flagStaticVoxels.Assign(program);
        flagStaticVoxels.BindTo("flagStaticVoxels");
    }
};