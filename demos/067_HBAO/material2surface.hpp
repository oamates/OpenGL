#pragma once

#include <map>
#include <vector>
#include <string>
#include "mtl.hpp"
#include "surface1.hpp"
#include "log.hpp"

std::map<std::string, Surface*> createSurfaceFromMaterial(const std::vector<Material> &mat, const std::string &path = "")
{
	std::map<std::string, Surface*> surf;

	for(unsigned int i = 0; i < mat.size(); ++i)
	{
		Surface *s = new Surface();


		if(mat[i].diffuseMap != "")
		{
			std::string p = path + mat[i].diffuseMap;
			//debug_msg("Loading diffuse :: %s", p.c_str());
			s->loadDiffuseTexture(p.c_str());
		}

		if(mat[i].maskMap != "")
		{
			std::string p = path + mat[i].maskMap;
			s->loadMaskTexture(p.c_str());
		}

		if(mat[i].normalMap != "")
		{
			std::string p = path + mat[i].normalMap;
			s->loadNormalTexture(p.c_str());
		}

		surf.insert( std::pair<std::string, Surface*> (mat[i].name, s) );	
	}

	return surf;
}