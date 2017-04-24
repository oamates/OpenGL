#include <cstdio>
#include <cstdint>
#include <map>
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#include "log.hpp"
#include "vao.hpp"
#include "plato.hpp"
#include "striper.hpp"
#include "stripifier.hpp"

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

void print_strips(const std::vector<uint32_t>& strips)
{
    printf("\nTotal size : %u\nStrip #1 : ", (uint32_t) strips.size());
    uint32_t index = 0;
    uint32_t s = 0;

    for(uint32_t i = 0; i < strips.size(); ++i)
    {
        uint32_t q = strips[index++];
        if (q == -1)
            printf("\nStrip #%u : ", ++s);
        else
            printf("%i ", q);
    }
    printf("\nNumber of strips :: %u", s);
}




int main(int argc, char* argv[])
{
    //================================================================================================================================================================================================================
    // Common data for stripifier test
    //================================================================================================================================================================================================================
    glm::uvec3* faces;
    uint32_t F;

    //================================================================================================================================================================================================================
    // TEST #1 : plain grid mesh
    //================================================================================================================================================================================================================
    const uint32_t size_x = 5;
    const uint32_t size_y = 3;

    F = 2 * size_x * size_y;
    faces = (glm::uvec3*) malloc (F * sizeof(glm::uvec3));

    uint32_t index = 0;
    uint32_t q = size_x + 1;
    uint32_t p = 0;

    for (uint32_t y = 0; y < size_y; ++y)
    {
        for (uint32_t x = 0; x < size_x; ++x)
        {
            faces[index++] = glm::uvec3(p, p + 1, q + 1);
            faces[index++] = glm::uvec3(p, q + 1, q);
            ++p; ++q;
        }
        ++p; ++q;
    }

    //========================================================================================================================================================================================================================
    // create edge structure array from the input data
    //========================================================================================================================================================================================================================
    trimesh_t<uint32_t> grid_mesh(faces, F);

    printf("Plain grid triangles (%u x %u):: \n\n", size_x, size_y);

    grid_mesh.print_ibo();
    grid_mesh.print_vertex_statistics();
    grid_mesh.print_edge_and_face_statistics();

    free(faces);

















    //================================================================================================================================================================================================================
    // TEST #2 : toral subdivision mesh
    //================================================================================================================================================================================================================

    F = 2 * size_x * size_y;
    faces = (glm::uvec3*) malloc (F * sizeof(glm::uvec3));

    index = 0;
    q = size_x;
    p = 0;

    for (uint32_t y = 0; y < size_y - 1; ++y)
    {
        for (uint32_t x = 0; x < size_x - 1; ++x)
        {
            faces[index++] = glm::uvec3(p, p + 1, q + 1);
            faces[index++] = glm::uvec3(p, q + 1, q);
            ++p; ++q;
        }
        faces[index++] = glm::uvec3(p, p - size_x + 1, q - size_x + 1);
        faces[index++] = glm::uvec3(p, q - size_x + 1, q);
        ++p; ++q;
    }
    q = 0;
    for (uint32_t x = 0; x < size_x - 1; ++x)
    {
        faces[index++] = glm::uvec3(p, p + 1, q + 1);
        faces[index++] = glm::uvec3(p, q + 1, q);
        ++p; ++q;
    }
    faces[index++] = glm::uvec3(p, p - size_x + 1, q - size_x + 1);
    faces[index++] = glm::uvec3(p, q - size_x + 1, q);

    //========================================================================================================================================================================================================================
    // create edge structure array from the input data
    //========================================================================================================================================================================================================================
    trimesh_t<uint32_t> toral_mesh(faces, F);

    printf("Torus subdivision (%u x %u):: \n\n", size_x, size_y);

    toral_mesh.print_ibo();
    toral_mesh.print_vertex_statistics();
    toral_mesh.print_edge_and_face_statistics();
    toral_mesh.count_edge_types();

    free(faces);













    //================================================================================================================================================================================================================
    // TEST #3 : sphere subdivision index buffer object
    //================================================================================================================================================================================================================
    const uint32_t level = 3;
    uint32_t deg4 = 1 << (2 * level);
    F = deg4 * (plato::icosahedron::F);
    uint32_t f, v = plato::icosahedron::V;

    faces = (glm::uvec3*) malloc(F * sizeof(glm::uvec3));

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

    //========================================================================================================================================================================================================================
    // create edge structure array from the input data
    //========================================================================================================================================================================================================================
    trimesh_t<uint32_t> sphere_mesh(faces, F);

    printf("Sphere subdivision of level %u :: V = %u \n\n", level, v);

    sphere_mesh.print_edge_and_face_statistics();
    sphere_mesh.print_vertex_statistics();
    sphere_mesh.count_edge_types();
    free(faces);


/*

    //================================================================================================================================================================================================================
    // TEST #4 : Real IBO from obj-file
    //================================================================================================================================================================================================================

    if (argc < 2)
        exit_msg("Usage : stripify <filename> ... \n");

    double total_uncompressed = 0.0, total_compressed = 0.0;

    for (int fname_idx = 1; fname_idx < argc; ++fname_idx)
    {
        const char* file_name = argv[fname_idx];
        printf("\n\nOBJ Mesh, source :: %s\n\n", file_name);

        //================================================================================================================================================================================================================
        // read VAO params
        //================================================================================================================================================================================================================
        vao_t::header_t header;
        
        FILE* file = fopen(file_name, "rb");
        if (!file) 
        {
            printf("File %s not found.\n", file_name);
            continue;
        }
        
        printf("Reading %s header ... \n", file_name);
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
            continue;
        }
        
        F = header.ibo_size / 3;
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
        
        faces = (glm::tvec3<GLuint>*) malloc(total_size);
        fread(faces, sizeof(GL_UNSIGNED_INT), header.ibo_size, file);
        fclose(file);
        
//        print_ibo(faces, F);

        fflush(stdout);

        trimesh_t<uint32_t> obj_mesh(faces, F);

        obj_mesh.print_ibo();
        obj_mesh.print_vertex_statistics();
        obj_mesh.print_edge_and_face_statistics();        
        fflush(stdout);

        char output_name[128];
        snprintf(output_name, 128, "_%s", file_name);
        FILE* output = fopen(output_name, "wb");
        header.mode = GL_TRIANGLE_STRIP;
        
        fwrite(&header, sizeof(vao_t::header_t), 1, output);
        fwrite(attr_buf_ptr, stride, header.vbo_size, output);
        fwrite(strips.data(), sizeof(GL_UNSIGNED_INT), strips.size(), output);
        fclose(output);

        free(faces);
        free(attr_buf_ptr);
    }
*/
    return 0;   
}