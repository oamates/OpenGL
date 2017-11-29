#include "loader.hpp"

#include <iostream>

/*
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
*/

bool loadMesh_assimp(
	const char * path,
	std::vector<unsigned short> & out_indices,
	std::vector<glm::vec3> & out_vertices, 
	std::vector<glm::vec2> & out_uvs, 
	std::vector<glm::vec3> & out_normals)
{	
/*
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		path, 
		aiProcess_GenSmoothNormals | 
		aiProcess_Triangulate | 
		aiProcess_CalcTangentSpace | 
		aiProcess_FlipUVs);

	if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "The file wasn't successfuly opened: " << path << std::endl;
		return false; 
	}

	aiMesh* mesh = scene->mMeshes[0];


	for(int i=0; i<mesh->mNumVertices; ++i)
	{
		glm::vec3 tmpVec;
		
		//position
		tmpVec.x = mesh->mVertices[i].x;
		tmpVec.y = mesh->mVertices[i].y;
		tmpVec.z = mesh->mVertices[i].z;
		out_vertices.push_back(tmpVec);
		
		//normals
		tmpVec.x = mesh->mNormals[i].x;
		tmpVec.y = mesh->mNormals[i].y;
		tmpVec.z = mesh->mNormals[i].z;
		out_normals.push_back(tmpVec);

		//Textures
		if(mesh->mTextureCoords[0])
		{
			tmpVec.x = mesh->mTextureCoords[0][i].x;
			tmpVec.y = mesh->mTextureCoords[0][i].y;				
		}else{
			tmpVec.x = tmpVec.y = tmpVec.z = 0.0;
		}

		out_uvs.push_back(glm::vec2(tmpVec.x, tmpVec.y));
	}

	for(int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for(int j = 0; j < face.mNumIndices; ++j)
		{
			out_indices.push_back(face.mIndices[j]);
		}
	}

	return true;
*/
	return true;
}