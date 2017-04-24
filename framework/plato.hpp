#ifndef _plato_included_0153674656137456743564378987346159763487516348756384765
#define _plato_included_0153674656137456743564378987346159763487516348756384765

//=======================================================================================================================================================================================================================
// geometric data for five regular Plato solids
// --- vertex positions, face normals, and texture coordinates
//=======================================================================================================================================================================================================================

#include <glm/glm.hpp>                                                      
#include "constants.hpp"

using namespace constants;

namespace plato {

    //===================================================================================================================================================================================================================
    // tetrahedron
    //===================================================================================================================================================================================================================
    namespace tetrahedron {

        const int F = 4;
        const int E = 6;
        const int V = 4;

        const glm::vec3 vertices[V] = 
        {
            inv_sqrt3 * glm::vec3 ( 1.0f,  1.0f,  1.0f),
            inv_sqrt3 * glm::vec3 (-1.0f, -1.0f,  1.0f),
            inv_sqrt3 * glm::vec3 ( 1.0f, -1.0f, -1.0f),
            inv_sqrt3 * glm::vec3 (-1.0f,  1.0f, -1.0f)
        };

        const glm::vec3 normals[F] = 
        {
            inv_sqrt3 * glm::vec3 (-1.0f, -1.0f, -1.0f),
            inv_sqrt3 * glm::vec3 ( 1.0f,  1.0f, -1.0f),
            inv_sqrt3 * glm::vec3 (-1.0f,  1.0f,  1.0f),
            inv_sqrt3 * glm::vec3 ( 1.0f, -1.0f,  1.0f)
        };

        const int faces[] = 
        {
            0, 1, 2,    0, 3, 1,
            1, 3, 2,    2, 3, 0
        };

    }; // namespace tetrahedron 

    //===================================================================================================================================================================================================================
    // cube
    //===================================================================================================================================================================================================================

    namespace cube {                                                                                                        

        const int F = 6;
        const int E = 12;
        const int V = 8;
        const int Q = 6;        

        const glm::vec3 vertices[V] = 
        {
            inv_sqrt3 * glm::vec3(-1.0f, -1.0f, -1.0f),
            inv_sqrt3 * glm::vec3( 1.0f, -1.0f, -1.0f),
            inv_sqrt3 * glm::vec3(-1.0f,  1.0f, -1.0f),
            inv_sqrt3 * glm::vec3( 1.0f,  1.0f, -1.0f),

            inv_sqrt3 * glm::vec3(-1.0f, -1.0f,  1.0f),
            inv_sqrt3 * glm::vec3( 1.0f, -1.0f,  1.0f),
            inv_sqrt3 * glm::vec3(-1.0f,  1.0f,  1.0f),
            inv_sqrt3 * glm::vec3( 1.0f,  1.0f,  1.0f)
        };

