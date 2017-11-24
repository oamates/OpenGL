#include <fstream>
#include <sstream>
#include <cstring>

#include "shaders.hpp"

// Issue: longer shaders are so long they don't fit inside the console window.
//#define PRINT_SHADER_SRC_ON_ERROR

void loadShader(std::string filename, GLenum shaderType, std::vector<ShaderInfo> & shaders)
{
	std::ifstream file(filename);
	if(!file)
	{
		printf("Unable to open file %s", filename.c_str());
		return;
	}
	
	std::stringstream buffer;
	buffer << file.rdbuf();
	
	ShaderInfo shaderInfo;
	shaderInfo.source = buffer.str();
	shaderInfo.shaderType = shaderType;
	shaderInfo.filename = filename;
	shaders.push_back(shaderInfo);
}

int q = 0;

int compileShaderProgram(std::vector<ShaderInfo>& shaders, GLuint& programHandle)
{	

	GLuint pHandle = glCreateProgram();

	std::vector<const GLchar *> shaderSrcPtr;

	for(unsigned int i = 0; i < shaders.size(); i++)
	{		
		shaderSrcPtr.push_back(shaders[i].source.c_str());

		if (shaders[i].shaderType == SHADER_HEADER || shaders[i].shaderType == SHADER_VERSION_HEADER) {
			continue;
		}

		GLuint shaderHandle = glCreateShader(shaders[i].shaderType);
		shaders[i].shaderHandle = shaderHandle;		

		glShaderSource(shaderHandle, shaderSrcPtr.size(), shaderSrcPtr.data(), 0);	
		glCompileShader(shaderHandle);

		GLint result;
		glGetShaderiv( shaderHandle, GL_COMPILE_STATUS, &result );
		if( result == GL_FALSE )
		{
			printf("Shader compilation failed!\n");
			for (int filenr = 0; filenr <= i; filenr++)
			{
				printf("File %i: %s\n", filenr, shaders[filenr].filename.c_str());
			}			

			char file_name[128];
			sprintf(file_name, "shader_source%u.txt", q);
			FILE* f = fopen(file_name, "w");

			for(size_t s = 0; s < shaderSrcPtr.size(); ++s)
			    fprintf(f, "@@@ %d @@@:%s", s + 1, shaderSrcPtr[s]);

			fclose(f);

			printf("shader[%u].sourceLength = %u\n", q, (int)shaderSrcPtr.size());

			GLint logLen;
			glGetShaderiv( shaderHandle, GL_INFO_LOG_LENGTH, &logLen );
			if( logLen > 0 )
			{
				char * log = (char *)malloc(logLen);
				GLsizei written;
				glGetShaderInfoLog(shaderHandle, logLen, &written, log);
				printf("Shader log:\n%s", log);
				free(log);				
			}
			
			exit(-1);
			return -1;
		}

		glAttachShader(pHandle, shaderHandle);
		shaderSrcPtr.clear();
	}

	glLinkProgram( pHandle);
	GLint status;
	glGetProgramiv( pHandle, GL_LINK_STATUS, &status );
	if( status == GL_FALSE ) 
	{
		printf("Failed to link shader program!\n" );
		for (unsigned int i = 0; i < shaders.size(); i++)
		{
			printf("%s\n", shaders[i].filename.c_str());
		}		
		GLint logLen;
		glGetProgramiv(pHandle, GL_INFO_LOG_LENGTH, &logLen);
		if( logLen > 0 )
		{
			char * log = (char *)malloc(logLen);
			GLsizei written;
			glGetProgramInfoLog(pHandle, logLen, &written, log);
			printf("Program log: \n%s", log);
			free(log);			
		}
		return -2;
	}
	else
	{
		programHandle = pHandle;
		return 0;
	}
	++q;
}
	