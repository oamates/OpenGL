#include <thread>
#include <glm/ext.hpp>

#include "sphere.hpp"
#include "log.hpp"
#include "plato.hpp"

#define ICOSAHEDRAL_SUBDIVISION

//=======================================================================================================================================================================================================================
// Iterative spherical topology surface generation procedure :
// on each step vertex position is displaced along the current normal by the provided "noise" function of the current
// after that the normal is recalculated and the algorithm proceeds to the next level
//=======================================================================================================================================================================================================================
/*
void sphere_t::fractal_surface(spheric_landscape_func func, int level)
{
    //===================================================================================================================================================================================================================
    // icosahedron has similar structures defined and can be used instead of cube
    //===================================================================================================================================================================================================================
    int V = plato::icosahedron::V;
    int E = plato::cube::E;
    int Q = plato::cube::Q;
    const glm::ivec2* edges = plato::cube::edges;
    const int* edge_indices = plato::cube::edge_indices;
    const glm::ivec4* quads = plato::cube::quads;
    const glm::vec3* initial_vertices = plato::cube::vertices;



}
*/


//=======================================================================================================================================================================================================================
// Single-threaded subdivision of sphere : provided function computes position-normal-texture coordinate vertex attributes
// V, E, F of any triangular subdivision satisfy : V - E + F = 2, 3F = 2E, F = 2V - 4, E = 3V - 6.
//=======================================================================================================================================================================================================================
template<typename vertex_t> void sphere_t::generate_vao(typename maps<vertex_t>::spheric_func func, int level)
{
    //===================================================================================================================================================================================================================
    // either icosahedron or cube can be used for initial subdivision of sphere
    //===================================================================================================================================================================================================================
  #ifndef ICOSAHEDRAL_SUBDIVISION
    int V = plato::cube::V;
    int E = plato::cube::E;
    int Q = plato::cube::Q;
    const glm::ivec2* edges = plato::cube::edges;
    const int* edge_indices = plato::cube::edge_indices;
    const glm::ivec4* quads = plato::cube::quads;
    const glm::vec3* initial_vertices = plato::cube::vertices;
  #else
    int V = plato::icosahedron::V;
    int E = plato::icosahedron::E;
    int Q = plato::icosahedron::Q;
    const glm::ivec2* edges = plato::icosahedron::edges;
    const int* edge_indices = plato::icosahedron::edge_indices;
    const glm::ivec4* quads = plato::icosahedron::quads;
    const glm::vec3* initial_vertices = plato::icosahedron::vertices;
  #endif
    //===================================================================================================================================================================================================================
    // some simple combinatorics
    //===================================================================================================================================================================================================================
    int vertices_per_quad = (level - 1) * (level - 1);
    int indices_per_quad = (level + level + 3) * level;
    GLuint index_count = indices_per_quad * Q;
    int total_V = V + (level - 1) * E + vertices_per_quad * Q;

    debug_msg("Single-threaded subdivision of sphere : V = %d. E = %d. Q = %d. index_count = %u. total_V = %d.", V, E, Q, index_count, total_V);

    //===================================================================================================================================================================================================================
    // vertex and index buffer allocation
    //===================================================================================================================================================================================================================
    vertex_t* vertices = (vertex_t*) malloc(total_V * sizeof(vertex_t));
    GLuint* indices = (GLuint*) malloc(index_count * sizeof(GLuint));

    for(int v = 0; v < V; ++v) vertices[v] = func(initial_vertices[v]);
    int vbo_index = V;
    int ibo_index = 0;

    //===================================================================================================================================================================================================================
    // compute new vertices on edges
    //===================================================================================================================================================================================================================
    float level_inv = 1.0f / level;
    for (int e = 0; e < E; ++e)
    {
        int A = edges[e].x;
        int B = edges[e].y;
        glm::vec3 direction = vertices[A].uvw;
        glm::vec3 delta = level_inv * (vertices[B].uvw - vertices[A].uvw);
        for(int p = 1; p < level; ++p)
        {
            direction += delta;
            vertices[vbo_index++] = func(direction);
        }
    }

    //===================================================================================================================================================================================================================
    // compute new vertices inside quads :
    // every quad (ABCD) is split into two triangles by the internal edge AC
    //===================================================================================================================================================================================================================
    for (int q = 0; q < Q; ++q)
    {
        int A = quads[q].x;
        int B = quads[q].y;
        int C = quads[q].z;
        int D = quads[q].w;

        glm::vec3 vertex_A = vertices[A].uvw;
        glm::vec3 vertex_B = vertices[B].uvw;
        glm::vec3 vertex_C = vertices[C].uvw;
        glm::vec3 vertex_D = vertices[D].uvw;

        int edge_index_AB = V + (level - 1) * edge_indices[A * V + B];
        int edge_index_BC = V + (level - 1) * edge_indices[B * V + C];
        int edge_index_CD = V + (level - 1) * edge_indices[C * V + D];
        int edge_index_DA = V + (level - 1) * edge_indices[D * V + A];

        for (GLint v = 1; v <= level - 1; ++v)
        {
            for (GLint u = 1; u <= level - 1; ++u)
            {
                glm::vec3 direction = (v >= u) ? (u * vertex_C + (level - v) * vertex_A + (v - u) * vertex_D): // the point is inside ACD or on the diagonal AC
                                                 (v * vertex_C + (level - u) * vertex_A + (u - v) * vertex_B); // the point is inside ABC
                vertices[vbo_index++] = func(direction);
            }
        }

        vbo_index -= vertices_per_quad;

        //===============================================================================================================================================================================================================
        // triangle strip near the edge AB
        //===============================================================================================================================================================================================================
        indices[ibo_index++] = (A < D) ? edge_index_DA : edge_index_DA + level - 2;
        indices[ibo_index++] = A;

        for (GLint p = 1; p < level; ++p)
        {
            indices[ibo_index++] = vbo_index + p - 1;
            indices[ibo_index++] = (A < B) ? edge_index_AB + p - 1: edge_index_AB + level - p - 1;
        }

        indices[ibo_index++] = (B < C) ? edge_index_BC : edge_index_BC + level - 2;
        indices[ibo_index++] = B;
        indices[ibo_index++] = -1;                                                                          // complete the strip with the primitive restart index

        //===============================================================================================================================================================================================================
        // triangle strips inside the quad
        //===============================================================================================================================================================================================================
        for (GLint p = 1; p < level - 1; ++p)
        {
            indices[ibo_index++] = (A < D) ? edge_index_DA + p : edge_index_DA + level - 2 - p;
            indices[ibo_index++] = (A < D) ? edge_index_DA + p - 1 : edge_index_DA + level - p - 1;

            for (GLint r = 1; r < level; ++r)
            {
                indices[ibo_index++] = vbo_index + level - 1;
                indices[ibo_index++] = vbo_index++;
            }

            indices[ibo_index++] = (B < C) ? edge_index_BC + p : edge_index_BC + level - 2 - p;
            indices[ibo_index++] = (B < C) ? edge_index_BC + p - 1 : edge_index_BC + level - p - 1;
            indices[ibo_index++] = -1;                                                                      // complete the strip with the primitive restart index
        }

        //===============================================================================================================================================================================================================
        // triangle strip near the edge CD
        //===============================================================================================================================================================================================================
        indices[ibo_index++] = D;
        indices[ibo_index++] = (A < D) ? edge_index_DA + level - 2 : edge_index_DA;

        for (GLint p = 1; p < level; ++p)
        {
            indices[ibo_index++] = (C < D) ? edge_index_CD + level - p - 1 : edge_index_CD + p - 1;
            indices[ibo_index++] = vbo_index++;
        }

        indices[ibo_index++] = C;
        indices[ibo_index++] = (B < C) ? edge_index_BC + level - 2 : edge_index_BC;
        indices[ibo_index++] = -1;                                                                          // complete the strip with the primitive restart index
    }

    //===================================================================================================================================================================================================================
    // create VAO
    //===================================================================================================================================================================================================================
    vao.init(GL_TRIANGLE_STRIP, vertices, total_V, indices, index_count);

    free(vertices);
    free(indices);
}

