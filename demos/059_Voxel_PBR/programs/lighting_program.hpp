#pragma once

#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

// Contains all necessary uniforms for the DeferredHandler's lighting program for light pass.
struct LightingProgram : public ProgramShader
{
        // light pass uniforms
        struct UniformLight
        {
            oglplus::Uniform<float> angleInnerCone;
            oglplus::Uniform<float> angleOuterCone;

            oglplus::Uniform<glm::vec3> ambient;
            oglplus::Uniform<glm::vec3> diffuse;
            oglplus::Uniform<glm::vec3> specular;

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

        // fragment shader uniforms
        oglplus::Uniform<glm::mat4x4> inverseProjectionView;
        oglplus::Uniform<glm::mat4x4> lightViewProjection;


        std::vector<UniformLight> directionalLight;
        std::vector<UniformLight> pointLight;
        std::vector<UniformLight> spotLight;
        std::array<oglplus::Uniform<unsigned int>, 3> lightTypeCount;

        oglplus::Uniform<glm::vec3> cameraPosition;

        oglplus::Uniform<glm::vec2> exponents;
        oglplus::Uniform<float> lightBleedingReduction;

        oglplus::Uniform<int> volumeDimension;
        oglplus::Uniform<float> voxelScale;
        oglplus::Uniform<glm::vec3> worldMinPoint;
        oglplus::Uniform<glm::vec3> worldMaxPoint;

        oglplus::Uniform<float> maxTracingDistanceGlobal;
        oglplus::Uniform<float> bounceStrength;
        oglplus::Uniform<float> aoFalloff;
        oglplus::Uniform<float> aoAlpha;
        oglplus::Uniform<float> samplingFactor;
        oglplus::Uniform<float> coneShadowTolerance;
        oglplus::Uniform<float> coneShadowAperture;
        oglplus::Uniform<unsigned int> mode;

        void ExtractUniforms() override;

        LightingProgram() = default;
        virtual ~LightingProgram();
        LightingProgram(LightingProgram const &r) = delete;
        LightingProgram &operator=(LightingProgram const &r) = delete;
};

