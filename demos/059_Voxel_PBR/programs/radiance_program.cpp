#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "radiance_program.hpp"
#include "../scene/light.hpp"

void InjectRadianceProgram::ExtractUniforms()
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
        light.attenuation.constant.BindTo("pointLight[" + index +
                                          "].attenuation.constant");
        light.attenuation.linear.BindTo("pointLight[" + index +
                                        "].attenuation.linear");
        light.attenuation.quadratic.BindTo("pointLight[" + index +
                                           "].attenuation.quadratic");
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
        light.attenuation.constant.BindTo("spotLight[" + index +
                                          "].attenuation.constant");
        light.attenuation.linear.BindTo("spotLight[" + index +
                                        "].attenuation.linear");
        light.attenuation.quadratic.BindTo("spotLight[" + index +
                                           "].attenuation.quadratic");
        light.angleInnerCone.BindTo("spotLight[" + index + "].angleInnerCone");
        light.angleOuterCone.BindTo("spotLight[" + index + "].angleOuterCone");
        light.shadowingMethod.Assign(program);
        light.shadowingMethod.BindTo("spotLight[" + index + "].shadowingMethod");
    }
}