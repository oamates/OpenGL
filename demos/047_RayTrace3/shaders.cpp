#include <fstream>
#include <sstream>
#include <cstring>

#include "shaders.hpp"

// Issue: longer shaders are so long they don't fit inside the console window.
#define PRINT_SHADER_SRC_ON_ERROR

void loadShader(std::string filename, GLenum shaderType, std::vector<ShaderInfo> & shaders)
{
	std::ifstream file(filename);
	if(!file)
	{
		fprintf(stdout, "Unable to open file %s", filename.c_str());
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

int compileShaderProgram(std::vector<ShaderInfo>& shaders, GLuint& programHandle)
{	
	GLuint pHandle = glCreateProgram();

	std::vector<const GLchar *> shaderSrcPtr;

	for(unsigned int i = 0; i < shaders.size(); i++)
	{		
		shaderSrcPtr.push_back(shaders[i].source.c_str());

		// If it's a shader header, it contains code that should 
		// be added to the next shader that isn't a shader header
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
			fprintf(stdout, "Shader compilation failed!\n");
			for (int filenr = 0; filenr <= i; filenr++)
			{
				fprintf(stdout, "File %i: %s\n", filenr, shaders[filenr].filename.c_str());
			}			

#ifdef PRINT_SHADER_SRC_ON_ERROR
			std::string s;
			unsigned int totalLen = 0;
			for (unsigned int j = 0; j < shaderSrcPtr.size(); j++)
			{
				totalLen += std::strlen(shaderSrcPtr[j]);
			}

			s.resize(totalLen + 1);
			GLsizei length;
			glGetShaderSource(shaderHandle, s.size(), &length, (GLchar *)s.c_str());
			fprintf(stdout, "Failed compiling shader :: \n%s\n", s.c_str());
#endif
			GLint logLen;
			glGetShaderiv( shaderHandle, GL_INFO_LOG_LENGTH, &logLen );
			if( logLen > 0 )
			{
				char * log = (char *)malloc(logLen);
				GLsizei written;
				glGetShaderInfoLog(shaderHandle, logLen, &written, log);
				fprintf(stdout, "Shader log:\n%s", log);
				free(log);				
			}

			std::string errFileName = "errdump_" + shaders[i].filename;
			std::ofstream f(errFileName.c_str());
			if(f.is_open())
			{
				for (auto j = 0; j < shaderSrcPtr.size(); j++)
				{
					f << shaderSrcPtr[j];
				}
				f.close();
				fprintf(stdout, "Shader written to %s\n", errFileName.c_str());
			}
			
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
		fprintf(stdout, "Failed to link shader program!\n" );
		for (unsigned int i = 0; i < shaders.size(); i++)
		{
			fprintf(stdout, "%s\n", shaders[i].filename.c_str());
		}		
		GLint logLen;
		glGetProgramiv(pHandle, GL_INFO_LOG_LENGTH, &logLen);
		if( logLen > 0 )
		{
			char * log = (char *)malloc(logLen);
			GLsizei written;
			glGetProgramInfoLog(pHandle, logLen, &written, log);
			fprintf(stdout, "Program log: \n%s", log);
			free(log);			
		}
		return -2;
	}
	else
	{
		programHandle = pHandle;
		return 0;
	}
}
	