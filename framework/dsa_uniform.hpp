#ifndef _dsa_uniform_included_7723567895219363257235679124719815025627512475235
#define _dsa_uniform_included_7723567895219363257235679124719815025627512475235

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "shader_program.hpp"

struct glsl_program_t;
struct glsl_shader_program_t;

//=======================================================================================================================================================================================================================
// structure representing OpenGL Direct State Access uniform variable
// glProgramUniform* family of functions is available since OpenGL 4.1
//=======================================================================================================================================================================================================================

struct dsa_uniform_t
{
    GLuint program_id;
    const char* name;
    GLint location;

    dsa_uniform_t() {}
    ~dsa_uniform_t() {}

    dsa_uniform_t(const glsl_program_t& program, const char* name);
    dsa_uniform_t(const glsl_shader_program_t& shader_program, const char* name);

    void init(const glsl_program_t& program, const char* name);
    void init(const glsl_shader_program_t& shader_program, const char* name);

    operator GLint() const
        { return location; }

    //===================================================================================================================================================================================================================
    // these operation still assume the program object be bound before the call
    //===================================================================================================================================================================================================================
    static void subroutine(GLenum shadertype, const GLuint* indices)
        { glUniformSubroutinesuiv(shadertype, 1, indices); }
    static void subroutines(GLenum shadertype, GLsizei count, const GLuint* indices)
        { glUniformSubroutinesuiv(shadertype, count, indices); }

