#pragma once
#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

struct InjectRadianceProgram : public ProgramShader
{
    struct UniformLight                                                     // light pass uniforms
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

    InjectRadianceProgram() {}
    ~InjectRadianceProgram() override {}

    void ExtractUniforms() override
    {
        normalWeightedLambert.Assign(program);
        normalWeightedLambert.BindTo("normalWeightedLambert");
        traceShadowHit.Assign(program);
        traceShadowHit.BindTo("traceShadowHit");
        voxelScale.Assign(program);
        voxelScale.BindTo("voxelScale");
        volumeDimension.Assign(program);
        volumeDimension.BindTo("volumeDimension");
        shadowMap.Assign(program);
        exponents.Assign(program);
        lightBleedingReduction.Assign(program);
        lightViewProjection.Assign(program);
        voxelSize.Assign(program);
        shadowMap.BindTo("shadowMap");
        exponents.BindTo("exponents");
        lightBleedingReduction.BindTo("lightBleedingReduction");
        lightViewProjection.BindTo("lightViewProjection");
        voxelSize.BindTo("voxelSize");
        worldMinPoint.Assign(program);
        worldMinPoint.BindTo("worldMinPoint");
        
        lightTypeCount[0].Assign(program);                                  // collections
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
            light.direction.Assign(program);
            light.diffuse.Assign(program);
            light.direction.BindTo("directionalLight[" + index + "].direction");
            light.diffuse.BindTo("directionalLight[" + index + "].diffuse");
            light.shadowingMethod.Assign(program);
            light.shadowingMethod.BindTo("directionalLight[" + index + "].shadowingMethod");
        }
    
        for (auto i = 0; i < pointLight.size(); i++)
        {
            auto &light = pointLight[i];
            auto index = std::to_string(i);
            light.position.Assign(program);
            light.diffuse.Assign(program);
            light.attenuation.constant.Assign(program);
            light.attenuation.linear.Assign(program);
            light.attenuation.quadratic.Assign(program);
            light.position.BindTo("pointLight[" + index + "].position");
            light.diffuse.BindTo("pointLight[" + index + "].diffuse");
            light.attenuation.constant.BindTo("pointLight[" + index + "].attenuation.constant");
            light.attenuation.linear.BindTo("pointLight[" + index + "].attenuation.linear");
            light.attenuation.quadratic.BindTo("pointLight[" + index + "].attenuation.quadratic");
            light.shadowingMethod.Assign(program);
            light.shadowingMethod.BindTo("pointLight[" + index + "].shadowingMethod");
        }
    
        for (auto i = 0; i < spotLight.size(); i++)
        {
            auto &light = spotLight[i];
            auto index = std::to_string(i);
            light.position.Assign(program);
            light.direction.Assign(program);
            light.diffuse.Assign(program);
            light.attenuation.constant.Assign(program);
            light.attenuation.linear.Assign(program);
            light.attenuation.quadratic.Assign(program);
            light.angleInnerCone.Assign(program);
            light.angleOuterCone.Assign(program);
            light.position.BindTo("spotLight[" + index + "].position");
            light.direction.BindTo("spotLight[" + index + "].direction");
            light.diffuse.BindTo("spotLight[" + index + "].diffuse");
            light.attenuation.constant.BindTo("spotLight[" + index + "].attenuation.constant");
            light.attenuation.linear.BindTo("spotLight[" + index + "].attenuation.linear");
            light.attenuation.quadratic.BindTo("spotLight[" + index + "].attenuation.quadratic");
            light.angleInnerCone.BindTo("spotLight[" + index + "].angleInnerCone");
            light.angleOuterCone.BindTo("spotLight[" + index + "].angleOuterCone");
            light.shadowingMethod.Assign(program);
            light.shadowingMethod.BindTo("spotLight[" + index + "].shadowingMethod");
        }
    }        
};