        const glm::vec3 normals[F] = 
        { 
            glm::vec3( 0.0f,  0.0f, -1.0f),
            glm::vec3( 0.0f,  0.0f,  1.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3( 1.0f,  0.0f,  0.0f),
            glm::vec3( 0.0f, -1.0f,  0.0f),
            glm::vec3( 0.0f,  1.0f,  0.0f)
        };

        const int faces[] = 
        {
            0, 2, 3, 1,     4, 5, 7, 6,                                                                 // faces parallel to xy plane : the face [0231] and the face [4576]
            0, 4, 6, 2,     1, 3, 7, 5,                                                                 // faces parallel to yz plane : the face [0462] and the face [1375]
            0, 1, 5, 4,     2, 6, 7, 3                                                                  // faces parallel to zx plane : the face [0154] and the face [2673]
        };

        //===============================================================================================================================================================================================================
        // quads, edges and edge_indices are initial arrays for parallelizable spherical surface generating algorithms
        // octahedron can also serve as such initial data, as its faces can be split into pairs forming quads 
        // using quads instead of just triangles allows to subdivide spherical surface nicely into a set of triangular strips reducing index buffer size by a factor of three on average
        //===============================================================================================================================================================================================================

        const glm::uvec4 quads[Q] = 
        {
            glm::uvec4(0, 2, 3, 1), 
            glm::uvec4(4, 5, 7, 6),
            glm::uvec4(0, 1, 5, 4),
            glm::uvec4(1, 3, 7, 5),
            glm::uvec4(3, 2, 6, 7),
            glm::uvec4(2, 0, 4, 6)
        };

        const glm::uvec2 edges[E] = 
        {
            glm::uvec2 (0, 1),
            glm::uvec2 (0, 2),
            glm::uvec2 (0, 4),
            glm::uvec2 (1, 3),
            glm::uvec2 (1, 5),
            glm::uvec2 (2, 3),
            glm::uvec2 (2, 6), 
            glm::uvec2 (3, 7), 
            glm::uvec2 (4, 5),
            glm::uvec2 (4, 6),
            glm::uvec2 (5, 7),
            glm::uvec2 (6, 7)
        };                                                                                                     

        const GLuint edge_indices[V * V] = 
        {
            -1u,  0u,  1u, -1u,  2u, -1u, -1u, -1u,
             0u, -1u, -1u,  3u, -1u,  4u, -1u, -1u,
             1u, -1u, -1u,  5u, -1u, -1u,  6u, -1u,
            -1u,  3u,  5u, -1u, -1u, -1u, -1u,  7u,
             2u, -1u, -1u, -1u, -1u,  8u,  9u, -1u,
            -1u,  4u, -1u, -1u,  8u, -1u, -1u, 10u,
            -1u, -1u,  6u, -1u,  9u, -1u, -1u, 11u,
            -1u, -1u, -1u,  7u, -1u, 10u, 11u, -1u
        };

    }; // namespace cube

    //===================================================================================================================================================================================================================
    // octahedron
    //===================================================================================================================================================================================================================

    namespace octahedron {

        const int F = 8;                                                                                        
        const int E = 12;
        const int V = 6;

        const glm::vec3 vertices[V] = 
        {

            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3( 1.0f,  0.0f,  0.0f),
            glm::vec3( 0.0f, -1.0f,  0.0f),
            glm::vec3( 0.0f,  1.0f,  0.0f),
            glm::vec3( 0.0f,  0.0f, -1.0f),
            glm::vec3( 0.0f,  0.0f,  1.0f)
        };

        const glm::vec3 normals[F] = 
        {
            inv_sqrt3 * glm::vec3 ( 1.0f,  1.0f,  1.0f),
            inv_sqrt3 * glm::vec3 (-1.0f,  1.0f,  1.0f),
            inv_sqrt3 * glm::vec3 ( 1.0f, -1.0f,  1.0f),
            inv_sqrt3 * glm::vec3 (-1.0f, -1.0f,  1.0f),
            inv_sqrt3 * glm::vec3 ( 1.0f,  1.0f, -1.0f),
            inv_sqrt3 * glm::vec3 (-1.0f,  1.0f, -1.0f),
            inv_sqrt3 * glm::vec3 ( 1.0f, -1.0f, -1.0f),
            inv_sqrt3 * glm::vec3 (-1.0f, -1.0f, -1.0f)
        };

        const int faces[] = 
        {
            1, 3, 5,    5, 2, 1,    5, 3, 0,    0, 2, 5,
            1, 2, 4,    4, 3, 1,    0, 3, 4,    4, 2, 0
        };                                                                                                     

    }; // namespace octahedron

    namespace dodecahedron {    

        const int F = 12;
        const int E = 30;
        const int V = 20;

