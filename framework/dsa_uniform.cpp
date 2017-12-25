//=======================================================================================================================================================================================================================
// glsl uniform variable structure methods implementation
//=======================================================================================================================================================================================================================

#include "dsa_uniform.hpp"
#include "log.hpp"

dsa_uniform_t::dsa_uniform_t(const glsl_program_t& program, const char* name)
    { init(program, name); }

dsa_uniform_t::dsa_uniform_t(const glsl_shader_program_t& shader_program, const char* name)
    { init(shader_program, name); }

void dsa_uniform_t::init(const glsl_program_t& program, const char* name)
{
    program_id = program.id;
    dsa_uniform_t::name = name;
    location = glGetUniformLocation(program_id, name);
    debug_msg("DSA program [%i] uniform [%s] id = [%i]", program_id, name, location);
}

void dsa_uniform_t::init(const glsl_shader_program_t& shader_program, const char* name)
{
    program_id = shader_program.id;
    dsa_uniform_t::name = name;
    location = glGetUniformLocation(program_id, name);
    debug_msg("DSA separable shader program [%i] uniform [%s] id = [%i]", program_id, name, location);
}
