#ifndef _model_included_2024976192723993135981283652392892362738523852378228436
#define _model_included_2024976192723993135981283652392892362738523852378228436

#include <GL/glew.h>

struct model
{

	GLuint vao_id;																									
	GLuint vbo_id, tbo_id, nbo_id, ibo_id;	
	GLuint triangles;																								

	bool load_vi(const char* file_name);
	bool load_vnti(const char* file_name);

	void render(); 

  private:	

	struct uvec3_lex : public glm::uvec3
	{
		friend bool operator < (const uvec3_lex a, const uvec3_lex b)
		{
			if (a.z < b.z) return true;
			if (a.z > b.z) return false;
			if (a.y < b.y) return true;
			if (a.y > b.y) return false;
			if (a.x < b.x) return true;
			return false;
		};
	};

};

#endif // _model_included_2024976192723993135981283652392892362738523852378228436