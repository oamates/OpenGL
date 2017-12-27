//=======================================================================================================================================================================================================================
// glsl uniform variable structure methods implementation
//=======================================================================================================================================================================================================================

#include "uniform.hpp"
#include "dsa_uniform.hpp"
#include "log.hpp"

uniform_t::uniform_t(const glsl_program_t& program, const char* name)
    { init(program, name); }

uniform_t::uniform_t(const glsl_shader_program_t& shader_program, const char* name)
    { init(shader_program, name); }

void uniform_t::init(const glsl_program_t& program, const char* name)
{
    program_id = program.id;
    location = glGetUniformLocation(program_id, name);
    debug_msg("Program [%i] uniform [%s] id = [%i]", program_id, name, location);
}

void uniform_t::init(const glsl_shader_program_t& shader_program, const char* name)
{
    program_id = shader_program.id;
    location = glGetUniformLocation(program_id, name);
    debug_msg("Separable shader program [%i] uniform [%s] id = [%i]", program_id, name, location);
}

uniform_t& uniform_t::operator = (const uniform_t& rhs)
{
    program_id = rhs.program_id;
    location = rhs.location;
    return *this;
}

uniform_t& uniform_t::operator = (const dsa_uniform_t& rhs)
{
    program_id = rhs.program_id;
    location = rhs.location;
    return *this;
}
