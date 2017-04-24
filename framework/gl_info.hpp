#ifndef _gl_info_included_51376526031756134171810846531849755265172359657814125
#define _gl_info_included_51376526031756134171810846531849755265172359657814125

//========================================================================================================================================================================================================================
// Module to easily dump OpenGL implementation-dependent information
//========================================================================================================================================================================================================================

#include <cstdio>
#include <GL/glew.h>
#include <glm/glm.hpp>

enum {
    OPENGL_BASIC_INFO = 0x00000001,
    OPENGL_EXTENSIONS_INFO = 0x00000002,
    OPENGL_COMPUTE_SHADER_INFO = 0x00000004,
};

namespace gl_info {

void dump(unsigned int categories)
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

} // namespace gl_info

#endif // _gl_info_included_51376526031756134171810846531849755265172359657814125
