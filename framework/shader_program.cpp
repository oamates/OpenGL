//=======================================================================================================================================================================================================================
// separable shader program structure methods implementation
//=======================================================================================================================================================================================================================
#include <memory>

#include "shader_program.hpp"
#include "utils.hpp"
#include "log.hpp"

glsl_shader_program_t::glsl_shader_program_t(const glsl_shader_t& shader)
{
    id = glCreateProgram();
    glAttachShader(id, shader.id);
    glProgramParameteri(id, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glLinkProgram(id);
    GLint linkage_status;
    glGetProgramiv(id, GL_LINK_STATUS, &linkage_status);

    if (GL_TRUE == linkage_status)
    {
        debug_msg("Separable shader program [%d] successfully linked.", id);
        return;
    }

    GLint error_msg_length;
    glGetProgramiv (id, GL_INFO_LOG_LENGTH, &error_msg_length);

    debug_msg("Program [%d] link not successful. Log message length = %d", id, error_msg_length);
    if (error_msg_length)
    {
        char* error_msg = static_cast<char*>(malloc ((size_t) error_msg_length));
        glGetProgramInfoLog (id, error_msg_length, 0, error_msg);
        debug_msg("Program linkage error : ");
        put_msg(error_msg);
        free(error_msg);
    }

    glDeleteProgram(id);
    exit_msg("Aborting program ...");
}

uniform_t glsl_shader_program_t::operator[] (const char* name) const
    { return uniform_t(*this, name); }

glsl_shader_program_t::~glsl_shader_program_t()
    { glDeleteProgram(id); }