        const glm::vec3 vertices[V] = 
        {
            inv_sqrt3 * glm::vec3 (   -1.0f,   -1.0f,   -1.0f),
            inv_sqrt3 * glm::vec3 (    1.0f,   -1.0f,   -1.0f),
            inv_sqrt3 * glm::vec3 (   -1.0f,    1.0f,   -1.0f),
            inv_sqrt3 * glm::vec3 (    1.0f,    1.0f,   -1.0f),
            inv_sqrt3 * glm::vec3 (   -1.0f,   -1.0f,    1.0f),
            inv_sqrt3 * glm::vec3 (    1.0f,   -1.0f,    1.0f),
            inv_sqrt3 * glm::vec3 (   -1.0f,    1.0f,    1.0f),
            inv_sqrt3 * glm::vec3 (    1.0f,    1.0f,    1.0f),
            inv_sqrt3 * glm::vec3 (-inv_phi,    0.0f,    -phi),
            inv_sqrt3 * glm::vec3 ( inv_phi,    0.0f,    -phi),
            inv_sqrt3 * glm::vec3 (-inv_phi,    0.0f,     phi),
            inv_sqrt3 * glm::vec3 ( inv_phi,    0.0f,     phi),
            inv_sqrt3 * glm::vec3 (    -phi,-inv_phi,    0.0f),
            inv_sqrt3 * glm::vec3 (    -phi, inv_phi,    0.0f),
            inv_sqrt3 * glm::vec3 (     phi,-inv_phi,    0.0f),
            inv_sqrt3 * glm::vec3 (     phi, inv_phi,    0.0f),
            inv_sqrt3 * glm::vec3 (    0.0f,    -phi,-inv_phi),
            inv_sqrt3 * glm::vec3 (    0.0f,    -phi, inv_phi),
            inv_sqrt3 * glm::vec3 (    0.0f,     phi,-inv_phi),
            inv_sqrt3 * glm::vec3 (    0.0f,     phi, inv_phi)
        };

        
        const double mu = 0.5257311121191336060256690848479;                                                // (3 - sqrt(5))/(2 * sqrt(5 - 2 * sqrt(5)))
        const double nu = 0.8506508083520399321815404970630;                                                // (sqrt(5) - 1)/(2 * sqrt(5 - 2 * sqrt(5)))

        const glm::vec3 normals[F] = 
        {
            glm::vec3(0.0f, -mu, -nu),
            glm::vec3(0.0f,  mu, -nu),
            glm::vec3(0.0f,  mu,  nu),
            glm::vec3(0.0f, -mu,  nu),
            glm::vec3(-nu, 0.0f, -mu),
            glm::vec3(-nu, 0.0f,  mu),
            glm::vec3( nu, 0.0f,  mu),
            glm::vec3( nu, 0.0f, -mu),
            glm::vec3(-mu, -nu, 0.0f),
            glm::vec3( mu, -nu, 0.0f),
            glm::vec3( mu,  nu, 0.0f),
            glm::vec3(-mu,  nu, 0.0f)
        };

        const int faces[] = 
        {
             8,  9,  1, 16,  0,      9,  8,  2, 18,  3,                                                     // pair of pentagon faces forming pyramid on the  back xy - face of cube
            10, 11,  7, 19,  6,     11, 10,  4, 17,  5,                                                     // pair of pentagon faces forming pyramid on the front xy - face of cube
            12, 13,  2,  8,  0,     13, 12,  4, 10,  6,                                                     // pair of pentagon faces forming pyramid on the  back yz - face of cube
            14, 15,  7, 11,  5,     15, 14,  1,  9,  3,                                                     // pair of pentagon faces forming pyramid on the front yz - face of cube
            16, 17,  4, 12,  0,     17, 16,  1, 14,  5,                                                     // pair of pentagon faces forming pyramid on the  back zx - face of cube
            18, 19,  7, 15,  3,     19, 18,  2, 13,  6                                                      // pair of pentagon faces forming pyramid on the front zx - face of cube
        };                                                                                                     

    }; // namespace dodecahedron

    namespace icosahedron {

        const int F = 20;
        const int E = 30;
        const int V = 12;
        const int Q = 10;

        const float inv_length = 0.52573111211913360602566908484787660729;                                  // sqrt(2) / sqrt(5 + sqrt(5))

