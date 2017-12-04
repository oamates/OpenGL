#include <fstream>
#include <GL/glew.h>

#include "shader.hpp"

GLuint ShaderLoader::loadShaders(const char* vertex_file_path, const char* geometry_file_path, const char* fragment_file_path)
{
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint GeometryShaderID;

    if (geometry_file_path)
        GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
  
    std::string GeometryShaderCode;
    if (geometry_file_path)
    {
        std::ifstream GeomatryShaderStream(geometry_file_path, std::ios::in);
        if(GeomatryShaderStream.is_open())
        {
            std::string Line = "";
            while(getline(GeomatryShaderStream, Line))
                GeometryShaderCode += "\n" + Line;
            GeomatryShaderStream.close();
        }
    }

    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
  
    GLint Result = GL_FALSE;
    int InfoLogLength;
  
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
  
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    if (geometry_file_path)
    {
        printf("Compiling shader : %s\n", geometry_file_path);
        char const * GeometrySourcePointer = GeometryShaderCode.c_str();
        glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer , NULL);
        glCompileShader(GeometryShaderID);
        
        glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        std::vector<char> GeometryShaderErrorMessage(InfoLogLength);
        glGetShaderInfoLog(GeometryShaderID, InfoLogLength, NULL, &GeometryShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &GeometryShaderErrorMessage[0]);
    }

    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
  
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
  
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    if (geometry_file_path)
        glAttachShader(ProgramID, GeometryShaderID);
    glAttachShader(ProgramID, FragmentShaderID);

    glLinkProgram(ProgramID);
  
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( std::max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
  
    glDeleteShader(VertexShaderID);
    if (geometry_file_path)
        glDeleteShader(GeometryShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}