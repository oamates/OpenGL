#include <cstdio>                                                 
#include <map>
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#include "log.hpp"
#include "vao.hpp"
#include "plato.hpp"
#include "tri_stripper.h"

struct uvec2_lex : public glm::uvec2
{
    uvec2_lex(GLuint a, GLuint b) : glm::uvec2(a, b) {};

    friend bool operator < (const uvec2_lex a, const uvec2_lex b)
    {
        if (a.y < b.y) return true;
        if (a.y > b.y) return false;
        if (a.x < b.x) return true;
        return false;
    }
};

/*

namespace triangle_stripper
{

    typedef unsigned int index;
    typedef std::vector<index> indices;

    enum primitive_type
    {
        TRIANGLES       = 0x0004,   // = GL_TRIANGLES
        TRIANGLE_STRIP  = 0x0005    // = GL_TRIANGLE_STRIP
    };

    struct primitive_group
    {
        indices         Indices;
        primitive_type  Type;
    };

    typedef std::vector<primitive_group> primitive_vector;
}

*/

void print_primitives(triangle_stripper::primitive_vector& strips)
{
    uint32_t q = 0;
    printf("\tprimitives vector size = %u.\n", (uint32_t) strips.size());
    for(uint32_t i = 0; i < strips.size(); ++i)
    {
        q += (strips[i].Type == triangle_stripper::TRIANGLES) ? strips[i].Indices.size() + 1 : (strips[i].Indices.size() / 3) * 4;
        printf("\t\tprimitive[%u].type = %s :: ", i, (strips[i].Type == triangle_stripper::TRIANGLES) ? "GL_TRIANGLES" : "GL_TRIANGLE_STRIP");
        for(uint32_t j = 0; j < strips[i].Indices.size(); ++j)
            printf(" %u", strips[i].Indices[j]);
        printf("\n");
    }
    printf("Total IBO Length :: %u indices.", q);    
}