        const glm::vec3 vertices[V] = 
        {
            inv_length * glm::vec3 ( 0.0f,-1.0f, -phi),
            inv_length * glm::vec3 ( 0.0f,-1.0f,  phi),
            inv_length * glm::vec3 ( 0.0f, 1.0f, -phi),
            inv_length * glm::vec3 ( 0.0f, 1.0f,  phi),
            inv_length * glm::vec3 (-1.0f, -phi, 0.0f),
            inv_length * glm::vec3 (-1.0f,  phi, 0.0f),
            inv_length * glm::vec3 ( 1.0f, -phi, 0.0f),
            inv_length * glm::vec3 ( 1.0f,  phi, 0.0f),
            inv_length * glm::vec3 ( -phi, 0.0f,-1.0f),
            inv_length * glm::vec3 (  phi, 0.0f,-1.0f),
            inv_length * glm::vec3 ( -phi, 0.0f, 1.0f),
            inv_length * glm::vec3 (  phi, 0.0f, 1.0f)
        };


        const glm::vec3 normals[] = 
        {
            inv_sqrt3 * glm::vec3(-inv_phi,      0.0,     -phi),
            inv_sqrt3 * glm::vec3( inv_phi,      0.0,     -phi),
            inv_sqrt3 * glm::vec3(     0.0,     -phi, -inv_phi),
            inv_sqrt3 * glm::vec3(    -1.0,     -1.0,     -1.0),
            inv_sqrt3 * glm::vec3(     1.0,     -1.0,     -1.0),
            inv_sqrt3 * glm::vec3(-inv_phi,      0.0,      phi),
            inv_sqrt3 * glm::vec3( inv_phi,      0.0,      phi),
            inv_sqrt3 * glm::vec3(     0.0,     -phi,  inv_phi),
            inv_sqrt3 * glm::vec3(    -1.0,     -1.0,      1.0),
            inv_sqrt3 * glm::vec3(     1.0,     -1.0,      1.0),
            inv_sqrt3 * glm::vec3(     0.0,      phi, -inv_phi),
            inv_sqrt3 * glm::vec3(    -1.0,      1.0,     -1.0),
            inv_sqrt3 * glm::vec3(     1.0,      1.0,     -1.0),
            inv_sqrt3 * glm::vec3(    -0.0,      phi,  inv_phi),
            inv_sqrt3 * glm::vec3(    -1.0,      1.0,      1.0),
            inv_sqrt3 * glm::vec3(     1.0,      1.0,      1.0),
            inv_sqrt3 * glm::vec3(    -phi, -inv_phi,      0.0),
            inv_sqrt3 * glm::vec3(    -phi,  inv_phi,      0.0),
            inv_sqrt3 * glm::vec3(     phi, -inv_phi,      0.0),
            inv_sqrt3 * glm::vec3(     phi,  inv_phi,      0.0)
        };

        const int faces[] = 
        {
            2,  0,  8,      0,  2,  9,      4,  0,  6,      0,  4,  8,      6,  0,  9,
            1,  3, 10,      3,  1, 11,      1,  4,  6,      4,  1, 10,      1,  6, 11,
            2,  5,  7,      5,  2,  8,      2,  7,  9,      5,  3,  7,      3,  5, 10,
            7,  3, 11,      8,  4, 10,      5,  8, 10,      6,  9, 11,      9,  7, 11
        };

        const glm::uvec3 triangles[F] = 
        {
            glm::uvec3(2,  0,  8),
            glm::uvec3(0,  2,  9),
            glm::uvec3(4,  0,  6),
            glm::uvec3(0,  4,  8),
            glm::uvec3(6,  0,  9),
            glm::uvec3(1,  3, 10),
            glm::uvec3(3,  1, 11),
            glm::uvec3(1,  4,  6),
            glm::uvec3(4,  1, 10),
            glm::uvec3(1,  6, 11),
            glm::uvec3(2,  5,  7),
            glm::uvec3(5,  2,  8),
            glm::uvec3(2,  7,  9),
            glm::uvec3(5,  3,  7),
            glm::uvec3(3,  5, 10),
            glm::uvec3(7,  3, 11),
            glm::uvec3(8,  4, 10),
            glm::uvec3(5,  8, 10),
            glm::uvec3(6,  9, 11),
            glm::uvec3(9,  7, 11) 
        };       
                 
