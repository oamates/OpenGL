#pragma once

#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

#include "../scene/light.hpp"

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

    LightingProgram() {}
    ~LightingProgram() override {}

    void ExtractUniforms() override
    {
        // assign program
        inverseProjectionView.Assign(program);
        lightViewProjection.Assign(program);
        exponents.Assign(program);
        lightBleedingReduction.Assign(program);
        // bind to uniform name
        inverseProjectionView.BindTo("inverseProjectionView");
        lightViewProjection.BindTo("lightViewProjection");
        exponents.BindTo("exponents");
        lightBleedingReduction.BindTo("lightBleedingReduction");
        // collections
        lightTypeCount[0].Assign(program);
        lightTypeCount[1].Assign(program);
        lightTypeCount[2].Assign(program);
        lightTypeCount[0].BindTo("lightTypeCount[0]");
        lightTypeCount[1].BindTo("lightTypeCount[1]");
        lightTypeCount[2].BindTo("lightTypeCount[2]");
        directionalLight.resize(Light::DirectionalsLimit);
        pointLight.resize(Light::PointsLimit);
        spotLight.resize(Light::SpotsLimit);

        for (auto i = 0; i < directionalLight.size(); i++)
        {
            auto &light = directionalLight[i];
            auto index = std::to_string(i);
            light.shadowingMethod.Assign(program);
            light.shadowingMethod.BindTo("directionalLight[" + index + "].shadowingMethod");
            light.direction.Assign(program);
            light.ambient.Assign(program);
            light.diffuse.Assign(program);
            light.specular.Assign(program);
            light.direction.BindTo("directionalLight[" + index + "].direction");
            light.ambient.BindTo("directionalLight[" + index + "].ambient");
            light.diffuse.BindTo("directionalLight[" + index + "].diffuse");
            light.specular.BindTo("directionalLight[" + index + "].specular");
        }

        for (auto i = 0; i < pointLight.size(); i++)
        {
            auto &light = pointLight[i];
            auto index = std::to_string(i);
            light.shadowingMethod.Assign(program);
            light.shadowingMethod.BindTo("pointLight[" + index + "].shadowingMethod");
            light.position.Assign(program);
            light.ambient.Assign(program);
            light.diffuse.Assign(program);
            light.specular.Assign(program);
            light.attenuation.constant.Assign(program);
            light.attenuation.linear.Assign(program);
            light.attenuation.quadratic.Assign(program);
            light.position.BindTo("pointLight[" + index + "].position");
            light.ambient.BindTo("pointLight[" + index + "].ambient");
            light.diffuse.BindTo("pointLight[" + index + "].diffuse");
            light.specular.BindTo("pointLight[" + index + "].specular");
            light.attenuation.constant.BindTo("pointLight[" + index + "].attenuation.constant");
            light.attenuation.linear.BindTo("pointLight[" + index + "].attenuation.linear");
            light.attenuation.quadratic.BindTo("pointLight[" + index + "].attenuation.quadratic");
        }

        for (auto i = 0; i < spotLight.size(); i++)
        {
            auto &light = spotLight[i];
            auto index = std::to_string(i);
            light.shadowingMethod.Assign(program);
            light.shadowingMethod.BindTo("spotLight[" + index + "].shadowingMethod");
            light.position.Assign(program);
            light.direction.Assign(program);
            light.ambient.Assign(program);
            light.diffuse.Assign(program);
            light.specular.Assign(program);
            light.attenuation.constant.Assign(program);
            light.attenuation.linear.Assign(program);
            light.attenuation.quadratic.Assign(program);
            light.angleInnerCone.Assign(program);
            light.angleOuterCone.Assign(program);
            light.position.BindTo("spotLight[" + index + "].position");
            light.direction.BindTo("spotLight[" + index + "].direction");
            light.ambient.BindTo("spotLight[" + index + "].ambient");
            light.diffuse.BindTo("spotLight[" + index + "].diffuse");
            light.specular.BindTo("spotLight[" + index + "].specular");
            light.attenuation.constant.BindTo("spotLight[" + index + "].attenuation.constant");
            light.attenuation.linear.BindTo("spotLight[" + index + "].attenuation.linear");
            light.attenuation.quadratic.BindTo("spotLight[" + index + "].attenuation.quadratic");
            light.angleInnerCone.BindTo("spotLight[" + index + "].angleInnerCone");
            light.angleOuterCone.BindTo("spotLight[" + index + "].angleOuterCone");
        }

        cameraPosition.Assign(program);
        cameraPosition.BindTo("cameraPosition");
        // voxel volume setup
        volumeDimension.Assign(program);
        volumeDimension.BindTo("volumeDimension");
        // from world to voxel space
        worldMinPoint.Assign(program);
        worldMinPoint.BindTo("worldMinPoint");
        worldMaxPoint.Assign(program);
        worldMaxPoint.BindTo("worldMaxPoint");
        voxelScale.Assign(program);
        voxelScale.BindTo("voxelScale");
        // gi options
        maxTracingDistanceGlobal.Assign(program);
        maxTracingDistanceGlobal.BindTo("maxTracingDistanceGlobal");
        bounceStrength.Assign(program);
        bounceStrength.BindTo("bounceStrength");
        aoFalloff.Assign(program);
        aoFalloff.BindTo("aoFalloff");
        aoAlpha.Assign(program);
        aoAlpha.BindTo("aoAlpha");
        mode.Assign(program);
        mode.BindTo("mode");
        samplingFactor.Assign(program);
        samplingFactor.BindTo("samplingFactor");
        coneShadowTolerance.Assign(program);
        coneShadowTolerance.BindTo("coneShadowTolerance");
        coneShadowAperture.Assign(program);
        coneShadowAperture.BindTo("coneShadowAperture");
    }

    LightingProgram(LightingProgram const &r) = delete;
    LightingProgram &operator=(LightingProgram const &r) = delete;
};