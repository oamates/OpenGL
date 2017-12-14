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
	void ExtractUniforms() override;
	VoxelDrawerProgram() = default;
    virtual ~VoxelDrawerProgram();
};