//=======================================================================================================================================================================================================================
// Multithreaded subdivision of sphere : provided function computes position-normal-texture coordinate vertex attributes
// V, E, F of any triangular subdivision satisfy : V - E + F = 2, 3F = 2E, F = 2V - 4, E = 3V - 6.
//=======================================================================================================================================================================================================================
template<typename vertex_t, int threads> void sphere_t::generate_vao_mt(typename maps<vertex_t>::spheric_func func, int level)
{
    //===================================================================================================================================================================================================================
    // prepare data to be read by all threads
    //===================================================================================================================================================================================================================
    compute_data<vertex_t> data;
    data.level = level;

    //===================================================================================================================================================================================================================
    // icosahedron has similar structures defined and can be used instead of cube
    //===================================================================================================================================================================================================================
  #ifndef ICOSAHEDRAL_SUBDIVISION
    data.V = plato::cube::V;
    data.E = plato::cube::E;
    data.Q = plato::cube::Q;
    data.edges = plato::cube::edges;
    data.edge_indices = plato::cube::edge_indices;
    data.quads = plato::cube::quads;
    const glm::vec3* initial_vertices = plato::cube::vertices;
  #else
    data.V = plato::icosahedron::V;
    data.E = plato::icosahedron::E;
    data.Q = plato::icosahedron::Q;
    data.edges = plato::icosahedron::edges;
    data.edge_indices = plato::icosahedron::edge_indices;
    data.quads = plato::icosahedron::quads;
    const glm::vec3* initial_vertices = plato::icosahedron::vertices;
  #endif

    //===================================================================================================================================================================================================================
    // compute this elementary combinatorial part here to avoid recalculating the same stuff by all threads
    //===================================================================================================================================================================================================================
    data.vertices_per_quad = (level - 1) * (level - 1);
    data.indices_per_quad = (level + level + 3) * level;
    data.quad_vertices_base_index = data.V + (level - 1) * data.E;
    int V = data.V + (level - 1) * data.E + data.vertices_per_quad * data.Q;
    GLuint index_count = data.indices_per_quad * data.Q;

    debug_msg("Multi-threaded subdivision of sphere : V = %d. E = %d. Q = %d. index_count = %u. total_V = %d.", data.V, data.E, data.Q, index_count, V);

    //===================================================================================================================================================================================================================
    // vertex and index buffer allocation
    //===================================================================================================================================================================================================================
    vertex_t* vertices = (vertex_t*) malloc(V * sizeof(vertex_t));
    GLuint* indices = (GLuint*) malloc(index_count * sizeof(GLuint));
    data.vertices = &vertices[0];
    data.indices  = &indices[0];

    for(GLuint v = 0; v < data.V; ++v) data.vertices[v] = func(initial_vertices[v]);

    //===================================================================================================================================================================================================================
    // run threads that compute their own chunks of vertices on edges and faces
    // in case the number of edges/faces is not divisible by the number of threads, threads that are launched first will compute a bit more
    //===================================================================================================================================================================================================================
    std::thread computation_thread[threads - 1];

    int edges_per_thread = data.E / threads;
    int extra_edges = data.E % threads;
    int quads_per_thread = data.Q / threads;
    int extra_quads = data.Q % threads;
    int edge_start = 0;
    int quad_start = 0;

    for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
    {
        GLuint edge_end = edge_start + edges_per_thread + GLint(thread_id < extra_edges);
        GLuint quad_end = quad_start + quads_per_thread + GLint(thread_id < extra_quads);
        debug_msg("Launching thread #%u. Edges to compute : [%u, %u]. Faces to compute : [%u, %u]", thread_id, edge_start, edge_end - 1, quad_start, quad_end - 1);
        computation_thread[thread_id] = std::thread(sphere_t::fill_vao_chunk<vertex_t>, func, data, edge_start, edge_end, quad_start, quad_end);
        edge_start = edge_end;
        quad_start = quad_end;
    }

    //===================================================================================================================================================================================================================
    // this thread will do the last task and will wait for others to finish
    //===================================================================================================================================================================================================================
    debug_msg("Main thread #%u. Edges to compute : [%u, %u]. Faces to compute : [%u, %u]", threads - 1, edge_start, edge_start + edges_per_thread - 1, quad_start, quad_start + quads_per_thread - 1);
    fill_vao_chunk<vertex_t>(func, data, edge_start, edge_start + edges_per_thread, quad_start, quad_start + quads_per_thread);

    for (int thread_id = 0; thread_id < threads - 1; ++thread_id)
    {
        computation_thread[thread_id].join();
        debug_msg("Thread #%u joined the main thread.", thread_id);
    }

    //===================================================================================================================================================================================================================
    // create VAO
    //===================================================================================================================================================================================================================
    vao.init(GL_TRIANGLE_STRIP, vertices, V, indices, index_count);

    free(vertices);
    free(indices);
}

