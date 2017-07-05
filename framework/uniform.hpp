#ifndef _uniform_included_12854961357283548273562875462587340867534896739846347
#define _uniform_included_12854961357283548273562875462587340867534896739846347

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

struct glsl_program_t;

//=======================================================================================================================================================================================================================
// structure representing OpenGL uniform variable
//=======================================================================================================================================================================================================================

struct uniform_t
{
    glsl_program_t* program;
    const char* name;
    GLint location;

    uniform_t() {};
    uniform_t(glsl_program_t* program, const char* name);
    ~uniform_t() {}
    
    operator GLint() const 
        { return location; }

    static void subroutine(GLenum shadertype, const GLuint* indices)
        { glUniformSubroutinesuiv(shadertype, 1, indices); }
    static void subroutines(GLenum shadertype, GLsizei count, const GLuint* indices)
        { glUniformSubroutinesuiv(shadertype, count, indices); }

    //===================================================================================================================================================================================================================
    // integral vectors
    //===================================================================================================================================================================================================================
    void operator = (const GLint arg)
        { glUniform1i(location, arg); }    
    void operator = (const glm::ivec2& arg)
        { glUniform2iv(location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::ivec3& arg)
        { glUniform3iv(location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::ivec4& arg)
        { glUniform4iv(location, 1, glm::value_ptr(arg)); }

    //===================================================================================================================================================================================================================
    // arrays of integral vectors
    //===================================================================================================================================================================================================================
    template <size_t S> void operator = (const GLint (&arg) [S]) 
        { glUniform1iv(location, S, &arg); }
    template <size_t S> void operator = (const glm::ivec2 (&arg) [S]) 
        { glUniform2iv(location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::ivec3 (&arg) [S]) 
        { glUniform3iv(location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::ivec4 (&arg) [S]) 
        { glUniform4iv(location, S, glm::value_ptr(arg[0])); }




    //===================================================================================================================================================================================================================
    // unsigned integral vectors
    //===================================================================================================================================================================================================================
    void operator = (const unsigned int arg)
        { glUniform1ui(location, arg); }
    void operator = (const glm::uvec2& arg)
        { glUniform2uiv(location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::uvec3& arg)
        { glUniform3uiv(location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::uvec4& arg)
        { glUniform4uiv(location, 1, glm::value_ptr(arg)); }

    //===================================================================================================================================================================================================================
    // arrays of unsigned integral vectors
    //===================================================================================================================================================================================================================
    template <size_t S> void operator = (const GLuint (&arg) [S]) 
        { glUniform1uiv(location, S, &arg); }
    template <size_t S> void operator = (const glm::uvec2 (&arg) [S]) 
        { glUniform2uiv(location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::uvec3 (&arg) [S]) 
        { glUniform3uiv(location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::uvec4 (&arg) [S]) 
        { glUniform4uiv(location, S, glm::value_ptr(arg[0])); }




    //===================================================================================================================================================================================================================
    // floating point single-precision vectors
    //===================================================================================================================================================================================================================
    void operator = (const float arg)
        { glUniform1f(location, arg); }
    void operator = (const glm::vec2& arg)
        { glUniform2fv(location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::vec3& arg)
        { glUniform3fv(location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::vec4& arg)
        { glUniform4fv(location, 1, glm::value_ptr(arg)); }

    //===================================================================================================================================================================================================================
    // arrays of floating point single-precision vectors
    //===================================================================================================================================================================================================================
    template <size_t S> void operator = (const float (&arg) [S]) 
        { glUniform1fv(location, S, &arg); }
    template <size_t S> void operator = (const glm::vec2 (&arg) [S]) 
        { glUniform2fv(location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::vec3 (&arg) [S]) 
        { glUniform3fv(location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::vec4 (&arg) [S]) 
        { glUniform4fv(location, S, glm::value_ptr(arg[0])); }




    //===================================================================================================================================================================================================================
    // floating point double-precision vectors
    //===================================================================================================================================================================================================================
    void operator = (const double arg)
        { glUniform1d(location, arg); }
    void operator = (const glm::dvec2& arg)
        { glUniform2dv(location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::dvec3& arg)
        { glUniform3dv(location, 1, glm::value_ptr(arg)); }
    void operator = (const glm::dvec4& arg)
        { glUniform4dv(location, 1, glm::value_ptr(arg)); }

    //===================================================================================================================================================================================================================
    // arrays of floating point double-precision vectors
    //===================================================================================================================================================================================================================
    template <size_t S> void operator = (const double (&arg) [S]) 
        { glUniform1dv(location, S, &arg); }
    template <size_t S> void operator = (const glm::dvec2 (&arg) [S]) 
        { glUniform2dv(location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::dvec3 (&arg) [S]) 
        { glUniform3dv(location, S, glm::value_ptr(arg[0])); }
    template <size_t S> void operator = (const glm::dvec4 (&arg) [S]) 
        { glUniform4dv(location, S, glm::value_ptr(arg[0])); }




    //===================================================================================================================================================================================================================
    // floating point single-precision matrices
    //===================================================================================================================================================================================================================
    void operator = (const glm::mat2& arg)
        { glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::mat2x3& arg)
        { glUniformMatrix2x3fv(location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::mat2x4& arg)
        { glUniformMatrix2x4fv(location, 1, GL_FALSE, glm::value_ptr(arg)); }

    void operator = (const glm::mat3x2& arg)
        { glUniformMatrix3x2fv(location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::mat3& arg)
        { glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::mat3x4& arg)
        { glUniformMatrix3x4fv(location, 1, GL_FALSE, glm::value_ptr(arg)); }

    void operator = (const glm::mat4x2& arg)
        { glUniformMatrix4x2fv(location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::mat4x3& arg)
        { glUniformMatrix4x3fv(location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::mat4& arg)
        { glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(arg)); }


    //===================================================================================================================================================================================================================
    // floating point double-precision matrices
    //===================================================================================================================================================================================================================
    void operator = (const glm::dmat2& arg)
        { glUniformMatrix2dv(location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::dmat2x3& arg)
        { glUniformMatrix2x3dv(location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::dmat2x4& arg)
        { glUniformMatrix2x4dv(location, 1, GL_FALSE, glm::value_ptr(arg)); }

    void operator = (const glm::dmat3x2& arg)
        { glUniformMatrix3x2dv(location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::dmat3& arg)
        { glUniformMatrix3dv(location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::dmat3x4& arg)
        { glUniformMatrix3x4dv(location, 1, GL_FALSE, glm::value_ptr(arg)); }

    void operator = (const glm::dmat4x2& arg)
        { glUniformMatrix4x2dv(location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::dmat4x3& arg)
        { glUniformMatrix4x3dv(location, 1, GL_FALSE, glm::value_ptr(arg)); }
    void operator = (const glm::dmat4& arg)
        { glUniformMatrix4dv(location, 1, GL_FALSE, glm::value_ptr(arg)); }
};

#endif  // _uniform_included_12854961357283548273562875462587340867534896739846347




