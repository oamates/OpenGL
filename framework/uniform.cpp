
#include "uniform.hpp"
#include "log.hpp"

uniform_t::uniform_t(glsl_program_t& program, const char* name) : program(program)
{
    uniform_t::name = name;
    location = glGetUniformLocation(program.id, name);
    debug_msg("Program [%i] uniform [%s] id = [%i]", program.id, name, location);
}