
#include <fstream>
#include <cstdio>
#include <cstring>
#include "shader1.hpp"
#include "log.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 2048

Shader *Shader::boundShader = 0;


Shader::Shader()
{
    vertexShader    = 0;
    fragmentShader  = 0;
    program         = 0;
    compiled        = 0;

    positionLoc     = -1;
    normalLoc       = -1;
    tangentLoc      = -1;
    texCoordLoc     = -1;
    viewMatrixLoc   = -1;
    projMatrixLoc   = -1;

    vertexFile      = 0;
    fragmentFile    = 0;
}

Shader::Shader(const char *vertFile, const char *fragFile)
{
    vertexShader    = 0;
    fragmentShader  = 0;
    program         = 0;
    compiled        = 0;

    positionLoc     = -1;
    normalLoc       = -1;
    tangentLoc      = -1;
    texCoordLoc     = -1;
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

    fragmentFile = new char[len+1];
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
    glAttachShader(program,vertexShader);
    glAttachShader(program,fragmentShader);
 
    glBindAttribLocation(program, 0, "in_position");
    glBindAttribLocation(program, 1, "in_normal");
    glBindAttribLocation(program, 2, "in_tangent");
    glBindAttribLocation(program, 3, "in_texCoord");

    glBindFragDataLocation(program, 0, "out_frag0");
    glBindFragDataLocation(program, 1, "out_frag1");
    glBindFragDataLocation(program, 2, "out_frag2");
    glBindFragDataLocation(program, 3, "out_frag3");

    glLinkProgram(program);
    printProgramInfoLog(program);
 
    positionLoc = glGetAttribLocation(program,"in_position");
    normalLoc = glGetAttribLocation(program, "in_normal");
    tangentLoc = glGetAttribLocation(program, "in_tangent");
    texCoordLoc = glGetAttribLocation(program, "in_texCoord");

    debug_msg("programId: %i, posLoc %i, normLoc %i, tangLoc %i, texLoc %i", program, positionLoc, normalLoc, tangentLoc, texCoordLoc);
 
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
    boundShader = this;
}

void Shader::unbind()
{
    glUseProgram(0);
    boundShader = 0;
}