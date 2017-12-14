#pragma once

#include <oglplus/program.hpp>

// Base class for each program shader used in deferred rendering
struct ProgramShader
{
    oglplus::Program program;                                                       // The program shader
    std::vector<std::unique_ptr<oglplus::Shader>> shaders;

    ProgramShader() = default;
    virtual ~ProgramShader() {}
    ProgramShader(ProgramShader const& r) = delete;
    ProgramShader &operator=(ProgramShader const& r) = delete;

    const oglplus::Program &Program() const;                                        // Returns the class shader program.

    void Use() const;                                                               // Uses this program.
    void Link();                                                                    // Links the shader program given all the attached shaders.
    void AttachShader(oglplus::ShaderType type, const std::string &filepath);       // Attaches a new shader with the given type and source code.

    // Extracts the uniforms associated with this program.
    // The method is meant to be implemented by inheriting classes which represent different shaders.
    virtual void ExtractUniforms() = 0;
};
