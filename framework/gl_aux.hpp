#ifndef _gl_info_included_51376526031756134171810846531849755265172359657814125
#define _gl_info_included_51376526031756134171810846531849755265172359657814125

//========================================================================================================================================================================================================================
// Module to easily dump OpenGL implementation-dependent information
//========================================================================================================================================================================================================================

#include <cstdio>

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>

enum {
    OPENGL_BASIC_INFO = 0x00000001,
    OPENGL_EXTENSIONS_INFO = 0x00000002,
    OPENGL_COMPUTE_SHADER_INFO = 0x00000004,
};

namespace gl_aux {

void dump_info(unsigned int categories)
{
    if (categories & OPENGL_BASIC_INFO)
    {
        printf("GL_VENDOR = %s\n", glGetString(GL_VENDOR));
        printf("GL_RENDERER = %s\n", glGetString(GL_RENDERER));
        printf("GL_VERSION = %s\n", glGetString(GL_VERSION));
        printf("GL_SHADING_LANGUAGE_VERSION = %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    }

    if (categories & OPENGL_EXTENSIONS_INFO)
    {
        GLint glNumExtensions;
        glGetIntegerv(GL_NUM_EXTENSIONS, &glNumExtensions);
        printf("GL_NUM_EXTENSIONS = %d\n", glNumExtensions);
        for (GLint i = 0; i < glNumExtensions; ++i)
            printf("\t#%d : %s\n", i, glGetStringi(GL_EXTENSIONS, i));
    }

    if (categories & OPENGL_COMPUTE_SHADER_INFO)
    {
        GLint glMaxComputeWorkGroupInvocations;
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &glMaxComputeWorkGroupInvocations);
        printf("GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS = %d\n", glMaxComputeWorkGroupInvocations);

        glm::ivec3 glMaxComputeWorkGroupCount;
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &glMaxComputeWorkGroupCount.x);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &glMaxComputeWorkGroupCount.y);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &glMaxComputeWorkGroupCount.z);
        printf("GL_MAX_COMPUTE_WORK_GROUP_COUNT = %d x %d x %d\n", glMaxComputeWorkGroupCount.x, glMaxComputeWorkGroupCount.y, glMaxComputeWorkGroupCount.z);

        glm::ivec3 glMaxComputeWorkGroupSize;
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &glMaxComputeWorkGroupSize.x);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &glMaxComputeWorkGroupSize.y);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &glMaxComputeWorkGroupSize.z);
        printf("GL_MAX_COMPUTE_WORK_GROUP_SIZE = %d x %d x %d\n", glMaxComputeWorkGroupSize.x, glMaxComputeWorkGroupSize.y, glMaxComputeWorkGroupSize.z);
   }
}

const char* internal_format_name(GLint format)
{
    switch (format)
    {
        /* Base Internal Formats */
        case GL_DEPTH_COMPONENT : return "GL_DEPTH_COMPONENT";
        case GL_DEPTH_STENCIL :   return "GL_DEPTH_STENCIL";
        case GL_RED :             return "GL_RED";
        case GL_RG :              return "GL_RG";
        case GL_RGB :             return "GL_RGB";
        case GL_RGBA :            return "GL_RGBA";

        /* Sized Internal Formats */
        case GL_R8 :              return "GL_R8";
        case GL_R8_SNORM :        return "GL_R8_SNORM";
        case GL_R16 :             return "GL_R16";
        case GL_R16_SNORM :       return "GL_R16_SNORM";
        case GL_RG8 :             return "GL_RG8";
        case GL_RG8_SNORM :       return "GL_RG8_SNORM";
        case GL_RG16 :            return "GL_RG16";
        case GL_RG16_SNORM :      return "GL_RG16_SNORM";
        case GL_R3_G3_B2 :        return "GL_R3_G3_B2";
        case GL_RGB4 :            return "GL_RGB4";
        case GL_RGB5 :            return "GL_RGB5";
        case GL_RGB8 :            return "GL_RGB8";
        case GL_RGB8_SNORM :      return "GL_RGB8_SNORM";
        case GL_RGB10 :           return "GL_RGB10";
        case GL_RGB12 :           return "GL_RGB12";
        case GL_RGB16_SNORM :     return "GL_RGB16_SNORM";
        case GL_RGBA2 :           return "GL_RGBA2";
        case GL_RGBA4 :           return "GL_RGBA4";
        case GL_RGB5_A1 :         return "GL_RGB5_A1";
        case GL_RGBA8 :           return "GL_RGBA8";
        case GL_RGBA8_SNORM :     return "GL_RGBA8_SNORM";
        case GL_RGB10_A2 :        return "GL_RGB10_A2";
        case GL_RGB10_A2UI :      return "GL_RGB10_A2UI";
        case GL_RGBA12 :          return "GL_RGBA12";
        case GL_RGBA16 :          return "GL_RGBA16";
        case GL_SRGB8 :           return "GL_SRGB8";
        case GL_SRGB8_ALPHA8 :    return "GL_SRGB8_ALPHA8";
        case GL_R16F :            return "GL_R16F";
        case GL_RG16F :           return "GL_RG16F";
        case GL_RGB16F :          return "GL_RGB16F";
        case GL_RGBA16F :         return "GL_RGBA16F";
        case GL_R32F :            return "GL_R32F";
        case GL_RG32F :           return "GL_RG32F";
        case GL_RGB32F :          return "GL_RGB32F";
        case GL_RGBA32F :         return "GL_RGBA32F";
        case GL_R11F_G11F_B10F :  return "GL_R11F_G11F_B10F";
        case GL_RGB9_E5 :         return "GL_RGB9_E5";
        case GL_R8I :             return "GL_R8I";
        case GL_R8UI :            return "GL_R8UI";
        case GL_R16I :            return "GL_R16I";
        case GL_R16UI :           return "GL_R16UI";
        case GL_R32I :            return "GL_R32I";
        case GL_R32UI :           return "GL_R32UI";
        case GL_RG8I :            return "GL_RG8I";
        case GL_RG8UI :           return "GL_RG8UI";
        case GL_RG16I :           return "GL_RG16I";
        case GL_RG16UI :          return "GL_RG16UI";
        case GL_RG32I :           return "GL_RG32I";
        case GL_RG32UI :          return "GL_RG32UI";
        case GL_RGB8I :           return "GL_RGB8I";
        case GL_RGB8UI :          return "GL_RGB8UI";
        case GL_RGB16I :          return "GL_RGB16I";
        case GL_RGB16UI :         return "GL_RGB16UI";
        case GL_RGB32I :          return "GL_RGB32I";
        case GL_RGB32UI :         return "GL_RGB32UI";
        case GL_RGBA8I :          return "GL_RGBA8I";
        case GL_RGBA8UI :         return "GL_RGBA8UI";
        case GL_RGBA16I :         return "GL_RGBA16I";
        case GL_RGBA16UI :        return "GL_RGBA16UI";
        case GL_RGBA32I :         return "GL_RGBA32I";
        case GL_RGBA32UI :        return "GL_RGBA32UI";
    }
    return "Unknown";
}

} // namespace gl_info

#endif // _gl_info_included_51376526031756134171810846531849755265172359657814125