    //===================================================================================================================================================================================================================
    // integral vectors
    //===================================================================================================================================================================================================================
    void operator = (const GLint arg)
        { glProgramUniform1i(program_id, location, arg); }
    void operator = (const glm::ivec2& arg)
        { glProgramUniform2iv(program_id, location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::ivec3& arg)
        { glProgramUniform3iv(program_id, location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::ivec4& arg)
        { glProgramUniform4iv(program_id, location, 1, glm::value_ptr(arg)); }

    //===================================================================================================================================================================================================================
    // arrays of integral vectors
    //===================================================================================================================================================================================================================
    template <size_t S> void operator = (const GLint (&arg) [S])
        { glProgramUniform1iv(program_id, location, S, &arg); }
    template <size_t S> void operator = (const glm::ivec2 (&arg) [S])
        { glProgramUniform2iv(program_id, location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::ivec3 (&arg) [S])
        { glProgramUniform3iv(program_id, location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::ivec4 (&arg) [S])
        { glProgramUniform4iv(program_id, location, S, glm::value_ptr(arg[0])); }




    //===================================================================================================================================================================================================================
    // unsigned integral vectors
    //===================================================================================================================================================================================================================
    void operator = (const unsigned int arg)
        { glProgramUniform1ui(program_id, location, arg); }
    void operator = (const glm::uvec2& arg)
        { glProgramUniform2uiv(program_id, location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::uvec3& arg)
        { glProgramUniform3uiv(program_id, location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::uvec4& arg)
        { glProgramUniform4uiv(program_id, location, 1, glm::value_ptr(arg)); }

    //===================================================================================================================================================================================================================
    // arrays of unsigned integral vectors
    //===================================================================================================================================================================================================================
    template <size_t S> void operator = (const GLuint (&arg) [S])
        { glProgramUniform1uiv(program_id, location, S, &arg); }
    template <size_t S> void operator = (const glm::uvec2 (&arg) [S])
        { glProgramUniform2uiv(program_id, location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::uvec3 (&arg) [S])
        { glProgramUniform3uiv(program_id, location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::uvec4 (&arg) [S])
        { glProgramUniform4uiv(program_id, location, S, glm::value_ptr(arg[0])); }




    //===================================================================================================================================================================================================================
    // floating point single-precision vectors
    //===================================================================================================================================================================================================================
    void operator = (const float arg)
        { glProgramUniform1f(program_id, location, arg); }
    void operator = (const glm::vec2& arg)
        { glProgramUniform2fv(program_id, location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::vec3& arg)
        { glProgramUniform3fv(program_id, location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::vec4& arg)
        { glProgramUniform4fv(program_id, location, 1, glm::value_ptr(arg)); }

    //===================================================================================================================================================================================================================
    // arrays of floating point single-precision vectors
    //===================================================================================================================================================================================================================
    template <size_t S> void operator = (const float (&arg) [S])
        { glProgramUniform1fv(program_id, location, S, (const GLfloat*) &arg); }
    template <size_t S> void operator = (const glm::vec2 (&arg) [S])
        { glProgramUniform2fv(program_id, location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::vec3 (&arg) [S])
        { glProgramUniform3fv(program_id, location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::vec4 (&arg) [S])
        { glProgramUniform4fv(program_id, location, S, glm::value_ptr(arg[0])); }




    //===================================================================================================================================================================================================================
    // floating point double-precision vectors
    //===================================================================================================================================================================================================================
    void operator = (const double arg)
        { glProgramUniform1d(program_id, location, arg); }
    void operator = (const glm::dvec2& arg)
        { glProgramUniform2dv(program_id, location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::dvec3& arg)
        { glProgramUniform3dv(program_id, location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::dvec4& arg)
        { glProgramUniform4dv(program_id, location, 1, glm::value_ptr(arg)); }

    //===================================================================================================================================================================================================================
    // arrays of floating point double-precision vectors
    //===================================================================================================================================================================================================================
    template <size_t S> void operator = (const double (&arg) [S])
        { glProgramUniform1dv(program_id, location, S, &arg); }
    template <size_t S> void operator = (const glm::dvec2 (&arg) [S])
        { glProgramUniform2dv(program_id, location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::dvec3 (&arg) [S])
        { glProgramUniform3dv(program_id, location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::dvec4 (&arg) [S])
        { glProgramUniform4dv(program_id, location, S, glm::value_ptr(arg[0])); }




    //===================================================================================================================================================================================================================
    // floating point single-precision matrices
    //===================================================================================================================================================================================================================
    void operator = (const glm::mat2& arg)
        { glProgramUniformMatrix2fv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::mat2x3& arg)
        { glProgramUniformMatrix2x3fv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::mat2x4& arg)
        { glProgramUniformMatrix2x4fv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }

    void operator = (const glm::mat3x2& arg)
        { glProgramUniformMatrix3x2fv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::mat3& arg)
        { glProgramUniformMatrix3fv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::mat3x4& arg)
        { glProgramUniformMatrix3x4fv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }

    void operator = (const glm::mat4x2& arg)
        { glProgramUniformMatrix4x2fv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::mat4x3& arg)
        { glProgramUniformMatrix4x3fv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::mat4& arg)
        { glProgramUniformMatrix4fv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }

    //===================================================================================================================================================================================================================
    // arrays of floating point single-precision matrices
    //===================================================================================================================================================================================================================
    template <size_t S> void operator = (const glm::mat2 (&arg) [S])
        { glProgramUniformMatrix2fv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::mat2x3 (&arg) [S])
        { glProgramUniformMatrix2x3fv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::mat2x4 (&arg) [S])
        { glProgramUniformMatrix2x4fv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }

    template <size_t S> void operator = (const glm::mat3x2 (&arg) [S])
        { glProgramUniformMatrix3x2fv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::mat3 (&arg) [S])
        { glProgramUniformMatrix3fv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::mat3x4 (&arg) [S])
        { glProgramUniformMatrix3x4fv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }

    template <size_t S> void operator = (const glm::mat4x2 (&arg) [S])
        { glProgramUniformMatrix4x2fv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::mat4x3 (&arg) [S])
        { glProgramUniformMatrix4x3fv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::mat4 (&arg) [S])
        { glProgramUniformMatrix4fv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }

    //===================================================================================================================================================================================================================
    // floating point double-precision matrices
    //===================================================================================================================================================================================================================
    void operator = (const glm::dmat2& arg)
        { glProgramUniformMatrix2dv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::dmat2x3& arg)
        { glProgramUniformMatrix2x3dv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::dmat2x4& arg)
        { glProgramUniformMatrix2x4dv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }

    void operator = (const glm::dmat3x2& arg)
        { glProgramUniformMatrix3x2dv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::dmat3& arg)
        { glProgramUniformMatrix3dv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::dmat3x4& arg)
        { glProgramUniformMatrix3x4dv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }

    void operator = (const glm::dmat4x2& arg)
        { glProgramUniformMatrix4x2dv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::dmat4x3& arg)
        { glProgramUniformMatrix4x3dv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::dmat4& arg)
        { glProgramUniformMatrix4dv(program_id, location, 1, GL_FALSE, glm::value_ptr(arg)); }

    //===================================================================================================================================================================================================================
    // arrays of floating point double-precision matrices
    //===================================================================================================================================================================================================================
    template <size_t S> void operator = (const glm::dmat2 (&arg) [S])
        { glProgramUniformMatrix2dv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::dmat2x3 (&arg) [S])
        { glProgramUniformMatrix2x3dv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::dmat2x4 (&arg) [S])
        { glProgramUniformMatrix2x4dv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }

    template <size_t S> void operator = (const glm::dmat3x2 (&arg) [S])
        { glProgramUniformMatrix3x2dv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::dmat3 (&arg) [S])
        { glProgramUniformMatrix3dv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::dmat3x4 (&arg) [S])
        { glProgramUniformMatrix3x4dv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }

    template <size_t S> void operator = (const glm::dmat4x2 (&arg) [S])
        { glProgramUniformMatrix4x2dv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::dmat4x3 (&arg) [S])
        { glProgramUniformMatrix4x3dv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::dmat4 (&arg) [S])
        { glProgramUniformMatrix4dv(program_id, location, S, GL_FALSE, glm::value_ptr(arg[0])); }

};

#endif  // _dsa_uniform_included_7723567895219363257235679124719815025627512475235
