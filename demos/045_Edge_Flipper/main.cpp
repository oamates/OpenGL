//========================================================================================================================================================================================================================
// DEMO 045 : Mesh Shells visualizer
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <map>
#include <sstream>
#include <fstream>
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "constants.hpp"

template<typename index_t> struct halfedge_t
{
    index_t a, b;
    uint32_t face;

    uint32_t next;
    uint32_t opposite;

    halfedge_t(index_t a, index_t b, uint32_t face)
        : a(a), b(b), face(face), next(-1), opposite(-1)
    {}
};

template<typename index_t> struct he_manifold_t
{
    //===================================================================================================================================================================================================================
    // the number of faces and the pointer to faces array
    //===================================================================================================================================================================================================================
    GLuint F;
    glm::tvec3<index_t>* faces;

    //===================================================================================================================================================================================================================
    // pointer to geometric position data, not a required field as many algorithms
    // may need only topological (index) data, will be 0 in this case
    //===================================================================================================================================================================================================================
    GLuint V;
    glm::dvec3* positions;

    //===================================================================================================================================================================================================================
    // main data structure for fast mesh traversing
    //===================================================================================================================================================================================================================
    GLuint E;
    halfedge_t<index_t>* edges;

    he_manifold_t(glm::tvec3<index_t>* faces, uint32_t F, glm::dvec3* positions, uint32_t V)
        : faces(faces), F(F), positions(positions), V(V)
    {
        E = F + F + F;
        edges = (halfedge_t<index_t>*) malloc(E * sizeof(halfedge_t<index_t>));

        //================================================================================================================================================================================================================
        // create half-edge structure array from the input data                                                                                                                                                                    
        //================================================================================================================================================================================================================
        for (uint32_t e = 0, f = 0; f < F; ++f)
        {
            debug_msg("Creating halfedge #%u", e);


            index_t a = faces[f].x,
                    b = faces[f].y,
                    c = faces[f].z;

            uint32_t i_ab = e++;
            uint32_t i_bc = e++;
            uint32_t i_ca = e++;

            edges[i_ab].a = a;
            edges[i_ab].b = b;
            edges[i_ab].face = f;
            edges[i_ab].next = i_bc;

            edges[i_bc].a = b;
            edges[i_bc].b = c;
            edges[i_bc].face = f;
            edges[i_bc].next = i_ca;

            edges[i_ca].a = c;
            edges[i_ca].b = a;
            edges[i_ca].face = f;
            edges[i_ca].next = i_ab;


        }

        debug_msg("Array filled\n\n");

        for(uint32_t e = 0; e < E; ++e)
        {
            debug_msg("edges[%u] = halfedge(a = %u, b = %u, face = %u, next = %u)", e, edges[e].a, edges[e].b, edges[e].face, edges[e].next);
        }


        //================================================================================================================================================================================================================
        // quick sort edges lexicographically
        //================================================================================================================================================================================================================
        const unsigned int STACK_SIZE = 32;                                                         // variables to emulate stack of sorting requests

        struct
        {
            int32_t l;                                                                             // left index of the sub-array that needs to be sorted
            int32_t r;                                                                             // right index of the sub-array to sort
        } _stack[STACK_SIZE];

        int sp = 0;                                                                                 // stack pointer, stack grows up not down
        _stack[sp].l = 0;
        _stack[sp].r = E - 1;
                                                                                                                                                                                                                          
        do                                                                                                                                                                                                                    
        {                                                                                                                                                                                                                     
            int32_t l = _stack[sp].l;                                                                                                                                                                                
            int32_t r = _stack[sp].r;                                                                                                                                                                                
            --sp;                                                                                                                                                                                                             
            do                                                                                                                                                                                                                
            {                                                                                                                                                                                                                 
                int32_t i = l;                                                                                                                                                                                       
                int32_t j = r;                                                                                                                                                                                       
                int32_t m = (i + j) / 2;                                                                                                                                                                         
                index_t a = edges[m].a;                                                                                                                                                                                             
                index_t b = edges[m].b;                                                                                                                                                                                             
                do                                                                                                                                                                                                            
                {                                                                                                                                                                                                             
                    while ((edges[i].b < b) || ((edges[i].b == b) && (edges[i].a < a))) i++;        // lexicographic compare and proceed forward if less                                                                      
                    while ((edges[j].b > b) || ((edges[j].b == b) && (edges[j].a > a))) j--;        // lexicographic compare and proceed backward if greater                                                                     
                                                                                                                                                                                                                          
                    if (i <= j)                                                                                                                                                                                               
                    {
                        std::swap(edges[i].a, edges[j].a);
                        std::swap(edges[i].b, edges[j].b);
                        std::swap(edges[i].face, edges[j].face);

                        uint32_t i_bc = edges[i].next; 
                        uint32_t j_bc = edges[j].next; 
                        edges[i_bc].next = j;
                        edges[j_bc].next = i;

                        i++;
                        j--;
                    }
                }
                while (i <= j);

                if (j - l < r - i)                                                                  // push the larger interval to stack and continue sorting the smaller one                                                      
                {
                    if (i < r)
                    {
                        ++sp;
                        _stack[sp].l = i;
                        _stack[sp].r = r;
                    }                                                                                                                                                                                                         
                    r = j;                                                                                                                                                                                                    
                }                                                                                                                                                                                                             
                else                                                                                                                                                                                                          
                {                                                                                                                                                                                                             
                    if (l < j)                                                                                                                                                                                                
                    {                                                                                                                                                                                                         
                        ++sp;                                                                                                                                                                                                 
                        _stack[sp].l = l;                                                                                                                                                                                     
                        _stack[sp].r = j;                                                                                                                                                                                     
                    }                                                                                                                                                                                                         
                    l = i;                                                                                                                                                                                                    
                }                                                                                                                                                                                                             
            }                                                                                                                                                                                                                 
            while(l < r);                                                                                                                                                                                                     
        }                                                                                                                                                                                                                     
        while (sp >= 0);

        debug_msg("Edges sorted\n\n");

        for(uint32_t e = 0; e < E; ++e)
        {
            debug_msg("edges[%u] = halfedge(a = %u, b = %u, face = %u, next = %u)", e, edges[e].a, edges[e].b, edges[e].face, edges[e].next);
        }

        //============================================================================================================================================================================================================
        // fill opposite edge indices, -1 will indicate boundary edges                                                                                                                                                        
        //============================================================================================================================================================================================================

        for (uint32_t e = 0; e < E; ++e)
        {
            index_t a = edges[e].a;
            index_t b = edges[e].b;

            if (a < b)
            {
                int32_t l = 0;
                int32_t r = E - 1;
                while (l <= r)
                {
                    int32_t o = (r + l) / 2;

                    if (edges[o].b < a) { l = o + 1; continue; }
                    if (edges[o].b > a) { r = o - 1; continue; }
                    if (edges[o].a < b) { l = o + 1; continue; }
                    if (edges[o].a > b) { r = o - 1; continue; }

                    //================================================================================================================================================================================================
                    // opposite edge found, o is its index
                    //================================================================================================================================================================================================
                    edges[e].opposite = o;
                    edges[o].opposite = e;
                    break;
                }
            }
        }

        debug_msg("Edges opposite filled :: \n\n");

        for(uint32_t e = 0; e < E; ++e)
        {
            debug_msg("edges[%u] = halfedge(a = %u, b = %u, face = %u, next = %u, opposite = %u)", e, edges[e].a, edges[e].b, edges[e].face, edges[e].next, edges[e].opposite);
        }


    }

    //================================================================================================================================================================================================================
    // replaces triangle pair ABC + ADB with pair ADC + BCD
    //================================================================================================================================================================================================================
    uint32_t flip_edge(uint32_t e)
    {
        uint32_t o = edges[e].opposite;
        index_t a = edges[e].a;
        index_t b = edges[e].b;

        index_t c = edges[edges[e].next].b;
        index_t d = edges[edges[o].next].b;

        edges[e].a = d;
        edges[e].b = c;
        edges[o].a = c;
        edges[o].b = d;

        faces[edges[o].next] = e;
        faces[edges[e].next] = o;
        edges[e].next = edges[edges[e].next].next;
        edges[o].next = edges[edges[o].next].next;

        faces[edges[e].face] = glm::tvec3<index_t>(a, d, c);
        faces[edges[o].face] = glm::tvec3<index_t>(b, c, d);
    }

    double angle_defect(double cos_A, double cos_B, double cos_C)
    {
        return glm::abs(cos_A - 0.5) + glm::abs(cos_B - 0.5) + glm::abs(cos_C - 0.5);
    }

    void flip_edges(double threshold)
    {
        for(GLuint e = 0; e < E; ++e)
        {
            uint32_t o = edges[e].opposite;

            index_t a = edges[e].a;
            index_t b = edges[e].b;
            index_t c = edges[edges[e].next].b;
            index_t d = edges[edges[o].next].b;

            glm::dvec3 A = positions[a];
            glm::dvec3 B = positions[b];
            glm::dvec3 C = positions[c];
            glm::dvec3 D = positions[d];


            glm::dvec3 AB = glm::normalize(B - A);
            glm::dvec3 BC = glm::normalize(C - B);
            glm::dvec3 CA = glm::normalize(A - C);

            glm::dvec3 AD = glm::normalize(D - A);
            glm::dvec3 DB = glm::normalize(B - D);

            glm::dvec3 DC = glm::normalize(C - D);

            //========================================================================================================================================================================================================
            // triangle ABC
            //========================================================================================================================================================================================================
            double cos_CAB = glm::dot(CA, AB);
            double cos_ABC = glm::dot(AB, BC);
            double cos_BCA = glm::dot(BC, CA);
            double degeneracy_ABC = angle_defect(cos_CAB, cos_ABC, cos_BCA);

            //========================================================================================================================================================================================================
            // triangle ADB
            //========================================================================================================================================================================================================
            double cos_DBA = -glm::dot(DB, AB);
            double cos_ADB =  glm::dot(AD, DB);
            double cos_BAD = -glm::dot(AB, AD);
            double degeneracy_ADB = angle_defect(cos_DBA, cos_ADB, cos_BAD);

            //========================================================================================================================================================================================================
            // triangle ADC
            //========================================================================================================================================================================================================
            double cos_DCA = glm::dot(DC, CA);
            double cos_ADC = glm::dot(AD, DC);
            double cos_CAD = glm::dot(CA, AD);
            double degeneracy_ADC = angle_defect(cos_DCA, cos_ADC, cos_CAD);

            //========================================================================================================================================================================================================
            // triangle DBC
            //========================================================================================================================================================================================================
            double cos_BCD = -glm::dot(BC, DC);
            double cos_DBC =  glm::dot(DB, BC);
            double cos_CDB = -glm::dot(DC, DB);
            double degeneracy_DBC = angle_defect(cos_BCD, cos_DBC, cos_CDB);


            if(degeneracy_ABC + degeneracy_ADB > degeneracy_ADC + degeneracy_DBC)
            {
            }

        }    
    }    

    ~he_manifold_t()
        { free(edges); }
};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // load model and build it edge-face structure
    //===================================================================================================================================================================================================================
    const int F = 20;
    const int V = 12;

    glm::uvec3 triangles[F] = 
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

    const double phi = constants::phi_d;

    glm::dvec3 positions[V] = 
    {
        glm::dvec3(  0.0, -1.0, -phi),
        glm::dvec3(  0.0, -1.0,  phi),
        glm::dvec3(  0.0,  1.0, -phi),
        glm::dvec3(  0.0,  1.0,  phi),
        glm::dvec3( -1.0, -phi,  0.0),
        glm::dvec3( -1.0,  phi,  0.0),
        glm::dvec3(  1.0, -phi,  0.0),
        glm::dvec3(  1.0,  phi,  0.0),
        glm::dvec3( -phi,  0.0, -1.0),
        glm::dvec3(  phi,  0.0, -1.0),
        glm::dvec3( -phi,  0.0,  1.0),
        glm::dvec3(  phi,  0.0,  1.0)
    };

    he_manifold_t<GLuint> manifold_struct(triangles, F, positions, V);


    return 0;
}