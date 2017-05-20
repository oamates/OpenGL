#ifndef SHADER_H
#define SHADER_H

//#include <glm/glm.hpp>
#include <GL/glew.h>

struct Shader
{
    GLuint program;
    GLuint vertexShader, fragmentShader;

    GLint positionLoc, normalLoc, tangentLoc, texCoordLoc;
    GLint modelMatrixLoc, viewMatrixLoc, projMatrixLoc;

    char *vertexFile, *fragmentFile;
    bool compiled;

    static Shader *boundShader;
    
    Shader();
    Shader(const char *vertFile, const char *fragFile);
    ~Shader();

    static Shader *getBoundShader()
        { return boundShader; }

    bool loadAndCompile();

    void setVertexFile(const char *vertFile);
    void setFragmentFile(const char *fragFile);

    const char *getVertexFile()
        { return vertexFile; }
    const char *getFragmentFile()
        { return fragmentFile; }

    GLint getAttributeLocation(const char *att);
    GLint getUniformLocation(const char *uni);

    GLint getPositionLocation()
        { return positionLoc; }
    GLint getNormalLocation()
        { return normalLoc; }
    GLint getTangentLocation()
        { return tangentLoc; }
    GLint getTexCoordLocation()
        { return texCoordLoc; }

    GLint getModelMatrixLocation()
        { return modelMatrixLoc; }
    GLint getViewMatrixLocation()
        { return viewMatrixLoc; }
    GLint getProjMatrixLocation()
        { return projMatrixLoc; }

    bool isCompiled() { return compiled; }

    void bind();
    void unbind();
};

#endif
