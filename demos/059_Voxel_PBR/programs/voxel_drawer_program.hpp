#pragma once
#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

struct VoxelDrawerProgram : public ProgramShader
{
    struct UniformMatrices
    {
        oglplus::Uniform<glm::mat4> modelViewProjection;
    };

	UniformMatrices matrices;
	oglplus::Uniform<unsigned int> volumeDimension;
	std::array<oglplus::Uniform<glm::vec4>, 6> frustumPlanes;
	oglplus::Uniform<float> voxelSize;
	oglplus::Uniform<glm::vec3> worldMinPoint;
	oglplus::Uniform<glm::vec4> colorChannels;

	VoxelDrawerProgram() {}
    ~VoxelDrawerProgram() override {}

    void ExtractUniforms() override
    {        
        volumeDimension.Assign(program);                                        // program owner
        matrices.modelViewProjection.Assign(program);   
        volumeDimension.BindTo("volumeDimension");                              // binding point
        matrices.modelViewProjection.BindTo("matrices.modelViewProjection");

        for(int i = 0; i < frustumPlanes.size(); i++)
        {
            frustumPlanes[i].Assign(program);
            frustumPlanes[i].BindTo("frustumPlanes[" + std::to_string(i) + "]");
        }

        voxelSize.Assign(program);
        voxelSize.BindTo("voxelSize");
        worldMinPoint.Assign(program);
        worldMinPoint.BindTo("worldMinPoint");
        colorChannels.Assign(program);
        colorChannels.BindTo("colorChannels");
    }
};