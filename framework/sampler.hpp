#ifndef _sampler_included_74356284375682347658436583465873467563458763485764358
#define _sampler_included_74356284375682347658436583465873467563458763485764358

//========================================================================================================================================================================================================================
// glSamplerParameteri valid pname arguments and their values
//========================================================================================================================================================================================================================
//
// *** GL_TEXTURE_MIN_FILTER ***
//  -- GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR
//
// *** GL_TEXTURE_MAG_FILTER ***
//  -- GL_NEAREST, GL_LINEAR
//
// *** GL_TEXTURE_WRAP_S / GL_TEXTURE_WRAP_T / GL_TEXTURE_WRAP_R ***
//  -- GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, GL_REPEAT, GL_CLAMP_TO_BORDER, GL_MIRROR_CLAMP_TO_EDGE ( gl >= 4.4)
//
// *** GL_TEXTURE_COMPARE_MODE ***
//  -- GL_COMPARE_REF_TO_TEXTURE, GL_NONE
//
// *** GL_TEXTURE_COMPARE_FUNC ***
//  -- GL_LEQUAL, GL_GEQUAL, GL_LESS, GL_GREATER, GL_EQUAL, GL_NOTEQUAL, GL_ALWAYS, GL_NEVER

//========================================================================================================================================================================================================================
// glSamplerParameterf valid pname arguments and their values
//========================================================================================================================================================================================================================
//
// *** GL_TEXTURE_MIN_LOD ***
//  Sets the minimum level-of-detail parameter, which limits the selection of the lowest mipmap level
//  The initial value is -1000.
//
// *** GL_TEXTURE_MAX_LOD ***
//  Sets the maximum level-of-detail parameter, which limits the selection of the highest mipmap level
//  The initial value is 1000.

struct sampler_t
{
    GLuint id;

    sampler_t(GLint mag_filter, GLint min_filter, GLint wrap_mode)
    {
        glGenSamplers(1, &id);
        glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, mag_filter);
        glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, min_filter);
        glSamplerParameteri(id, GL_TEXTURE_WRAP_S, wrap_mode);
        glSamplerParameteri(id, GL_TEXTURE_WRAP_T, wrap_mode);
    }

    sampler_t(GLint mag_filter, GLint min_filter, GLint wrap_mode_s, GLint wrap_mode_t)
    {
        glGenSamplers(1, &id);
        glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, mag_filter);
        glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, min_filter);
        glSamplerParameteri(id, GL_TEXTURE_WRAP_S, wrap_mode_s);
        glSamplerParameteri(id, GL_TEXTURE_WRAP_T, wrap_mode_t);
    }

    sampler_t(GLint mag_filter, GLint min_filter, GLint wrap_mode_s, GLint wrap_mode_t, GLint wrap_mode_r)
    {
        glGenSamplers(1, &id);
        glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, mag_filter);
        glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, min_filter);
        glSamplerParameteri(id, GL_TEXTURE_WRAP_S, wrap_mode_s);
        glSamplerParameteri(id, GL_TEXTURE_WRAP_T, wrap_mode_t);
        glSamplerParameteri(id, GL_TEXTURE_WRAP_R, wrap_mode_r);
    }

    void bind(GLuint unit)
        { glBindSampler(unit, id); }

    void set_parameteri(GLenum pname, GLint param)
        { glSamplerParameteri(id, pname, param); }

    void set_parameterf(GLenum pname, GLfloat param)
        { glSamplerParameterf(id, pname, param); }

    void set_parameteriv(GLenum pname, const GLint* param)
        { glSamplerParameteriv(id, pname, param); }

    void set_parameterfv(GLenum pname, const GLfloat* param)
        { glSamplerParameterfv(id, pname, param); }

    void set_max_af_level(GLfloat max_af_level)
        { set_parameterf(GL_TEXTURE_MAX_ANISOTROPY_EXT, max_af_level); }

    ~sampler_t()
    {
        glDeleteSamplers(1, &id);
    }
};

#endif // _sampler_included_74356284375682347658436583465873467563458763485764358
