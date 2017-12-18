#include <thread>
#include <glm/ext.hpp>

#include "tess.hpp"
#include "log.hpp"
#include "plato.hpp"

namespace tess
{
//=======================================================================================================================================================================================================================
// Single-threaded tesselation function
// V, E, F of any triangular subdivision satisfy : V - E + F = 2, 3F = 2E, F = 2V - 4, E = 3V - 6.
//=======================================================================================================================================================================================================================
template<typename vertex_t> vao_t generate_vao(const vertex_t* initial_vertices, GLuint V,
                                               const glm::uvec4* quads, GLuint Q,
                                               typename maps<vertex_t>::edge_tess_func edge_func,
                                               typename maps<vertex_t>::face_tess_func face_func,
                                               GLuint level)
{
    //===================================================================================================================================================================================================================
    // any mesh split into quads can be used as initial
    //===================================================================================================================================================================================================================
    GLuint E = Q + Q;

    glm::uvec2* edges = (glm::uvec2*) malloc(E * sizeof(glm::uvec2));
    GLuint* edge_indices = (GLuint*) malloc(V * V * sizeof(GLuint));

    GLuint e = 0;
    for(GLuint q = 0; q < Q; ++q)
    {
        GLuint A = quads[q].x;
        GLuint B = quads[q].y;
        GLuint C = quads[q].z;
        GLuint D = quads[q].w;
        if (A < B) edges[e++] = glm::uvec2(A, B);
        if (B < C) edges[e++] = glm::uvec2(B, C);
        if (C < D) edges[e++] = glm::uvec2(C, D);
        if (D < A) edges[e++] = glm::uvec2(D, A);
    }

    for(GLuint a = 0; a < V * V; ++a) edge_indices[a] = -1;

    for(GLuint e = 0; e < E; ++e)
    {
        edge_indices[edges[e].y + V * edges[e].x] = e;
        edge_indices[edges[e].x + V * edges[e].y] = e;
    }

    float inv_level = 1.0f / level;

    //===================================================================================================================================================================================================================
    // some simple combinatorics
    //===================================================================================================================================================================================================================
    GLuint vertices_per_quad = (level - 1) * (level - 1);
    GLuint indices_per_quad = (level + level + 3) * level;
    GLuint index_count = indices_per_quad * Q;
    GLuint total_V = V + (level - 1) * E + vertices_per_quad * Q;

    //===================================================================================================================================================================================================================
    // vertex and index buffer allocation
    //===================================================================================================================================================================================================================
    vertex_t* vertices = (vertex_t*) malloc(total_V * sizeof(vertex_t));
    GLuint* indices = (GLuint*) malloc(index_count * sizeof(GLuint));

    for(GLuint v = 0; v < V; ++v) vertices[v] = initial_vertices[v];
    GLuint vbo_index = V;
    GLuint ibo_index = 0;

    //===================================================================================================================================================================================================================
    // compute new vertices on edges
    //===================================================================================================================================================================================================================
    float level_inv = 1.0f / level;
    for (int e = 0; e < E; ++e)
    {
        GLuint A = edges[e].x;
        GLuint B = edges[e].y;
        for(int p = 1; p < level; ++p)
        {
            glm::vec2 uv = inv_level * glm::vec2(level - p, p);
            vertices[vbo_index++] = edge_func(vertices[A], vertices[B], uv);
        }
    }

    //===================================================================================================================================================================================================================
    // compute new vertices inside quads :
    // every quad (ABCD) is split into two triangles by the internal edge AC
    //===================================================================================================================================================================================================================
    for (int q = 0; q < Q; ++q)
    {
        GLuint A = quads[q].x;
        GLuint B = quads[q].y;
        GLuint C = quads[q].z;
        GLuint D = quads[q].w;

        GLuint edge_index_AB = V + (level - 1) * edge_indices[A * V + B];
        GLuint edge_index_BC = V + (level - 1) * edge_indices[B * V + C];
        GLuint edge_index_CD = V + (level - 1) * edge_indices[C * V + D];
        GLuint edge_index_DA = V + (level - 1) * edge_indices[D * V + A];

        for (GLint v = 1; v <= level - 1; ++v)
        {
            for (GLint u = 1; u <= level - 1; ++u)
            {
                if (v >= u)
                {
                    glm::vec3 uvw = inv_level * glm::vec3(level - v, u, v - u);
                    vertices[vbo_index++] = face_func(vertices[A], vertices[C], vertices[D], uvw);
                }
                else
                {
                    glm::vec3 uvw = inv_level * glm::vec3(level - u, u - v, v);
                    vertices[vbo_index++] = face_func(vertices[A], vertices[B], vertices[C], uvw);
                }
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
    vao_t vao;
    vao.init(GL_TRIANGLE_STRIP, vertices, total_V, indices, index_count);

    free(edges);
    free(edge_indices);
    free(vertices);
    free(indices);

    return vao;
}

template vao_t generate_vao<vertex_pn_t>  (const vertex_pn_t*,   GLuint, const glm::uvec4*, GLuint, typename maps<vertex_pn_t>::edge_tess_func,   typename maps<vertex_pn_t>::face_tess_func,   GLuint);
template vao_t generate_vao<vertex_pno_t> (const vertex_pno_t*,  GLuint, const glm::uvec4*, GLuint, typename maps<vertex_pno_t>::edge_tess_func,  typename maps<vertex_pno_t>::face_tess_func,  GLuint);
template vao_t generate_vao<vertex_pnoh_t>(const vertex_pnoh_t*, GLuint, const glm::uvec4*, GLuint, typename maps<vertex_pnoh_t>::edge_tess_func, typename maps<vertex_pnoh_t>::face_tess_func, GLuint);


//=======================================================================================================================================================================================================================
// Multithreaded mesh tesselation : provided functions should compute vertex attributes given barycentric coordinates on edges/faces respectively
// V, E, F of any triangular subdivision satisfy : V - E + F = 2, 3F = 2E, F = 2V - 4, E = 3V - 6.
//=======================================================================================================================================================================================================================
template<typename vertex_t, int threads> vao_t generate_vao_mt(const vertex_t* initial_vertices, GLuint V,
                                               const glm::uvec4* quads, GLuint Q,
                                               typename maps<vertex_t>::edge_tess_func edge_func,
                                               typename maps<vertex_t>::face_tess_func face_func,
                                               GLuint level)
{
    //===================================================================================================================================================================================================================
    // prepare data to be read by all threads
    //===================================================================================================================================================================================================================
    compute_data<vertex_t> data;
    data.level = level;
    data.V = V;
    data.E = Q + Q;
    data.Q = Q;
    data.edges = (glm::uvec2*) malloc(data.E * sizeof(glm::uvec2));
    data.edge_indices = (GLuint*) malloc(V * V * sizeof(GLuint));
    data.quads = quads;

    //===================================================================================================================================================================================================================
    // compute auxiliary edge data
    //===================================================================================================================================================================================================================
    GLuint E = Q + Q;
    GLuint e = 0;
    for(GLuint q = 0; q < Q; ++q)
    {
        GLuint A = quads[q].x;
        GLuint B = quads[q].y;
        GLuint C = quads[q].z;
        GLuint D = quads[q].w;
        if (A < B) data.edges[e++] = glm::uvec2(A, B);
        if (B < C) data.edges[e++] = glm::uvec2(B, C);
        if (C < D) data.edges[e++] = glm::uvec2(C, D);
        if (D < A) data.edges[e++] = glm::uvec2(D, A);
    }

    for(GLuint a = 0; a < V * V; ++a) data.edge_indices[a] = -1;
    for(GLuint e = 0; e < E; ++e)
    {
        data.edge_indices[data.edges[e].y + V * data.edges[e].x] = e;
        data.edge_indices[data.edges[e].x + V * data.edges[e].y] = e;
    }

    float inv_level = 1.0f / level;

    //===================================================================================================================================================================================================================
    // compute this elementary combinatorial part here to avoid recalculating the same stuff by all threads
    //===================================================================================================================================================================================================================
    data.vertices_per_quad = (level - 1) * (level - 1);
    data.indices_per_quad = (level + level + 3) * level;
    data.quad_vertices_base_index = data.V + (level - 1) * data.E;

    GLuint vertex_count = data.V + (level - 1) * data.E + data.vertices_per_quad * data.Q;
    GLuint index_count = data.indices_per_quad * data.Q;

    debug_msg("Multi-threaded tesselation of quad surfaces : V = %d. E = %d. Q = %d. index_count = %u. total_V = %d.", data.V, data.E, data.Q, index_count, V);

    //===================================================================================================================================================================================================================
    // vertex and index buffer allocation
    //===================================================================================================================================================================================================================
    data.vertices = (vertex_t*) malloc(vertex_count * sizeof(vertex_t));
    data.indices  = (GLuint*) malloc(index_count * sizeof(GLuint));

    for(GLuint v = 0; v < data.V; ++v) data.vertices[v] = initial_vertices[v];

    //===================================================================================================================================================================================================================
    // run threads that compute their own chunks of vertices on edges and faces
    // in case the number of edges/faces is not divisible by the number of threads, threads that are launched first will compute a bit more
    //===================================================================================================================================================================================================================
    std::thread computation_thread[threads - 1];

    GLuint edges_per_thread = data.E / threads;
    GLuint extra_edges = data.E % threads;
    GLuint quads_per_thread = data.Q / threads;
    GLuint extra_quads = data.Q % threads;
    GLuint edge_start = 0;
    GLuint quad_start = 0;

    for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
    {
        GLuint edge_end = edge_start + edges_per_thread + GLint(thread_id < extra_edges);
        GLuint quad_end = quad_start + quads_per_thread + GLint(thread_id < extra_quads);
        debug_msg("Launching thread #%u. Edges to compute : [%u, %u]. Faces to compute : [%u, %u]", thread_id, edge_start, edge_end - 1, quad_start, quad_end - 1);
        computation_thread[thread_id] = std::thread(fill_vao_chunk<vertex_t>, data, edge_func, face_func, edge_start, edge_end, quad_start, quad_end);
        edge_start = edge_end;
        quad_start = quad_end;
    }

    //===================================================================================================================================================================================================================
    // this thread will do the last task and will wait for others to finish
    //===================================================================================================================================================================================================================
    debug_msg("Main thread #%u. Edges to compute : [%u, %u]. Faces to compute : [%u, %u]", threads - 1, edge_start, edge_start + edges_per_thread - 1, quad_start, quad_start + quads_per_thread - 1);
    fill_vao_chunk<vertex_t>(data, edge_func, face_func, edge_start, edge_start + edges_per_thread, quad_start, quad_start + quads_per_thread);

    for (int thread_id = 0; thread_id < threads - 1; ++thread_id)
    {
        computation_thread[thread_id].join();
        debug_msg("Thread #%u joined the main thread.", thread_id);
    }

    //===================================================================================================================================================================================================================
    // create VAO
    //===================================================================================================================================================================================================================
    vao_t vao;
    vao.init(GL_TRIANGLE_STRIP, data.vertices, vertex_count, data.indices, index_count);

    free(data.edges);
    free(data.edge_indices);
    free(data.vertices);
    free(data.indices);

    return vao;
}

template vao_t generate_vao_mt<vertex_pnoh_t>(const vertex_pnoh_t*, GLuint, const glm::uvec4*, GLuint, typename maps<vertex_pnoh_t>::edge_tess_func, typename maps<vertex_pnoh_t>::face_tess_func, GLuint);
template vao_t generate_vao_mt<vertex_pno_t> (const vertex_pno_t*,  GLuint, const glm::uvec4*, GLuint, typename maps<vertex_pno_t>::edge_tess_func,  typename maps<vertex_pno_t>::face_tess_func,  GLuint);
template vao_t generate_vao_mt<vertex_pn_t>  (const vertex_pn_t*,   GLuint, const glm::uvec4*, GLuint, typename maps<vertex_pn_t>::edge_tess_func,   typename maps<vertex_pn_t>::face_tess_func,   GLuint);

//=======================================================================================================================================================================================================================
// Auxiliary function that populates its own chunk of vertex and index buffers
//=======================================================================================================================================================================================================================
template<typename vertex_t> void fill_vao_chunk(const compute_data<vertex_t>& data,
                                                typename maps<vertex_t>::edge_tess_func edge_func,
                                                typename maps<vertex_t>::face_tess_func face_func,
                                                GLuint edge_start, GLuint edge_end,
                                                GLuint quad_start, GLuint quad_end)
{
    float inv_level = 1.0f / data.level;
    //===================================================================================================================================================================================================================
    // Compute new vertices on edges
    //===================================================================================================================================================================================================================
    for (GLuint e = edge_start; e < edge_end; ++e)
    {
        GLuint A = data.edges[e].x;
        GLuint B = data.edges[e].y;
        GLuint vbo_index = data.V + (data.level - 1) * data.edge_indices[A * data.V + B];

        for(GLint p = 1; p < data.level; ++p)
        {
            glm::vec2 uv = inv_level * glm::vec2(data.level - p, p);
            data.vertices[vbo_index++] = edge_func(data.vertices[A], data.vertices[B], uv);
        }
    }

    //===================================================================================================================================================================================================================
    // Compute new vertices inside quads : every quad (ABCD) is split into two triangles by the internal edge AC
    //===================================================================================================================================================================================================================
    GLuint vbo_index = data.quad_vertices_base_index + quad_start * data.vertices_per_quad;
    GLuint ibo_index = data.indices_per_quad * quad_start;

    for (GLuint q = quad_start; q < quad_end; ++q)
    {
        GLuint A = data.quads[q].x;
        GLuint B = data.quads[q].y;
        GLuint C = data.quads[q].z;
        GLuint D = data.quads[q].w;

        GLuint edge_indexAB = data.V + (data.level - 1) * data.edge_indices[A * data.V + B];
        GLuint edge_indexBC = data.V + (data.level - 1) * data.edge_indices[B * data.V + C];
        GLuint edge_indexCD = data.V + (data.level - 1) * data.edge_indices[C * data.V + D];
        GLuint edge_indexDA = data.V + (data.level - 1) * data.edge_indices[D * data.V + A];

        for (GLint v = 1; v <= data.level - 1; ++v)
        {
            for (GLint u = 1; u <= data.level - 1; ++u)
            {
                if (v >= u)
                {
                    glm::vec3 uvw = inv_level * glm::vec3(data.level - v, u, v - u);
                    data.vertices[vbo_index++] = face_func(data.vertices[A], data.vertices[C], data.vertices[D], uvw);
                }
                else
                {
                    glm::vec3 uvw = inv_level * glm::vec3(data.level - u, u - v, v);
                    data.vertices[vbo_index++] = face_func(data.vertices[A], data.vertices[B], data.vertices[C], uvw);
                }
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

} // namespace tess