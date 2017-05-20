#pragma once
#include <string>

#include <glm/glm.hpp>

struct Surface
{
	typedef struct Texture
	{
		int width, height;
		GLuint handle;
		Texture() : width(0), height(0), handle(0) {}
	}sTexture;

	std::string name;
	glm::vec3 diffuseColor;
	glm::vec4 specularColor;

	sTexture diffuseTexture;
	sTexture specularTexture;
	sTexture normalTexture;

	static sTexture defaultDiffuseTexture;
	static sTexture defaultNormalTexture;

	static void init();
	static void cleanUp();

	Surface() {}

	~Surface()
	{
		if(diffuseTexture.handle && glIsTexture(diffuseTexture.handle))
			glDeleteTextures(1, &diffuseTexture.handle);
		if(specularTexture.handle && glIsTexture(specularTexture.handle))
			glDeleteTextures(1, &specularTexture.handle);
		if(normalTexture.handle && glIsTexture(normalTexture.handle))
			glDeleteTextures(1, &normalTexture.handle);
	}

	void loadDiffuseTexture(const char* filename);
	void loadMaskTexture(const char* filename);
	void loadSpecularTexture(const char* filename);
	void loadNormalTexture(const char* filename);

	inline void setDiffuseColor(const glm::vec3& diffuse)
		{ diffuseColor = diffuse; }

	void bind();
	void unbind();
};