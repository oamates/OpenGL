#ifndef _isosurface_included_12375612350902375324392930126548301853229202342556      
#define _isosurface_included_12375612350902375324392930126548301853229202342556

#include <GL/glew.h> 														                                                // OpenGL extensions
#include <glm/glm.hpp>

#include "vao.hpp"

//========================================================================================================================================================================================================================
// indexed mesh, representing a level surface
//========================================================================================================================================================================================================================
typedef double (*scalar_field) (const glm::dvec3& point);

struct isosurface
{
    vao_t vao;

	isosurface() {}
	~isosurface() {}

    void generate_vao(scalar_field func);
    void generate_vao_2(scalar_field func);
	void render()
        { vao.render(); }
};

#endif // _isosurface_included_12375612350902375324392930126548301853229202342556
