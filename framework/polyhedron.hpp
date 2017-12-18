#ifndef _polyhedron_included_16890722587364762534765237462537628911976150352836
#define _polyhedron_included_16890722587364762534765237462537628911976150352836

#include <GL/glew.h>                                                                                                        // OpenGL extensions
#include <glm/glm.hpp>

#include "vertex.hpp"
#include "vao.hpp"

//=======================================================================================================================================================================================================================
// Polyhedron is different from other geometric structures --- it is truly a polyhedron, not a smoothed version of it
//  * same geometric vertices are duplicated to have separate normals when counted as incident with different faces
//  * when rendered uses GL_TRIANGLES primitive and no index buffer
//=======================================================================================================================================================================================================================

struct polyhedron
{
    GLuint vao_id;
    vbo_t vbo;

    polyhedron() {};
    void generate_pft2_vao(const glm::vec3* positions, const glm::vec3* normals, const glm::vec2* uvs, int vertex_count);

    //===================================================================================================================================================================================================================
    // functions to generate regular plato solids with :
    //  * position + frame + texture coordinate buffer
    //  * position + normal + texture coordinate buffer
    //  * position + normal
    //===================================================================================================================================================================================================================
    void regular_pft2_vao(int V, int F, const glm::vec3* positions, const glm::vec3* normals, const int* faces, float scale = 1.0f, bool invert_normals = false);
    void regular_pnt2_vao(int V, int F, const glm::vec3* positions, const glm::vec3* normals, const int* faces, float scale = 1.0f, bool invert_normals = false);
    void regular_pn_vao(int V, int F, const glm::vec3* positions, const glm::vec3* normals, const int* faces, float scale = 1.0f, bool invert_normals = false);

    void render();
    void instanced_render(GLsizei primcount);

    ~polyhedron();
};

#endif // _polyhedron_included_16890722587364762534765237462537628911976150352836
