// Application resources manager

#ifndef RESOURCE_MANAGER
#define RESOURCE_MANAGER

#include "defines.hpp"
#include "singleton.hpp"
#include "primitive.hpp"

#include <map>
#include <list>
#include <cstdio>
#include <cstdlib>

// Structure d'une texture
struct Texture
{
	GLuint id;
	int w;
	int l;
	unsigned char* content;
};

struct ObjFile
{
	std::list<Triangle> listTriangle;
	Materiau material;
	const Texture* albTex;
	const Texture* rugTex;
	const Texture* specTex;
	const Texture* normalTex;
};

struct ResourceManager: public singleton_t <ResourceManager>
{
	ResourceManager();
	~ResourceManager();
	
	// Chargement d'une texture
	const Texture* LoadTexture(const std::string& parFileName);
	// Chargement d'un modèle obj
	ObjFile* LoadModel(const std::string& parFileName, const std::string& parAlbTexFileName, const std::string& parRugTexFileName, const std::string& parSpecTexFileName, const std::string& parNormalTexFileName );

	std::map<std::string, Texture*> FTextures;
	std::map<std::string, ObjFile*> FModels;
	int FTexIndex;
};

#endif // RESOURCE_MANAGER
