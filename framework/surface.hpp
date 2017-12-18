#ifndef _surface_included_35387456923845625068331078643456892347582734651243244
#define _surface_included_35387456923845625068331078643456892347582734651243244

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "vertex.hpp"
#include "vao.hpp"

typedef float (*surface_landscape_func) (const glm::vec2& uv, int level);

struct surface_t
{
    vao_t vao;

     surface_t() {};
    ~surface_t() {}

    //===================================================================================================================================================================================================================
    // Construct 2d surface (function graph) mesh given a full generating function --- which generates the whole vertex_pft/vertex_pnt structure
    //===================================================================================================================================================================================================================
    template<typename vertex_t> void generate_vao(typename maps<vertex_t>::surface_func func, int size_x, int size_y);

    //===================================================================================================================================================================================================================
    // Auxiliary functions that generates index buffer
    //===================================================================================================================================================================================================================
    void generate_ibo(int size_x, int size_y);
    template<typename index_t> void generate_IBO(int size_x, int size_y);

    //===================================================================================================================================================================================================================
    // Construct 2d surface (function graph) mesh of a position function on unit square -1 <= u,v <= 1
    // Texture coordinates of a generated vertex are set to the value of uv-parameter, tangent and normals are computed with finite difference derivative approximation.
    //===================================================================================================================================================================================================================
    void generate_pft2_vao(typename maps<glm::vec3>::surface_func func, int size_x, int size_y);

    //===================================================================================================================================================================================================================
    // Construct 2d surface (function graph) mesh of a position function on unit square -1 <= u,v <= 1
    // Texture coordinates of a generated vertex are set to the value of uv-parameter, normals are computed with finite difference derivative approximation
    //===================================================================================================================================================================================================================
    void generate_pnt2_vao(typename maps<glm::vec3>::surface_func func, int size_x, int size_y);

    //===================================================================================================================================================================================================================
    // Rendering functions
    //===================================================================================================================================================================================================================
    void render();
    void render(GLsizei count, const GLvoid* offset);
    void instanced_render(GLsizei primcount);

};

#endif // _surface_included_35387456923845625068331078643456892347582734651243244
