#ifndef _isosurface_included_12375612350902375324392930126548301853229202342556      
#define _isosurface_included_12375612350902375324392930126548301853229202342556

#include <GL/glew.h> 														                                                // OpenGL extensions
#include <glm/glm.hpp>

// ======================================================================================================================================================================================================================
// Indexed mesh, representing a level surface
// ======================================================================================================================================================================================================================
struct isosurface
{
	GLuint vao_id, vbo_id, nbo_id, tbo_id, ibo_id;
	GLuint triangles;

	typedef float (*scalar_field) (const glm::vec3& point);

	isosurface();
	~isosurface();
	void generate_vao(scalar_field func, float iso_value);
	void render();

};

#endif // _isosurface_included_12375612350902375324392930126548301853229202342556
