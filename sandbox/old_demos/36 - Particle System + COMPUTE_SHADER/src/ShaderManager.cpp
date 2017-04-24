#include "shaderManager.h"

std::string	ShaderManager::getFileContents(const std::string& filename) const
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (!in) throw std::runtime_error("Could not open file: "+ filename+ "!");
	std::string contents;
	in.seekg(0, std::ios::end);
	contents.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&contents[0], contents.size());
	in.close();
	return(contents);
}

void ShaderManager::loadShader(const std::string& filename, const std::string& shaderKey, const GLenum type)
{

	std::string shaderCode = getFileContents(filename);

	GLuint shaderID = glCreateShader(type);
	const char* source = shaderCode.c_str();

	if(shaderID != 0)
	{
		_shaderData.saveShader(shaderKey, shaderID);
		glShaderSource(shaderID, 1, &source, NULL);
		glCompileShader(shaderID);
	}

	GLint compileStatus;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileStatus);

	if(glGetError() != GL_NO_ERROR || compileStatus == GL_FALSE){

		if(shaderID != 0){
			_shaderData.deleteShader(shaderKey);
		}

		GLsizei length;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length); //Get the length of the compilation log
		char* compilationLog = new char[length];			 //Create the needed char array to store the log
		glGetShaderInfoLog(shaderID, length, NULL, compilationLog); //Get the compilation log
		std::string compilationLogString(compilationLog); //Create string for the compilation log
		delete[] compilationLog; //Delete the compilation log array
		
		throw std::runtime_error(("ERROR: \nCompilation log of shader "+shaderKey+":\n"+compilationLogString).c_str());
	}
	
}

void ShaderManager::attachShader(const std::string& shaderKey,  const std::string& shaderProgramKey)
{
	GLuint shaderID = _shaderData.getShaderID(shaderKey);
	GLuint shaderProgramID = _shaderData.getShaderProgramID(shaderProgramKey);
	glAttachShader(shaderProgramID, shaderID);
};

void ShaderManager::detachShader(const std::string& shaderKey, const std::string& shaderProgramKey)
{
	GLuint shaderID = _shaderData.getShaderID(shaderKey);
	GLuint shaderProgramID = _shaderData.getShaderProgramID(shaderProgramKey);
	glDetachShader(shaderProgramID, shaderID);
};

void ShaderManager::resetProgram()
	{ glUseProgram(0); };

void ShaderManager::createProgram(const std::string& shaderProgramKey)
{
	GLuint shaderProgramID = glCreateProgram();

	if(shaderProgramID != 0){
		_shaderData.saveShaderProgram(shaderProgramKey, shaderProgramID);
	};
};

void ShaderManager::useProgram(const std::string& shaderProgramKey)
{
	GLuint shaderProgramID = _shaderData.getShaderProgramID(shaderProgramKey);
	glUseProgram(shaderProgramID);
};

void ShaderManager::linkProgram(const std::string& shaderProgramKey)
{
	GLuint shaderProgramID = _shaderData.getShaderProgramID(shaderProgramKey);
	glLinkProgram(shaderProgramID);
	GLint linkStatus;
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &linkStatus);
	if(glGetError() != GL_NO_ERROR || linkStatus == GL_FALSE){

		GLsizei length;
		glGetProgramiv(shaderProgramID, GL_INFO_LOG_LENGTH, &length); //Get the length of the compilation log
		char* linkingLog = new char[length];			 //Create the needed char array to store the log
		glGetProgramInfoLog(shaderProgramID, length, NULL, linkingLog); //Get the compilation log
		std::string linkingLogString(linkingLog);	//Save the linking log in a string
		delete[] linkingLog;	//Free the allocated memory
	};
};

GLuint ShaderManager::getShaderID(const std::string& shaderKey)
	{ return _shaderData.getShaderID(shaderKey); };

GLuint ShaderManager::getShaderProgramID(const std::string& shaderProgramKey)
	{ return _shaderData.getShaderProgramID(shaderProgramKey); };

void ShaderManager::deleteProgram(const std::string& shaderProgramKey)
{
	GLuint shaderProgramID = _shaderData.getShaderProgramID(shaderProgramKey);
	glDeleteProgram(shaderProgramID);
}

void ShaderManager::deleteShader(const std::string& shaderKey)
{
	GLuint shaderID = _shaderData.getShaderID(shaderKey);
	glDeleteShader(shaderID);
};

ShaderManager* ShaderManager::_instance = 0;

ShaderManager* ShaderManager::getInstance()
{
	if (_instance == 0) _instance = new ShaderManager();
	return _instance;
};

void ShaderManager::loadUintUniform(const std::string& shaderProgram, const std::string& name, const GLuint value)
{
	int _uniID = glGetUniformLocation(getShaderProgramID(shaderProgram), name.c_str());
	glUniform1ui(_uniID, value);
};

void ShaderManager::loadFloatUniform(const std::string& shaderProgram, const std::string& name, const GLfloat value)
{
	int _uniID = glGetUniformLocation(getShaderProgramID(shaderProgram), name.c_str());
	glUniform1f(_uniID, value);
};

void ShaderManager::loadVec4Uniform(const std::string& shaderProgram, const std::string& name, const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w)
{
	int _uniID = glGetUniformLocation(getShaderProgramID(shaderProgram), name.c_str());
	glUniform4f(_uniID, x, y, z, w);
};

void ShaderManager::loadMatrix4Uniform(const std::string& shaderProgram, const std::string& name, const GLfloat* value)
{
	int _uniID = glGetUniformLocation(getShaderProgramID(shaderProgram), name.c_str());
	glUniformMatrix4fv(_uniID, 1, GL_FALSE, value);
};

void ShaderManager::getBufferVariableIndices(const std::string& shaderProgram, const int length, const GLchar** names, GLint* indices)
{
	for(int i = 0; i < length; ++i) 
		indices[i] = glGetProgramResourceIndex(getShaderProgramID(shaderProgram), GL_BUFFER_VARIABLE, names[i]);
};