template void sphere_t::generate_vao_mt<vertex_pnt3_t>(typename maps<vertex_pnt3_t>::spheric_func func, int level);

//=======================================================================================================================================================================================================================
// Multithreaded subdivision of sphere : function creates quad list subdivision buffers
// to be used with tesselation and rendered with primitive type = GL_PATCHES
//=======================================================================================================================================================================================================================
template<typename vertex_t, int threads> void sphere_t::generate_quads_mt(typename maps<vertex_t>::spheric_func func, int level)
{
    //===================================================================================================================================================================================================================
    // prepare data to be read by all threads
    //===================================================================================================================================================================================================================
    compute_data<vertex_t> data;
    data.level = level;

    //===================================================================================================================================================================================================================
    // icosahedron has similar structures defined and can be used instead of cube
    //===================================================================================================================================================================================================================
  #ifndef ICOSAHEDRAL_SUBDIVISION
    data.V = plato::cube::V;
    data.E = plato::cube::E;
    data.Q = plato::cube::Q;
    data.edges = plato::cube::edges;
    data.edge_indices = plato::cube::edge_indices;
    data.quads = plato::cube::quads;
    const glm::vec3* initial_vertices = plato::cube::vertices;
  #else
    data.V = plato::icosahedron::V;
    data.E = plato::icosahedron::E;
    data.Q = plato::icosahedron::Q;
    data.edges = plato::icosahedron::edges;
    data.edge_indices = plato::icosahedron::edge_indices;
    data.quads = plato::icosahedron::quads;
    const glm::vec3* initial_vertices = plato::icosahedron::vertices;
  #endif

    //===================================================================================================================================================================================================================
    // compute this elementary combinatorial part here to avoid recalculating the same stuff by all threads
    //===================================================================================================================================================================================================================
    data.vertices_per_quad = (level - 1) * (level - 1);
    data.indices_per_quad = 4 * level * level;
    data.quad_vertices_base_index = data.V + (level - 1) * data.E;
    int V = data.V + (level - 1) * data.E + data.vertices_per_quad * data.Q;
    GLuint index_count = data.indices_per_quad * data.Q;

    debug_msg("Multi-threaded quad subdivision generation of sphere : V = %d. E = %d. Q = %d. index_count = %u. total_V = %d.", data.V, data.E, data.Q, index_count, V);

    //===================================================================================================================================================================================================================
    // vertex and index buffer allocation
    //===================================================================================================================================================================================================================
    vertex_t* vertices = (vertex_t*) malloc(V * sizeof(vertex_t));
    GLuint* indices = (GLuint*) malloc(index_count * sizeof(GLuint));
    data.vertices = &vertices[0];
    data.indices  = &indices[0];

    for(GLuint v = 0; v < data.V; ++v) data.vertices[v] = func(initial_vertices[v]);

    //===================================================================================================================================================================================================================
    // run threads that compute their own chunks of vertices on edges and faces
    // in case the number of edges/faces is not divisible by the number of threads, threads that are launched first will compute a bit more
    //===================================================================================================================================================================================================================
    std::thread computation_thread[threads - 1];

    int edges_per_thread = data.E / threads;
    int extra_edges = data.E % threads;
    int quads_per_thread = data.Q / threads;
    int extra_quads = data.Q % threads;
    int edge_start = 0;
    int quad_start = 0;

    for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
    {
        GLuint edge_end = edge_start + edges_per_thread + GLint(thread_id < extra_edges);
        GLuint quad_end = quad_start + quads_per_thread + GLint(thread_id < extra_quads);
        debug_msg("Launching thread #%u. Edges to compute : [%u, %u]. Faces to compute : [%u, %u]", thread_id, edge_start, edge_end - 1, quad_start, quad_end - 1);
        computation_thread[thread_id] = std::thread(sphere_t::fill_quad_chunk<vertex_t>, func, data, edge_start, edge_end, quad_start, quad_end);
        edge_start = edge_end;
        quad_start = quad_end;
    }

    //===================================================================================================================================================================================================================
    // this thread will do the last task and will wait for others to finish
    //===================================================================================================================================================================================================================
    debug_msg("Main thread #%u. Edges to compute : [%u, %u]. Faces to compute : [%u, %u]", threads - 1, edge_start, edge_start + edges_per_thread - 1, quad_start, quad_start + quads_per_thread - 1);
    fill_quad_chunk<vertex_t>(func, data, edge_start, edge_start + edges_per_thread, quad_start, quad_start + quads_per_thread);

    for (int thread_id = 0; thread_id < threads - 1; ++thread_id)
    {
        computation_thread[thread_id].join();
        debug_msg("Thread #%u joined the main thread.", thread_id);
    }

    //===================================================================================================================================================================================================================
    // create VAO
    //===================================================================================================================================================================================================================
    vao.init(GL_PATCHES, vertices, V, indices, index_count);

    free(vertices);
    free(indices);
}