        //===============================================================================================================================================================================================================
        // quads, edges and edge_indices are initial arrays for parallelizable spherical surface generating algorithms
        // cube can also serve as such initial data, as its faces can be split into pairs forming quads 
        // using quads instead of just triangles allows to subdivide spherical surface nicely into a set of triangular strips reducing index buffer size by a factor of three on average
        //===============================================================================================================================================================================================================

        const glm::uvec4 quads[Q] = 
        {
            glm::uvec4(11,  7,  3,  1), 
            glm::uvec4(11,  6,  9,  7),
            glm::uvec4( 9,  0,  2,  7),
            glm::uvec4( 6,  4,  0,  9),
            glm::uvec4( 0,  4,  8,  2),
            glm::uvec4( 2,  8,  5,  7),
            glm::uvec4( 5, 10,  3,  7),
            glm::uvec4(10,  4,  1,  3),
            glm::uvec4( 8,  4, 10,  5),
            glm::uvec4( 6, 11,  1,  4)
        };       
                 
        const glm::uvec2 edges[E] = 
        {
            glm::uvec2 ( 0,  2),
            glm::uvec2 ( 0,  4),
            glm::uvec2 ( 0,  6),
            glm::uvec2 ( 0,  8),
            glm::uvec2 ( 0,  9),
            glm::uvec2 ( 1,  3),
            glm::uvec2 ( 1,  4),
            glm::uvec2 ( 1,  6),
            glm::uvec2 ( 1, 10),
            glm::uvec2 ( 1, 11),
            glm::uvec2 ( 2,  5), 
            glm::uvec2 ( 2,  7), 
            glm::uvec2 ( 2,  8), 
            glm::uvec2 ( 2,  9),
            glm::uvec2 ( 3,  5),
            glm::uvec2 ( 3,  7),
            glm::uvec2 ( 3, 10),
            glm::uvec2 ( 3, 11),
            glm::uvec2 ( 4,  6),
            glm::uvec2 ( 4,  8),
            glm::uvec2 ( 4, 10),
            glm::uvec2 ( 5,  7),
            glm::uvec2 ( 5,  8),
            glm::uvec2 ( 5, 10),
            glm::uvec2 ( 6,  9),
            glm::uvec2 ( 6, 11),
            glm::uvec2 ( 7,  9),
            glm::uvec2 ( 7, 11),
            glm::uvec2 ( 8, 10),
            glm::uvec2 ( 9, 11)
        };                                                                                                     
                 
        const GLuint edge_indices[V * V] = 
        {        
           -1u, -1u,  0u, -1u,  1u, -1u,  2u, -1u,  3u,  4u, -1u, -1u,
           -1u, -1u, -1u,  5u,  6u, -1u,  7u, -1u, -1u, -1u,  8u,  9u,
            0u, -1u, -1u, -1u, -1u, 10u, -1u, 11u, 12u, 13u, -1u, -1u,
           -1u,  5u, -1u, -1u, -1u, 14u, -1u, 15u, -1u, -1u, 16u, 17u,
            1u,  6u, -1u, -1u, -1u, -1u, 18u, -1u, 19u, -1u, 20u, -1u,
           -1u, -1u, 10u, 14u, -1u, -1u, -1u, 21u, 22u, -1u, 23u, -1u,
            2u,  7u, -1u, -1u, 18u, -1u, -1u, -1u, -1u, 24u, -1u, 25u,
           -1u, -1u, 11u, 15u, -1u, 21u, -1u, -1u, -1u, 26u, -1u, 27u, 
            3u, -1u, 12u, -1u, 19u, 22u, -1u, -1u, -1u, -1u, 28u, -1u,
            4u, -1u, 13u, -1u, -1u, -1u, 24u, 26u, -1u, -1u, -1u, 29u,
           -1u,  8u, -1u, 16u, 20u, 23u, -1u, -1u, 28u, -1u, -1u, -1u,
           -1u,  9u, -1u, 17u, -1u, -1u, 25u, 27u, -1u, 29u, -1u, -1u,
        };
                                        

    }; // namespace icosahedron

}; // namespace plato

#endif  // _plato_included_0153674656137456743564378987346159763487516348756384765