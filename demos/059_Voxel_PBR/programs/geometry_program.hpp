#pragma once

#include "../types/program_shader.hpp"

#include <oglplus/uniform.hpp>
#include <oglplus/interop/glm.hpp>

/// Contains all necessary uniforms for the DeferredHandler's geometry program for geometry pass.
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
        // fragment shader uniforms
        UniformMaterial material;
        oglplus::Uniform<float> alphaCutoff;
        // vertex shader uniforms
        UniformMatrices matrices;

        void ExtractUniforms() override;

        GeometryProgram() = default;
        virtual ~GeometryProgram();
        GeometryProgram(GeometryProgram const &r) = delete;
        GeometryProgram &operator=(GeometryProgram const &r) = delete;
};
