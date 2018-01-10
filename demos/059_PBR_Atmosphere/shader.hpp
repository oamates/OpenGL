#pragma once

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

enum class ShaderType
{
	VERTEX   = static_cast<GLuint>(GL_VERTEX_SHADER),
	TESSCRTL = static_cast<GLuint>(GL_TESS_CONTROL_SHADER),
	TESSEVAL = static_cast<GLuint>(GL_TESS_EVALUATION_SHADER),
	GEOMETRY = static_cast<GLuint>(GL_GEOMETRY_SHADER),
	FRAGMENT = static_cast<GLuint>(GL_FRAGMENT_SHADER),
};

struct Shader
{
	Shader(ShaderType type, const std::string& sourceFile);

    GLuint shaderID();

	bool _compileShader();

	ShaderType	m_ShaderType;
	std::string m_ShaderSourceFile;
	std::string m_ShaderSource;
	GLuint		m_ShaderID;
};

struct Program
{
    Program();

    void attach(Shader* shader);
    void link();
    
    void setUniform(const std::string& name, const glm::mat4& matrix);
    void setUniform(const std::string& name, const glm::vec3& vector);
    void setUniform(const std::string& name, const glm::vec2& vector);
    void setUniform(const std::string& name, float value);
    void setUniform(const std::string& name, int value);

    void bind();
    void unbind();

	GLuint m_ProgramID;
};

struct Texture
{
    explicit Texture(const std::string& filename);
    
    void bind();
    void unbind();
    
    GLuint m_TextureID;
    int    m_Width;
    int    m_Height;
    int    m_NumberOfComponents;
};

struct CubeMap
{
	explicit CubeMap(const std::string& cubemapName);

	void bind();
	void unbind();

	struct FaceInfo
	{
		unsigned char* data = nullptr;
		int width;
		int height;
		int numberOfComponents;
	};

	FaceInfo _loadFaceData(const std::string& basename, const std::string& face);

	GLuint m_CubeMapID;
};