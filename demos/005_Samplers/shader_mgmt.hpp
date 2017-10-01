#ifndef SHADERSMANAGMENT_H
#define SHADERSMANAGMENT_H

#include "GL/glew.h"
#include <string>

std::string loadTextFile(const char *name);
GLuint createShader(const char *fileName, GLuint shaderType, GLuint shaderID=0);
void checkProgramInfos(GLuint programID, GLuint stat);
GLuint createShaderProgram(const char *fileNameVS, const char *fileNameFS, GLuint programID=0);

void setShadersGlobalMacro(const char *macro, int val);
void setShadersGlobalMacro(const char *macro, float val);
void resetShadersGlobalMacros();

#endif
