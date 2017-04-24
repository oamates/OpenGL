#include "log.hpp"
#include "surface.hpp"

//=======================================================================================================================================================================================================================
// Construct 2d surface (function graph) mesh given a full generating function --- which generates the whole vertex_pft/vertex_pnt structure                                                                         
//=======================================================================================================================================================================================================================
template<typename vertex_t> void surface_t::generate_vao(typename maps<vertex_t>::surface_func func, int size_x, int size_y)
{
    GLuint V = (size_x + 1) * (size_y + 1);
    vertex_t* vertices = (vertex_t*) malloc(V * sizeof(vertex_t));

    float delta_x = 1.0f / size_x;
    float delta_y = 1.0f / size_y;
    glm::vec2 uv = glm::vec2(0.0f, 0.0f);

    //===================================================================================================================================================================================================================
    // main calculation loop 
    //===================================================================================================================================================================================================================
    int index = 0;
    for (int v = 0; v <= size_y; ++v)
    {
        for (int u = 0; u <= size_x; ++u)
        {
            vertices[index++] = func(uv);           
            uv.x += delta_x;
        }
        uv.y += delta_y;
        uv.x = 0.0f;
    }
    
    //===================================================================================================================================================================================================================
    // Create VAO and fill the attribute buffer                                                                                                                                                                          
    //===================================================================================================================================================================================================================
    glGenVertexArrays(1, &vao.id);
    glBindVertexArray(vao.id);
    vao.vbo.init(vertices, V);

    //===================================================================================================================================================================================================================
    // Fill the index buffer                                                                                                                                                                                             
    //===================================================================================================================================================================================================================
    generate_ibo(size_x, size_y);
    free(vertices);
}

template void surface_t::generate_vao<vertex_pnt2_t>(typename maps<vertex_pnt2_t>::surface_func, int, int);
template void surface_t::generate_vao<vertex_pft2_t>(typename maps<vertex_pft2_t>::surface_func, int, int);

//=======================================================================================================================================================================================================================
// Auxiliary functions that generates index buffer                                                                                                                                                                        
//=======================================================================================================================================================================================================================
void surface_t::generate_ibo(int size_x, int size_y)
{
    GLuint V = (size_x + 1) * (size_y + 1);
    if (V >= 0xFFFF)
    {
        vao.ibo.type = GL_UNSIGNED_INT;
        generate_IBO<GLuint>(size_x, size_y);
    }
    else if (V >= 0xFF)
    {
        vao.ibo.type = GL_UNSIGNED_SHORT;
        generate_IBO<GLushort>(size_x, size_y);
    }
    else
    {
        vao.ibo.type = GL_UNSIGNED_BYTE;
        generate_IBO<GLubyte>(size_x, size_y);
    }
}

template<typename index_t> void surface_t::generate_IBO(int size_x, int size_y)
{
    int index = 0;
    GLuint index_count = size_y * (size_x + size_x + 3);
    index_t* indices = (index_t*) malloc (index_count * sizeof(index_t));
    index_t q = size_x + 1;
    index_t p = 0;
    for (int u = 0; u < size_y; ++u)
    {
        for (int v = 0; v <= size_x; ++v)
        {
            indices[index++] = q;
            indices[index++] = p;
            ++p; ++q;
        }
        indices[index++] = -1;
    }
    vao.ibo.init(GL_TRIANGLE_STRIP, indices, index_count);
}

template void surface_t::generate_IBO<GLuint>  (int, int);
template void surface_t::generate_IBO<GLushort>(int, int);
template void surface_t::generate_IBO<GLubyte> (int, int);