template void sphere_t::generate_quads_mt<vertex_t3_t>(typename maps<vertex_t3_t>::spheric_func func, int level);

//=======================================================================================================================================================================================================================
// Rendering functions
//=======================================================================================================================================================================================================================
void sphere_t::render()
    { vao.render(); }

void sphere_t::render(GLenum override_mode)
    { vao.render(override_mode); }

void sphere_t::render(GLsizei count, const GLvoid* offset)
    { vao.render(count, offset); }

void sphere_t::instanced_render(GLsizei primcount)
    { vao.instanced_render(primcount); }

//=======================================================================================================================================================================================================================
// Auxiliary function that populates its own chunk of vertex and index buffers for triangle strip primitive type
//=======================================================================================================================================================================================================================
template<typename vertex_t> void sphere_t::fill_vao_chunk(typename maps<vertex_t>::spheric_func func, const compute_data<vertex_t>& data, GLuint edge_start, GLuint edge_end, GLuint quad_start, GLuint quad_end)
{
    //===================================================================================================================================================================================================================
    // Compute new vertices on edges
    //===================================================================================================================================================================================================================
    for (GLuint e = edge_start; e < edge_end; ++e)
    {
        GLuint A = data.edges[e].x;
        GLuint B = data.edges[e].y;

        glm::vec3 direction = data.vertices[A].uvw;
        glm::vec3 delta = (data.vertices[B].uvw - data.vertices[A].uvw) / data.level;

        GLuint vbo_index = data.V + (data.level - 1) * data.edge_indices[A * data.V + B];
        for(GLint p = 1; p < data.level; ++p)
        {
            direction += delta;
            data.vertices[vbo_index++] = func(direction);
        }
    }

    //===================================================================================================================================================================================================================
    // Compute new vertices inside quads : every quad (ABCD) is split into two triangles by the internal edge AC
    //===================================================================================================================================================================================================================
    GLuint vbo_index = data.quad_vertices_base_index + quad_start * data.vertices_per_quad;
    GLuint ibo_index = data.indices_per_quad * quad_start;

    for (GLuint q = quad_start; q < quad_end; ++q)
    {
        GLint A = data.quads[q].x;
        GLint B = data.quads[q].y;
        GLint C = data.quads[q].z;
        GLint D = data.quads[q].w;

        glm::vec3 vertexA = data.vertices[A].uvw;
        glm::vec3 vertexB = data.vertices[B].uvw;
        glm::vec3 vertexC = data.vertices[C].uvw;
        glm::vec3 vertexD = data.vertices[D].uvw;

        GLuint edge_indexAB = data.V + (data.level - 1) * data.edge_indices[A * data.V + B];
        GLuint edge_indexBC = data.V + (data.level - 1) * data.edge_indices[B * data.V + C];
        GLuint edge_indexCD = data.V + (data.level - 1) * data.edge_indices[C * data.V + D];
        GLuint edge_indexDA = data.V + (data.level - 1) * data.edge_indices[D * data.V + A];

        for (GLint v = 1; v <= data.level - 1; ++v)
        {
            for (GLint u = 1; u <= data.level - 1; ++u)
            {
                glm::vec3 direction = (v >= u) ? (u * vertexC + (data.level - v) * vertexA + (v - u) * vertexD): // the point is inside ACD or on the diagonal AC
                                                 (v * vertexC + (data.level - u) * vertexA + (u - v) * vertexB); // the point is inside ABC
                data.vertices[vbo_index++] = func(direction);
            }
        }

        vbo_index -= data.vertices_per_quad;

        //===============================================================================================================================================================================================================
        // triangle strip near the edge AB
        //===============================================================================================================================================================================================================
        data.indices[ibo_index++] = (A < D) ? edge_indexDA : edge_indexDA + data.level - 2;
        data.indices[ibo_index++] = A;

        for (GLint p = 1; p < data.level; ++p)
        {
            data.indices[ibo_index++] = vbo_index + p - 1;
            data.indices[ibo_index++] = (A < B) ? edge_indexAB + p - 1: edge_indexAB + data.level - p - 1;
        }

        data.indices[ibo_index++] = (B < C) ? edge_indexBC : edge_indexBC + data.level - 2;
        data.indices[ibo_index++] = B;
        data.indices[ibo_index++] = -1;                                                                         // complete the strip with the primitive restart index

        //===============================================================================================================================================================================================================
        // triangle strips inside the quad
        //===============================================================================================================================================================================================================
        for (GLint p = 1; p < data.level - 1; ++p)
        {
            data.indices[ibo_index++] = (A < D) ? edge_indexDA + p : edge_indexDA + data.level - 2 - p;
            data.indices[ibo_index++] = (A < D) ? edge_indexDA + p - 1 : edge_indexDA + data.level - p - 1;

            for (GLint r = 1; r < data.level; ++r)
            {
                data.indices[ibo_index++] = vbo_index + data.level - 1;
                data.indices[ibo_index++] = vbo_index++;
            }

            data.indices[ibo_index++] = (B < C) ? edge_indexBC + p : edge_indexBC + data.level - 2 - p;
            data.indices[ibo_index++] = (B < C) ? edge_indexBC + p - 1 : edge_indexBC + data.level - p - 1;
            data.indices[ibo_index++] = -1;                                                                     // complete the strip with the primitive restart index
        }

        //===============================================================================================================================================================================================================
        // triangle strip near the edge CD
        //===============================================================================================================================================================================================================
        data.indices[ibo_index++] = D;
        data.indices[ibo_index++] = (A < D) ? edge_indexDA + data.level - 2 : edge_indexDA;

        for (GLint p = 1; p < data.level; ++p)
        {
            data.indices[ibo_index++] = (C < D) ? edge_indexCD + data.level - p - 1 : edge_indexCD + p - 1;
            data.indices[ibo_index++] = vbo_index++;
        }

        data.indices[ibo_index++] = C;
        data.indices[ibo_index++] = (B < C) ? edge_indexBC + data.level - 2 : edge_indexBC;
        data.indices[ibo_index++] = -1;                                                                         // complete the strip with the primitive restart index
    }
}

