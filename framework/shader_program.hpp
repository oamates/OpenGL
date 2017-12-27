#ifndef _shader_program_included_8346058632056932586403986192836419826589234542
#define _shader_program_included_8346058632056932586403986192836419826589234542

#define GLEW_STATIC
#include <GL/glew.h>

#include "shader.hpp"
#include "uniform.hpp"
#include "dsa_uniform.hpp"

struct glsl_shader_t;
struct uniform_t;

struct glsl_shader_program_t
{
    GLuint id;

    glsl_shader_program_t(const glsl_shader_t& shader);
    ~glsl_shader_program_t();

    uniform_t operator[] (const char* name) const;
};

#endif // _shader_program_included_8346058632056932586403986192836419826589234542
