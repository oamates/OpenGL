#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>

struct Shader
{
    GLuint program;
    GLuint vertexShader, fragmentShader;

    GLint modelMatrixLoc, viewMatrixLoc, projMatrixLoc;

    char *vertexFile, *fragmentFile;
    bool compiled;
    
    Shader(const char *vertFile, const char *fragFile);
    ~Shader();

    bool loadAndCompile();

    void setVertexFile(const char *vertFile);
    void setFragmentFile(const char *fragFile);

    GLint getAttributeLocation(const char *att);
    GLint getUniformLocation(const char *uni);

    GLint getModelMatrixLocation()
        { return modelMatrixLoc; }
    GLint getViewMatrixLocation()
        { return viewMatrixLoc; }
    GLint getProjMatrixLocation()
        { return projMatrixLoc; }

    void bind();
    void unbind();
};

#endif