template void sphere_t::fill_vao_chunk(typename maps<vertex_pnt3_t>::spheric_func func, const compute_data<vertex_pnt3_t>& data, GLuint edge_start, GLuint edge_end, GLuint quad_start, GLuint quad_end);

//=======================================================================================================================================================================================================================
// Auxiliary function that populates its own chunk of vertex and index buffers for triangle strip primitive type
//=======================================================================================================================================================================================================================
template<typename vertex_t> void sphere_t::fill_quad_chunk(typename maps<vertex_t>::spheric_func func, const compute_data<vertex_t>& data, GLuint edge_start, GLuint edge_end, GLuint quad_start, GLuint quad_end)
{
    //===================================================================================================================================================================================================================
    // Compute new vertices on edges
    //===================================================================================================================================================================================================================
    for (GLuint e = edge_start; e < edge_end; ++e)
    {
        GLuint A = data.edges[e].x;
        GLuint B = data.edges[e].y;

        glm::vec3 direction = data.vertices[A].uvw;
        glm::vec3 delta = (data.vertices[B].uvw - data.vertices[A].uvw) / data.level;

        GLuint vbo_index = data.V + (data.level - 1) * data.edge_indices[A * data.V + B];
        for(GLint p = 1; p < data.level; ++p)
        {
            direction += delta;
            data.vertices[vbo_index++] = func(direction);
        }
    }

    //===================================================================================================================================================================================================================
    // Compute new vertices inside quads : every quad (ABCD) is split into two triangles by the internal edge AC
    //===================================================================================================================================================================================================================
    GLuint vbo_index = data.quad_vertices_base_index + quad_start * data.vertices_per_quad;
    GLuint ibo_index = data.indices_per_quad * quad_start;

    for (GLuint q = quad_start; q < quad_end; ++q)
    {
        GLint A = data.quads[q].x;
        GLint B = data.quads[q].y;
        GLint C = data.quads[q].z;
        GLint D = data.quads[q].w;

        glm::vec3 vertexA = data.vertices[A].uvw;
        glm::vec3 vertexB = data.vertices[B].uvw;
        glm::vec3 vertexC = data.vertices[C].uvw;
        glm::vec3 vertexD = data.vertices[D].uvw;

        GLuint edge_indexAB = data.V + (data.level - 1) * data.edge_indices[A * data.V + B];
        GLuint edge_indexBC = data.V + (data.level - 1) * data.edge_indices[B * data.V + C];
        GLuint edge_indexCD = data.V + (data.level - 1) * data.edge_indices[C * data.V + D];
        GLuint edge_indexDA = data.V + (data.level - 1) * data.edge_indices[D * data.V + A];

        for (GLint v = 1; v <= data.level - 1; ++v)
        {
            for (GLint u = 1; u <= data.level - 1; ++u)
            {
                glm::vec3 direction = (v >= u) ? (u * vertexC + (data.level - v) * vertexA + (v - u) * vertexD): // the point is inside ACD or on the diagonal AC
                                                 (v * vertexC + (data.level - u) * vertexA + (u - v) * vertexB); // the point is inside ABC
                data.vertices[vbo_index++] = func(direction);
            }
        }

        vbo_index -= data.vertices_per_quad;

        //===============================================================================================================================================================================================================
        // triangle strip near the edge AB
        //===============================================================================================================================================================================================================
        data.indices[ibo_index++] = (A < D) ? edge_indexDA : edge_indexDA + data.level - 2;
        data.indices[ibo_index++] = A;

        for (GLint p = 1; p < data.level; ++p)
        {
            GLuint T = vbo_index + p - 1;
            GLuint S = (A < B) ? edge_indexAB + p - 1: edge_indexAB + data.level - p - 1;
            data.indices[ibo_index++] = S;
            data.indices[ibo_index++] = T;
            data.indices[ibo_index++] = T;
            data.indices[ibo_index++] = S;
        }

        data.indices[ibo_index++] = B;
        data.indices[ibo_index++] = (B < C) ? edge_indexBC : edge_indexBC + data.level - 2;

        //===============================================================================================================================================================================================================
        // triangle strips inside the quad
        //===============================================================================================================================================================================================================
        for (GLint p = 1; p < data.level - 1; ++p)
        {
            data.indices[ibo_index++] = (A < D) ? edge_indexDA + p : edge_indexDA + data.level - 2 - p;
            data.indices[ibo_index++] = (A < D) ? edge_indexDA + p - 1 : edge_indexDA + data.level - p - 1;

            for (GLint r = 1; r < data.level; ++r)
            {
                GLuint T = vbo_index + data.level - 1;
                GLuint S = vbo_index++;

                data.indices[ibo_index++] = S;
                data.indices[ibo_index++] = T;
                data.indices[ibo_index++] = T;
                data.indices[ibo_index++] = S;
            }

            data.indices[ibo_index++] = (B < C) ? edge_indexBC + p - 1 : edge_indexBC + data.level - p - 1;
            data.indices[ibo_index++] = (B < C) ? edge_indexBC + p : edge_indexBC + data.level - 2 - p;
        }

        //===============================================================================================================================================================================================================
        // triangle strip near the edge CD
        //===============================================================================================================================================================================================================
        data.indices[ibo_index++] = D;
        data.indices[ibo_index++] = (A < D) ? edge_indexDA + data.level - 2 : edge_indexDA;

        for (GLint p = 1; p < data.level; ++p)
        {
            GLuint T = (C < D) ? edge_indexCD + data.level - p - 1 : edge_indexCD + p - 1;
            GLuint S = vbo_index++;
            data.indices[ibo_index++] = S;
            data.indices[ibo_index++] = T;
            data.indices[ibo_index++] = T;
            data.indices[ibo_index++] = S;
        }

        data.indices[ibo_index++] = (B < C) ? edge_indexBC + data.level - 2 : edge_indexBC;
        data.indices[ibo_index++] = C;
    }
}

