#pragma once

#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

// Contains all necessary uniforms for the DeferredHandler's geometry program for geometry pass.
struct GeometryProgram : public ProgramShader
{
    struct UniformMaterial
    {
        oglplus::Uniform<glm::vec3> ambient;
        oglplus::Uniform<glm::vec3> diffuse;
        oglplus::Uniform<glm::vec3> specular;
        oglplus::Uniform<glm::vec3> emissive;
        oglplus::Uniform<float> shininess;
        oglplus::Uniform<unsigned int> useNormalsMap;
    };

    struct UniformMatrices
    {
        oglplus::Uniform<glm::mat4> normal;
        oglplus::Uniform<glm::mat4> modelViewProjection;
    };
        
    UniformMaterial material;                                       // fragment shader uniforms
    oglplus::Uniform<float> alphaCutoff;        
    UniformMatrices matrices;                                       // vertex shader uniforms

    GeometryProgram() {}
    ~GeometryProgram() override {}

    void ExtractUniforms() override
    {
        material.diffuse.Assign(program);                           // assign program
        material.specular.Assign(program);
        material.shininess.Assign(program);
        material.emissive.Assign(program);
        material.useNormalsMap.Assign(program);
        alphaCutoff.Assign(program);
        matrices.normal.Assign(program);
        matrices.modelViewProjection.Assign(program);
        material.diffuse.BindTo("material.diffuse");                // bind to uniform name
        material.specular.BindTo("material.specular");
        material.emissive.BindTo("material.emissive");
        material.shininess.BindTo("material.shininess");
        material.useNormalsMap.BindTo("material.useNormalsMap");
        alphaCutoff.BindTo("alphaCutoff");
        matrices.normal.BindTo("matrices.normal");
        matrices.modelViewProjection.BindTo("matrices.modelViewProjection");
    }

    GeometryProgram(GeometryProgram const &r) = delete;
    GeometryProgram &operator=(GeometryProgram const &r) = delete;
};