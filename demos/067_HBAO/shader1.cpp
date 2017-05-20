
#include <fstream>
#include <cstdio>
#include <cstring>
#include "shader1.hpp"
#include "log.hpp"
#include "utils.hpp"

Shader::Shader(const char *vertFile, const char *fragFile)
{
    vertexShader    = 0;
    fragmentShader  = 0;
    program         = 0;
    compiled        = 0;

    viewMatrixLoc   = -1;
    projMatrixLoc   = -1;

    vertexFile      = 0;
    fragmentFile    = 0; 

    setVertexFile(vertFile);
    setFragmentFile(fragFile);
    loadAndCompile();
}

Shader::~Shader()
{
    if(vertexFile) delete vertexFile;
    if(fragmentFile) delete fragmentFile;

    glDetachShader(program, fragmentShader);
    glDetachShader(program, vertexShader);

    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteProgram(program);
}

void Shader::setVertexFile(const char* vertFile)
{
    if(!vertFile)
    {
        debug_msg("A null-pointer was passed");
        return;
    }

    if(vertexFile)
        delete vertexFile;

    int len = (int)strlen(vertFile);

    vertexFile = new char[len+1];
    strcpy(vertexFile, vertFile);
    vertexFile[len] = '\0';
}

void Shader::setFragmentFile(const char* fragFile)
{
    if(!fragFile)
    {
        debug_msg("A null-pointer was passed");
        return;
    }

    if(fragmentFile)
        delete fragmentFile;

    int len = (int)strlen(fragFile);

    fragmentFile = new char[len + 1];
    strcpy(fragmentFile, fragFile);
    fragmentFile[len] = '\0';
}

void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
 
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
 
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}
 
void printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
 
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
 
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n", infoLog);
        free(infoLog);
    }
}

bool Shader::loadAndCompile()
{
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
 
    char* vs = utils::file_read(vertexFile);
    char* fs = utils::file_read(fragmentFile);
 
    glShaderSource(vertexShader, 1, &vs, 0);
    glShaderSource(fragmentShader, 1, &fs, 0);
 
    free(vs);
    free(fs);
 
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
 
    printShaderInfoLog(vertexShader);
    printShaderInfoLog(fragmentShader);

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    printProgramInfoLog(program);
 
    projMatrixLoc = glGetUniformLocation(program, "projMatrix");
    viewMatrixLoc = glGetUniformLocation(program, "viewMatrix");
    modelMatrixLoc = glGetUniformLocation(program, "modelMatrix");

    glUseProgram(program);

    char str[10];
    for(int i = 0; i < 8; ++i)
    {
        sprintf(str, "texture%i",i);
        int textureLoc = glGetUniformLocation(program, str);
        if(textureLoc > -1)
            glUniform1i(textureLoc, i);
    }

    glUseProgram(0);

    compiled = true; 
    return true;
}

GLint Shader::getAttributeLocation(const char *att)
{
    return glGetAttribLocation(program, att);
}


GLint Shader::getUniformLocation(const char *uni)
{
    return glGetUniformLocation(program, uni);
}

void Shader::bind()
{
    if(!compiled) return;
    glUseProgram(program);
}

void Shader::unbind()
{
    glUseProgram(0);
}