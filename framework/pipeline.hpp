#ifndef _pipeline_included_8562035860256120562035602356120893754620456128045102
#define _pipeline_included_8562035860256120562035602356120893754620456128045102

#define GLEW_STATIC
#include <GL/glew.h>

#include "shader.hpp"
#include "shader_program.hpp"

struct glsl_pipeline_t
{
    GLuint id;

    glsl_pipeline_t()
        { glGenProgramPipelines(1, &id); }

    void bind()
        { glBindProgramPipeline(id); }

    void active_shader_program(const glsl_program_t& program)
        { glActiveShaderProgram(id, program.id); }

    void active_shader_program(const glsl_shader_program_t& shader_program)
        { glActiveShaderProgram(id, shader_program.id); }

    void add_stage(GLbitfield stage_bitmask, const glsl_program_t& program)
        { glUseProgramStages(id, stage_bitmask, program.id); }

    void add_stage(GLbitfield stage_bitmask, const glsl_shader_program_t& shader_program)
        { glUseProgramStages(id, stage_bitmask, shader_program.id); }

    void validate()
        { glValidateProgramPipeline(id); }

    ~glsl_pipeline_t()
        { glDeleteProgramPipelines(1, &id); }
};

#endif // _pipeline_included_8562035860256120562035602356120893754620456128045102