int main(int argc, char* argv[])
{
    //================================================================================================================================================================================================================
    // Common data for stripifier test
    //================================================================================================================================================================================================================

    //================================================================================================================================================================================================================
    // TEST #1 : one-strip set
    //================================================================================================================================================================================================================
/*
    printf("Simple one-strip test :: \n");
    uint32_t topology[] = 
    {
        0, 1, 2,
        2, 1, 3,
        2, 3, 4,
        4, 3, 5,
        4, 5, 6,
        6, 5, 7,
        6, 7, 8,
        8, 7, 9
    };


    uint32_t I = sizeof(topology) / sizeof(uint32_t);
    std::vector<uint32_t> topology_vec (topology, topology + I);
    triangle_stripper::tri_stripper stripper(topology_vec);
    stripper.SetCacheSize(0);
    stripper.SetMinStripSize(2);
    stripper.SetBackwardSearch(true);

    triangle_stripper::primitive_vector strips;
    stripper.Strip(&strips);
    print_primitives(strips);
*/
    

    //================================================================================================================================================================================================================
    // TEST #2 : plain grid
    //================================================================================================================================================================================================================
/*
    std::vector<uint32_t> faces;
    const uint32_t size_x = 73;
    const uint32_t size_y = 31;

    printf("Plain grid triangles :: \n\n");

    uint32_t index = 0;
    uint32_t q = size_x + 1;
    uint32_t p = 0;

    for (uint32_t y = 0; y < size_y; ++y)
    {
        for (uint32_t x = 0; x < size_x; ++x)
        {
            faces.push_back(p);
            faces.push_back(p + 1);
            faces.push_back(q + 1);
            faces.push_back(p);
            faces.push_back(q + 1);
            faces.push_back(q);
            ++p; ++q;
        }
        ++p; ++q;
    }

    triangle_stripper::tri_stripper stripper(faces);
    stripper.SetCacheSize(0);
    stripper.SetMinStripSize(2);
    stripper.SetBackwardSearch(true);

    triangle_stripper::primitive_vector strips;
    stripper.Strip(&strips);
    print_primitives(strips);
*/
    //================================================================================================================================================================================================================
    // TEST #3 : toral index buffer object
    //================================================================================================================================================================================================================
/*
    std::vector<uint32_t> faces;
    const uint32_t size_x = 73;
    const uint32_t size_y = 31;

    uint32_t q = size_x;
    uint32_t p = 0;

    for (uint32_t y = 0; y < size_y - 1; ++y)
    {
        for (uint32_t x = 0; x < size_x - 1; ++x)
        {
            faces.push_back(p);
            faces.push_back(p + 1);
            faces.push_back(q + 1);
            faces.push_back(p);
            faces.push_back(q + 1);
            faces.push_back(q);
            ++p; ++q;
        }
        faces.push_back(p);
        faces.push_back(p - size_x + 1);
        faces.push_back(q - size_x + 1);
        faces.push_back(p);
        faces.push_back(q - size_x + 1);
        faces.push_back(q);
        ++p; ++q;
    }
    q = 0;
    for (uint32_t x = 0; x < size_x - 1; ++x)
    {
        faces.push_back(p);
        faces.push_back(p + 1);
        faces.push_back(q + 1);
        faces.push_back(p);
        faces.push_back(q + 1);
        faces.push_back(q);
        ++p; ++q;
    }
    faces.push_back(p);
    faces.push_back(p - size_x + 1);
    faces.push_back(q - size_x + 1);
    faces.push_back(p);
    faces.push_back(q - size_x + 1);
    faces.push_back(q);

    triangle_stripper::tri_stripper stripper(faces);
    stripper.SetCacheSize(0);
    stripper.SetMinStripSize(2);
    stripper.SetBackwardSearch(true);

    triangle_stripper::primitive_vector strips;
    stripper.Strip(&strips);
    print_primitives(strips);
*/
    //================================================================================================================================================================================================================
    // TEST #4 : sphere subdivision index buffer object
    //================================================================================================================================================================================================================
/*
    const uint32_t level = 5;
    uint32_t deg4 = 1 << (2 * level);
    uint32_t F = deg4 * (plato::icosahedron::F);
    uint32_t f, v = plato::icosahedron::V;

    glm::uvec3* faces = (glm::uvec3*) malloc(F * sizeof(glm::uvec3));

    for(f = 0; f < plato::icosahedron::F; ++f) faces[f] = plato::icosahedron::triangles[f];

    for (uint32_t l = 0; l < level; ++l)
    {
        uint32_t end = f;

        std::map<uvec2_lex, GLuint> center_index;
        for (GLuint triangle = 0; triangle < end; ++triangle)
        {
            uint32_t P = faces[triangle].x;
            uint32_t Q = faces[triangle].y;
            uint32_t R = faces[triangle].z;
            uint32_t S, T, U;

            uvec2_lex PQ = (P < Q) ? uvec2_lex(P, Q) : uvec2_lex(Q, P);
            uvec2_lex QR = (Q < R) ? uvec2_lex(Q, R) : uvec2_lex(R, Q);
            uvec2_lex RP = (R < P) ? uvec2_lex(R, P) : uvec2_lex(P, R);

            std::map<uvec2_lex, GLuint>::iterator it;
            
            it = center_index.find(PQ); 
            if (it != center_index.end()) S = it->second;
            else
            {
                S = v++;
                center_index[PQ] = S;
            }

            it = center_index.find(QR); 
            if (it != center_index.end()) T = it->second;
            else
            {
                T = v++;
                center_index[QR] = T;                
            }

            it = center_index.find(RP); 
            if (it != center_index.end()) U = it->second;
            else
            {
                U = v++;
                center_index[RP] = U;
            }

            faces[triangle] = glm::uvec3(S, T, U);
            faces[f++] = glm::uvec3(P, S, U);
            faces[f++] = glm::uvec3(Q, T, S);
            faces[f++] = glm::uvec3(R, U, T);
        }
    }

    std::vector<uint32_t> faces_vec ((uint32_t*) faces, (uint32_t*) faces + 3 * F);

    triangle_stripper::tri_stripper stripper(faces_vec);
    stripper.SetCacheSize(0);
    stripper.SetMinStripSize(2);
    stripper.SetBackwardSearch(true);

    triangle_stripper::primitive_vector strips;
    stripper.Strip(&strips);
    print_primitives(strips);

    free(faces);
*/
    //================================================================================================================================================================================================================
    // TEST #5 : Real IBO from obj-file
    //================================================================================================================================================================================================================

    if (argc != 2)
        exit_msg("Usage : stripify <filename> ... \n");

    const char* file_name = argv[1];
    debug_msg("Stripifying %s\n\n", file_name);

    //================================================================================================================================================================================================================
    // read VAO params
    //================================================================================================================================================================================================================
    vao_t::header_t header;
    
    FILE* file = fopen(file_name, "rb");
    if (!file)
        exit_msg("File %s not found.\n", file_name);
    
    debug_msg("Reading %s header ... \n", file_name);
    fread (&header, sizeof(vao_t::header_t), 1, file);
    
    //================================================================================================================================================================================================================
    // check that the primitive mode is GL_TRIANGES and the number of indices is divisible by 3
    //================================================================================================================================================================================================================
    if((header.mode != GL_TRIANGLES) || ((header.ibo_size % 3) != 0) || (header.type != GL_UNSIGNED_INT))
    {
        if ((header.mode != GL_TRIANGLES))
            printf("\nIndex primitive mode (%x) is not GL_TRIANGLES(%x). Cannot stripify ... \n", header.mode, GL_TRIANGLES);
        if ((header.ibo_size % 3) != 0)
            printf("\nThe number of indices is not divisible by 3. Cannot stripify ... \n");
        if (header.type != GL_UNSIGNED_INT)
            printf("\nUnsupported index type : %x ...\n", header.type);
        fclose(file);
        return -1;
    }
    
    uint32_t F = header.ibo_size / 3;
    printf("\n\tPrimitive mode is GL_TRIANGLES (%x).\n\tNumber of trianges : %i.\n", GL_TRIANGLES, F);        
    
    //================================================================================================================================================================================================================
    // read attribute buffer
    //================================================================================================================================================================================================================
    GLsizei stride = 0;        
    for(GLuint layout = header.layout; layout; layout >>= 2) 
        stride += (1 + (layout & 0x3)) * sizeof(GLfloat);
    
    printf("\n\tAttribute buffer layout : %i.\n\tVBO size : %i vertices.\n\tAttribute size : %i bytes.\n\tTotal size : %i bytes.\n", header.layout, header.vbo_size, stride, header.vbo_size * stride);
    
    void* attr_buf_ptr = malloc(stride * header.vbo_size);
    fread(attr_buf_ptr, stride, header.vbo_size, file);        
    
    
    //================================================================================================================================================================================================================
    // read the index buffer : GL_UNSIGNED_INT case
    //================================================================================================================================================================================================================
    GLuint total_size = sizeof(GL_UNSIGNED_INT) * header.ibo_size;        
    printf("\n\tIndex type is GL_UNSIGNED_INT.\n\tIBO size : %i indices.\n\tTotal size : %i bytes.\n", header.ibo_size, total_size);
    
    glm::uvec3* faces = (glm::uvec3*) malloc(total_size);
    fread(faces, sizeof(GL_UNSIGNED_INT), header.ibo_size, file);
    fclose(file);
    
//    print_ibo(faces, F);
    std::vector<uint32_t> faces_vec ((uint32_t*) faces, (uint32_t*) faces + header.ibo_size);

    triangle_stripper::tri_stripper stripper(faces_vec);
    stripper.SetCacheSize(0);
    stripper.SetMinStripSize(2);
    stripper.SetBackwardSearch(true);

    triangle_stripper::primitive_vector strips;
    stripper.Strip(&strips);
    print_primitives(strips);

//    char output_name[128];
//    snprintf(output_name, 128, "_%s", file_name);
/*
    FILE* output = fopen(output_name, "wb");
    header.mode = GL_TRIANGLE_STRIP;
    
    fwrite(&header, sizeof(vao_t::header_t), 1, output);
    fwrite(attr_buf_ptr, stride, header.vbo_size, output);
    fwrite(strips.data(), sizeof(GL_UNSIGNED_INT), strips.size(), output);
    fclose(output);
*/
    free(faces);
    free(attr_buf_ptr);
    
    return 0;   
}