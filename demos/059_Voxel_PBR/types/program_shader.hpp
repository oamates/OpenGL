#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <oglplus/program.hpp>

#include "../make_unique.hpp"

inline std::string SourceFromFile(const std::string& filepath)
{
    std::ifstream file(filepath);
    std::string result((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return result;
}

// Base class for each program shader used in deferred rendering
struct ProgramShader
{
    oglplus::Program program;                                                       // The program shader
    std::vector<std::unique_ptr<oglplus::Shader>> shaders;

    ProgramShader() = default;
    virtual ~ProgramShader() {}

    ProgramShader(const ProgramShader& r) = delete;
    ProgramShader& operator = (const ProgramShader& r) = delete;

    const oglplus::Program& Program() const                                         // Returns the class shader program.
        { return program; }

    void Use() const                                                                // Uses this program.
        { program.Use(); }

    void Link()                                                                     // Links the shader program given all the attached shaders.
        { program.Link(); }

    void AttachShader(oglplus::ShaderType type, const std::string& filepath)        // Attaches a new shader with the given type and source code.
    {
        const std::string& source = SourceFromFile(filepath);
        auto shader = std::make_unique<oglplus::Shader>(type, source);
        shader->Compile();
        program.AttachShader(*shader);
        shaders.push_back(move(shader));
    }

    // Extracts the uniforms associated with this program.
    // The method is meant to be implemented by inheriting classes which represent different shaders.
    virtual void ExtractUniforms() = 0;
};