//=======================================================================================================================================================================================================================
// Construct 2d surface (function graph) mesh of a position function on unit square -1 <= u,v <= 1                                                                                                                       
// Texture coordinates of a generated vertex are set to the value of uv-parameter, tangent and normals are computed with finite difference derivative approximation.                                                 
//=======================================================================================================================================================================================================================
void surface_t::generate_pft2_vao(typename maps<glm::vec3>::surface_func func, int size_x, int size_y)
{
    GLuint V = (size_x + 1) * (size_y + 1);
    vertex_pft2_t* vertices = (vertex_pft2_t*) malloc(V * sizeof(vertex_pft2_t));
    
    float delta_x = 1.0f / size_x;
    float delta_y = 1.0f / size_y;
    
    glm::vec2 uv = glm::vec2(0.0f, 0.0f);
    glm::vec2 uv_pdx = glm::vec2(delta_x, 0.0f);
    glm::vec2 uv_pdy = glm::vec2(0.0f, delta_y);
    glm::vec2 uv_mdx = glm::vec2(-delta_x, 0.0f);
    glm::vec2 uv_mdy = glm::vec2(0.0f, -delta_y);
    
    //===================================================================================================================================================================================================================
    // main calculation loop                                                                                                                                                                                             
    //===================================================================================================================================================================================================================
    int index = 0;
    for (int v = 0; v <= size_y; ++v)
    {
        for (int u = 0; u <= size_x; ++u)
        {
            vertices[index].position = func(uv);          
            vertices[index].tangent_x = glm::normalize(func(uv_pdx) - func(uv_mdx));
            vertices[index].tangent_y = glm::normalize(func(uv_pdy) - func(uv_mdy));
            vertices[index].normal = glm::normalize (glm::cross(vertices[index].tangent_x, vertices[index].tangent_y));
            vertices[++index].uv = uv;
            uv.x += delta_x;
            uv_pdx.x += delta_x;
            uv_pdy.x += delta_x;
            uv_mdx.x += delta_x;
            uv_mdy.x += delta_x;
        }
        uv.y += delta_y;
        uv_pdx.x = delta_x;
        uv_pdy.y += delta_y;
        uv_mdx.x = -delta_x;
        uv_mdy.y += delta_y;
        uv.x = 0.0f;
    }
    
    //===================================================================================================================================================================================================================
    // Create VAO and fill the attribute buffer                                                                                                                                                                          
    //===================================================================================================================================================================================================================
    glGenVertexArrays(1, &vao.id);
    glBindVertexArray(vao.id);
    vao.vbo.init(vertices, V);

    //===================================================================================================================================================================================================================
    // Fill the index buffer                                                                                                                                                                                             
    //===================================================================================================================================================================================================================
    generate_ibo(size_x, size_y);
    free(vertices);
}

//=======================================================================================================================================================================================================================
// Construct 2d surface (function graph) mesh of a position function on unit square -1 <= u,v <= 1                                                                                                                       
// Texture coordinates of a generated vertex are set to the value of uv-parameter, normals are computed with finite difference derivative approximation                                                              
//=======================================================================================================================================================================================================================
void surface_t::generate_pnt2_vao(typename maps<glm::vec3>::surface_func func, int size_x, int size_y)
{
    GLuint V = (size_x + 1) * (size_y + 1);
    vertex_pnt2_t* vertices = (vertex_pnt2_t*) malloc(V * sizeof(vertex_pnt2_t));

    float delta_x = 1.0f / size_x;
    float delta_y = 1.0f / size_y;

    glm::vec2 uv = glm::vec2(0.0f, 0.0f);
    glm::vec2 uv_pdx = glm::vec2(delta_x, 0.0f);
    glm::vec2 uv_pdy = glm::vec2(0.0f, delta_y);
    glm::vec2 uv_mdx = glm::vec2(-delta_x, 0.0f);
    glm::vec2 uv_mdy = glm::vec2(0.0f, -delta_y);

    //===================================================================================================================================================================================================================
    // main calculation loop                                                                                                                                                                                             
    //===================================================================================================================================================================================================================
    int index = 0;
    for (int v = 0; v <= size_y; ++v)
    {
        for (int u = 0; u <= size_x; ++u)
        {
            vertices[index].position = func(uv);          
            vertices[index].normal = glm::normalize (glm::cross(func(uv_pdx) - func(uv_mdx), func(uv_pdy) - func(uv_mdy)));
            vertices[++index].uv = uv;
            uv.x += delta_x;
            uv_pdx.x += delta_x;
            uv_pdy.x += delta_x;
            uv_mdx.x += delta_x;
            uv_mdy.x += delta_x;
        }
        uv.y += delta_y;
        uv_pdx.x = delta_x;
        uv_pdy.y += delta_y;
        uv_mdx.x = -delta_x;
        uv_mdy.y += delta_y;
        uv.x = 0.0f;
    }
    
    //===================================================================================================================================================================================================================
    // Create VAO and fill the attribute buffer                                                                                                                                                                          
    //===================================================================================================================================================================================================================
    glGenVertexArrays(1, &vao.id);
    glBindVertexArray(vao.id);
    vao.vbo.init(vertices, V);

    //===================================================================================================================================================================================================================
    // Fill the index buffer                                                                                                                                                                                             
    //===================================================================================================================================================================================================================
    generate_ibo(size_x, size_y);
    free(vertices);
}

//=======================================================================================================================================================================================================================
// Rendering functions                                                                                                                                                                                                   
//=======================================================================================================================================================================================================================
void surface_t::render()
    { vao.render(); }

void surface_t::render(GLsizei count, const GLvoid* offset)
    { vao.render(count, offset); }

void surface_t::instanced_render(GLsizei primcount)
    { vao.instanced_render(primcount); }
