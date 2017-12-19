//=======================================================================================================================================================================================================================
// glsl uniform variable structure methods implementation
//=======================================================================================================================================================================================================================

#include "uniform.hpp"
#include "log.hpp"

uniform_t::uniform_t(const glsl_program_t* program, const char* name)
{
    uniform_t::name = name;
    location = glGetUniformLocation(program->id, name);
    debug_msg("Program [%i] uniform [%s] id = [%i]", program->id, name, location);
}

uniform_t::uniform_t(const glsl_shader_program_t* shader_program, const char* name)
{
    uniform_t::name = name;
    location = glGetUniformLocation(shader_program->id, name);
    debug_msg("Separable shader program [%i] uniform [%s] id = [%i]", shader_program->id, name, location);
}
