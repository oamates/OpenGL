#include "ParticleManager.h"

void ParticleManager::loadParticleBuffer(int numParticles, int iniRadius)
{
	int _numParticles = numParticles;
	Particle* _particles = new Particle[_numParticles];
	setParticles(_particles, _numParticles, iniRadius);
	
	GLuint bufferID;
	glGenBuffers(1, &bufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles*sizeof(Particle), _particles ,GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferID);
	
	_bufferData.saveBuffer("particleBuffer", bufferID);
};

void ParticleManager::setParticles(Particle* particles, int numParticles, int iniRadius)
{
	float rndX, rndY, rndZ;
	std::mt19937 eng;
	std::uniform_real_distribution<float> dist(static_cast<float>(iniRadius)*(-1.0f), (float) iniRadius);

	for(int i = 0; i < numParticles; ++i)
	{
		rndX = dist(eng);
		rndY = dist(eng);
		rndZ = dist(eng);
		particles[i]._currPosition = glm::vec4(rndX, rndY, rndZ, 1.0f);
		particles[i]._prevPosition = glm::vec4(rndX, rndY, rndZ, 1.0f);
	};
};

GLuint ParticleManager::getParticleBufferID() const
{
	return _bufferData.getBufferID("particleBuffer");
};

void ParticleManager::loadUintUniform(GLuint shaderProgramID, const std::string& name, GLuint value)
{
	int _uniID = glGetUniformLocation(shaderProgramID, name.c_str());
	glUniform1ui(_uniID, value);
};

void ParticleManager::loadFloatUniform(GLuint shaderProgramID, const std::string& name, GLfloat value)
{
	int _uniID = glGetUniformLocation(shaderProgramID, name.c_str());
	glUniform1f(_uniID, value);
};

void ParticleManager::loadVec4Uniform(GLuint shaderProgramID, const std::string& name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	int _uniID = glGetUniformLocation(shaderProgramID, name.c_str());
	glUniform4f(_uniID, x, y, z, w);
};

void ParticleManager::loadMatrix4Uniform(GLuint shaderProgramID, const std::string& name, const GLfloat* value)
{
	int _uniID = glGetUniformLocation(shaderProgramID, name.c_str());
	glUniformMatrix4fv(_uniID, 1, GL_FALSE, value);
};

void ParticleManager::deleteParticleManager()
{
 	GLuint ID = _bufferData.getBufferID("particleBuffer");
	glDeleteBuffers(1, &ID);
	_bufferData.deleteBuffer("particleBuffer");
};