template void sphere_t::fill_quad_chunk(typename maps<vertex_t3_t>::spheric_func func, const compute_data<vertex_t3_t>& data, GLuint edge_start, GLuint edge_end, GLuint quad_start, GLuint quad_end);


//=======================================================================================================================================================================================================================
// Sphere is iteratively subdivided beginning from one of the regular plato solids, e.g. icosahedron.
// V,E,F of any triangular subdivision satisfy : V - E + F = 2, 3F = 2E, F = 2V - 4, E = 3V - 6.
// After n subdivision iterations V(n), E(n), F(n) take values :
// Fn = 4^n * F, En = 4^n * E, Vn = 4^n * (V - 2) + 2

//=======================================================================================================================================================================================================================
/*

void sphere_pnti::generate_landscape_vao(spherical_landscape_func func_l, int level)
{
    GLuint deg4 = 1 << (2 * level);
    GLuint V = deg4 * (plato::icosahedron::V - 2) + 2;
    GLuint E = deg4 * (plato::icosahedron::E);
    GLuint F = deg4 * (plato::icosahedron::F);
    GLuint triangles = F;

    debug_msg("V = %u. E = %u. F = %u.", V, E, F);

    GLuint VBO_SIZE = V * sizeof(glm::vec3);
    GLuint IBO_SIZE = F * sizeof(glm::ivec3);
    GLuint v, f;

    debug_msg("VBO_SIZE = %u. IBO_SIZE = %u.", VBO_SIZE, IBO_SIZE);

    std::unique_ptr<vertex_pnt[]> vertex_data(new vertex_pnt[V]);
    std::unique_ptr<glm::ivec3[]> indices(new glm::ivec3[F]);

    for(v = 0; v < plato::icosahedron::V; ++v)
    {
        vertex_data[v].uvw = plato::icosahedron::vertices[v];
        vertex_data[v].position = func_l(vertex_data[v].uvw, 0);
    }
    for(f = 0; f < plato::icosahedron::F; ++f) index_data[f] = plato::icosahedron::triangles[f];

    // ==================================================================================================================================================================================================================
    // vertices and texture coordinates
    // ==================================================================================================================================================================================================================

    for (GLuint l = 0; l < level; ++l)
    {
        GLuint end = f;
        std::map<uvec2_lex, GLuint> center_index;
        for (GLuint triangle = 0; triangle < end; ++triangle)
        {
            GLuint P = indices[triangle].x;
            GLuint Q = indices[triangle].y;
            GLuint R = indices[triangle].z;
            GLuint S, T, U;

            uvec2_lex PQ = (P < Q) ? uvec2_lex(P, Q) : uvec2_lex(Q, P);
            uvec2_lex QR = (Q < R) ? uvec2_lex(Q, R) : uvec2_lex(R, Q);
            uvec2_lex RP = (R < P) ? uvec2_lex(R, P) : uvec2_lex(P, R);

            std::map<uvec2_lex, GLuint>::iterator it = center_index.find(PQ);
            if (it != center_index.end()) S = it->second;
            else
            {
                S = v++;
                center_index[PQ] = S;
                vertices[S] = func(uvs[S] = glm::normalize(uvs[P] + uvs[Q]));

            }
            it = center_index.find(QR);
            if (it != center_index.end()) T = it->second;
            else
            {
                T = v++;
                center_index[QR] = T;
                vertices[T] = func(uvs[T] = glm::normalize(uvs[Q] + uvs[R]));


            }
            it = center_index.find(RP);
            if (it != center_index.end()) U = it->second;
            else
            {
                U = v++;
                center_index[RP] = U;
                vertices[U] = func(uvs[U] = glm::normalize(uvs[R] + uvs[P]));
            }

            indices[triangle] = glm::ivec3(S, T, U);
            indices[f++]      = glm::ivec3(P, S, U);
            indices[f++]      = glm::ivec3(Q, T, S);
            indices[f++]      = glm::ivec3(R, U, T);
        }
    }

    // ==================================================================================================================================================================================================================
    // vertices and texture coordinates
    // ==================================================================================================================================================================================================================
    for (f = 0; f < F; ++f)
    {
        glm::vec3 normal = glm::cross(vertices[indices[f].y] - vertices[indices[f].x], vertices[indices[f].z] - vertices[indices[f].x]);
        vertex_data.normals[indices[f].x] += normal;
        normals[indices[f].y] += normal;
        normals[indices[f].z] += normal;
    }

    for (v = 0; v < V; ++v) normals[v] = glm::normalize(normals[v]);


    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_pnt) * V, glm::value_ptr(vertices[0]), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pnt), (const GLvoid *) offsetof(vertex_pnt, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pnt), (const GLvoid *) offsetof(vertex_pnt, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pnt), (const GLvoid *) offsetof(vertex_pnt, pqr));

    glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IBO_SIZE, glm::value_ptr(indices[0]), GL_STATIC_DRAW);
}

*/