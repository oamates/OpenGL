#ifndef _model_included_2024976192723993135981283652392892362738523852378228436
#define _model_included_2024976192723993135981283652392892362738523852378228436

#include <GL/glew.h>

struct model_vti
{
	GLuint vao_id;																									
	GLuint vbo_id, tbo_id, ibo_id;
	GLuint mesh_size;

	bool load(const char* file_name);
	void render();
};

#endif // _model_included_2024976192723993135981283652392892362738523852378228436