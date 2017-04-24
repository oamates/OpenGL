#ifndef _tess_included_83471516416133709139417583648360158314713612610731451498
#define _tess_included_83471516416133709139417583648360158314713612610731451498

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "vertex.hpp"
#include "vao.hpp"

//===================================================================================================================================================================================================================
// tesselation functions header
//===================================================================================================================================================================================================================

namespace tess
{
    //===============================================================================================================================================================================================================
    // Single-threaded tesselation function
    // V, E, F of any triangular subdivision satisfy : V - E + F = 2, 3F = 2E, F = 2V - 4, E = 3V - 6.
    //===============================================================================================================================================================================================================
    template<typename vertex_t> vao_t generate_vao(const vertex_t* initial_vertices, GLuint V,
                                                   const glm::uvec4* quads, GLuint Q,
                                                   typename maps<vertex_t>::edge_tess_func edge_func,
                                                   typename maps<vertex_t>::face_tess_func face_func,
                                                   GLuint level);


    //===============================================================================================================================================================================================================
    // Auxiliary structure for multithreaded buffer filling
    //===============================================================================================================================================================================================================
    template<typename vertex_t> struct compute_data
    {
        //===========================================================================================================================================================================================================
        // Computation parameters : barycentric subdivision level, vertices, edges and faces of the initial mesh.
        //===========================================================================================================================================================================================================
        GLuint level;                                                       // the level of barycentric subdivision
        GLuint V, E, Q;                                                     // the number of vertices, edges and quads in the initial mesh
        glm::uvec2* edges;                                                  // array of edges (vertex-vertex pairs)
        GLuint* edge_indices;                                               // edge (vertex-vertex pair) to edge index array 
        const glm::uvec4* quads;                                                  // array of quads in the initial mesh
        GLuint vertices_per_quad;                                           // the number of new vertices inside a quad, the number of new vertices on any edge is simply = level - 1
        GLuint indices_per_quad;                                         
        GLuint quad_vertices_base_index;                                   

        //===========================================================================================================================================================================================================
        // dynamically allocated attribute and index buffers for the functions below to fill
        //===========================================================================================================================================================================================================
        vertex_t* vertices;
        GLuint* indices;

    };

    //===============================================================================================================================================================================================================
    // Multithreaded mesh tesselation : provided functions should compute vertex attributes given barycentric coordinates on edges/faces respectively
    // V, E, F of any triangular subdivision satisfy : V - E + F = 2, 3F = 2E, F = 2V - 4, E = 3V - 6.
    //===============================================================================================================================================================================================================
    template<typename vertex_t, int threads = 6> vao_t generate_vao_mt(const vertex_t* initial_vertices, GLuint V, 
                                                                       const glm::uvec4* quads, GLuint Q, 
                                                                       typename maps<vertex_t>::edge_tess_func edge_func, 
                                                                       typename maps<vertex_t>::face_tess_func face_func, 
                                                                       GLuint level);

    //===============================================================================================================================================================================================================
    // Auxiliary function that populates its own chunk of vertex and index buffers
    //===============================================================================================================================================================================================================
    template<typename vertex_t> void fill_vao_chunk(const compute_data<vertex_t>& data,
                                                    typename maps<vertex_t>::edge_tess_func edge_func,
                                                    typename maps<vertex_t>::face_tess_func face_func,
                                                    GLuint edge_start, GLuint edge_end, 
                                                    GLuint quad_start, GLuint quad_end);

}

#endif // _tess_included_83471516416133709139417583648360158314713612610731451498