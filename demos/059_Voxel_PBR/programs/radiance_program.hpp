#pragma once
#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

struct InjectRadianceProgram : public ProgramShader
{
        // light pass uniforms
        struct UniformLight
        {
            oglplus::Uniform<float> angleInnerCone;
            oglplus::Uniform<float> angleOuterCone;

            oglplus::Uniform<glm::vec3> diffuse;

            oglplus::Uniform<glm::vec3> position;
            oglplus::Uniform<glm::vec3> direction;

            struct UniformAttenuation
            {
                oglplus::Uniform<float> constant;
                oglplus::Uniform<float> linear;
                oglplus::Uniform<float> quadratic;
            };

            oglplus::Uniform<unsigned int> shadowingMethod;
            UniformAttenuation attenuation;
        };

        InjectRadianceProgram() {}
        ~InjectRadianceProgram() {}

		std::vector<UniformLight> directionalLight;
        std::vector<UniformLight> pointLight;
        std::vector<UniformLight> spotLight;
        std::array<oglplus::Uniform<unsigned int>, 3> lightTypeCount;
        oglplus::Uniform<glm::mat4x4> lightViewProjection;
        oglplus::UniformSampler shadowMap;
        oglplus::Uniform<glm::vec2> exponents;
        oglplus::Uniform<float> lightBleedingReduction;
        oglplus::Uniform<float> voxelSize;
        oglplus::Uniform<glm::vec3> worldMinPoint;
        oglplus::Uniform<int> volumeDimension;
        oglplus::Uniform<float> voxelScale;
        oglplus::Uniform<float> traceShadowHit;
        oglplus::Uniform<unsigned int> normalWeightedLambert;

        void ExtractUniforms() override;
};

