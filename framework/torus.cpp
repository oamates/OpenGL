#include <cassert>
#include <thread>

#include "torus.hpp"
#include "log.hpp"
#include "gcd.hpp"

//=======================================================================================================================================================================================================================
// construct 2d surface topologically equivalent to torus given a full generating function as a single triangle strip
// integers size_x and size_y must be coprime
//=======================================================================================================================================================================================================================
template<typename vertex_t> void torus_t::generate_vao_ss(typename maps<vertex_t>::toral_func func, unsigned int size_x, unsigned int size_y, adjacency_vao_t* adjacency_vao_ptr)
{
    assert(coprime(size_x, size_y) && "generate_vao_ss :: size_x and size_y must be coprime!");
}

//=======================================================================================================================================================================================================================
// Construct 2d surface topologically equivalent to torus given a full generating function
//=======================================================================================================================================================================================================================
template<typename vertex_t> void torus_t::generate_vao(typename maps<vertex_t>::toral_func func, int size_x, int size_y, adjacency_vao_t* adjacency_vao_ptr)
{
    GLuint V = size_x * size_y;
    GLuint index_count     = size_y * (2 * size_x + 3);
    GLuint adj_index_count = size_y * (4 * size_x + 5);

    float delta_x = 1.0f / float(size_x);
    float delta_y = 1.0f / float(size_y);

    //===================================================================================================================================================================================================================
    // allocate space for attribute and index buffers
    //===================================================================================================================================================================================================================
    vertex_t* vertices = (vertex_t*) malloc(V * sizeof(vertex_pft2_t));
    GLuint* indices = (GLuint*) malloc(adjacency_vao_ptr ? adj_index_count * sizeof(GLuint) : index_count * sizeof(GLuint));

    glm::vec2 uv;
    uv.y = 0.0f;

    int vbo_index = 0;
    int ibo_index = 0;


    for (int q = 0; q < size_y; ++q)
    {
        uv.x = 0.0f;
        GLuint next_row_index = vbo_index + size_x;
        if (next_row_index == V) next_row_index = 0;

        for (int p = 0; p < size_x; ++p)
        {
            //===========================================================================================================================================================================================================
            // add two indices to the strip, fill vertices with data
            //===========================================================================================================================================================================================================
            indices[ibo_index++] = vbo_index;
            indices[ibo_index++] = next_row_index;
            vertices[vbo_index++] = func(uv);
            uv.x += delta_x;
            ++next_row_index;
        }

        //===============================================================================================================================================================================================================
        // complete the strip with the last two indices and finish it with primitive restart index = -1
        //===============================================================================================================================================================================================================
        indices[ibo_index++] = vbo_index - size_x;
        indices[ibo_index++] = next_row_index - size_x;
        indices[ibo_index++] = -1;
        uv.y += delta_y;
    }

    vao.init(GL_TRIANGLE_STRIP, vertices, V, indices, index_count);

    if (adjacency_vao_ptr) 
    {
        //===============================================================================================================================================================================================================
        // assuming the vertex_t has position as its first attribute we use the same attribute buffer
        //===============================================================================================================================================================================================================
        glGenVertexArrays(1, &adjacency_vao_ptr->id);
        glBindVertexArray(adjacency_vao_ptr->id);    
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);


        //===============================================================================================================================================================================================================
        // generate the adjacency index buffer
        //===============================================================================================================================================================================================================
        int i0 = 0;
        int ibo_index = 0;

        for (int q = 0; q < size_y; ++q)
        {
            int i1 = i0 + size_x; if (i1 >= V) i1 = 0;
            int i2 = i1 + size_x; if (i2 >= V) i2 = 0;
            int im = (i0 < size_x) ? i0 - size_x + V: i0 - size_x;

            indices[ibo_index++] = i0++;
            indices[ibo_index++] = i1 + size_x - 1;
            indices[ibo_index++] = i1++;
            im++;

            for (int p = 1; p < size_x; ++p)
            {
                indices[ibo_index++] = im++;
                indices[ibo_index++] = i0++;
                indices[ibo_index++] = i2++;
                indices[ibo_index++] = i1++;
            }

            indices[ibo_index++] = im - size_x;
            indices[ibo_index++] = i0 - size_x;
            indices[ibo_index++] = i2++;
            indices[ibo_index++] = i1 - size_x;
            indices[ibo_index++] = i0 - size_x + 1;
            indices[ibo_index++] = -1;
        }
        debug_msg("ibo_index = %d", ibo_index);
        debug_msg("adj_index_count = %d", adj_index_count);
        adjacency_vao_ptr->ibo.init(GL_TRIANGLE_STRIP_ADJACENCY, indices, adj_index_count);
    }
    
    free(vertices);
    free(indices);
}

template void torus_t::generate_vao<vertex_pnt2_t>(typename maps<vertex_pnt2_t>::toral_func, int, int, adjacency_vao_t*);
template void torus_t::generate_vao<vertex_pft2_t>(typename maps<vertex_pft2_t>::toral_func, int, int, adjacency_vao_t*);
template void torus_t::generate_vao<vertex_pf_t  >(typename maps<vertex_pf_t  >::toral_func, int, int, adjacency_vao_t*);
template void torus_t::generate_vao<vertex_pn_t  >(typename maps<vertex_pn_t  >::toral_func, int, int, adjacency_vao_t*);

//=======================================================================================================================================================================================================================
// Assume that the vertex generating function is a costly to compute. The function below uses multiple threads to fill in the vertex and index buffer
// Toral topology allows this as torus obviously splits into a union of triangular strips and calculations can be done independently
//=======================================================================================================================================================================================================================
template<typename vertex_t, int threads> void torus_t::generate_vao_mt(typename maps<vertex_t>::toral_func func, int size_x, int size_y)
{
    //===================================================================================================================================================================================================================
    // prepare data to be read by all threads
    //===================================================================================================================================================================================================================
    compute_data<vertex_t> data;
    data.size_x = size_x;
    data.size_y = size_y;
    data.delta_x = 1.0f / float(size_x);
    data.delta_y = 1.0f / float(size_y);

    data.strips_per_thread = size_y / threads;
    data.vertices_per_thread = data.strips_per_thread * size_x;
    data.indices_per_thread = data.strips_per_thread * (2 * (size_x + 1) + 1);                  
    int V = threads * data.vertices_per_thread;
    GLuint index_count = threads * data.indices_per_thread;

    //===================================================================================================================================================================================================================
    // allocate space for attribute and index buffers
    //===================================================================================================================================================================================================================
    vertex_t* vertices = (vertex_t*) malloc(V * sizeof(vertex_t));
    GLuint* indices = (GLuint*) malloc(index_count * sizeof(GLuint));
    data.vertices = vertices;
    data.indices = indices;

    //===================================================================================================================================================================================================================
    // run threads...                                                                                                                                                                                                    
    //===================================================================================================================================================================================================================
    std::thread computation_thread[threads - 1];

    for (int thread_id = 0; thread_id < threads - 1; ++thread_id)
        computation_thread[thread_id] = std::thread(fill_vao_chunk<vertex_t>, func, data, thread_id);

    //===================================================================================================================================================================================================================
    // this thread will do the last task ... and will wait for others to finish                                                                                                                                          
    //===================================================================================================================================================================================================================
    fill_vao_chunk(func, data, threads - 1);           

    for (int thread_id = 0; thread_id < threads - 1; ++thread_id) 
        computation_thread[thread_id].join();

    //===================================================================================================================================================================================================================
    // create VAO                                                                                                                                                                                                        
    //===================================================================================================================================================================================================================
    vao.init(GL_TRIANGLE_STRIP, vertices, V, indices, index_count);

    free(vertices);
    free(indices);
}

template void torus_t::generate_vao_mt<vertex_pnt2_t>(typename maps<vertex_pnt2_t>::toral_func func, int, int);
template void torus_t::generate_vao_mt<vertex_pft2_t>(typename maps<vertex_pft2_t>::toral_func func, int, int);
template void torus_t::generate_vao_mt<vertex_pf_t  >(typename maps<vertex_pf_t  >::toral_func func, int, int);
template void torus_t::generate_vao_mt<vertex_pn_t  >(typename maps<vertex_pn_t  >::toral_func func, int, int);

//=======================================================================================================================================================================================================================
// Rendering functions                                                                                                                                                                                                   
//=======================================================================================================================================================================================================================
void torus_t::render()
    { vao.render(); };

void torus_t::render(GLsizei count, const GLvoid* offset)
    { vao.render(count, offset); };

void torus_t::instanced_render(GLsizei primcount)
    { vao.instanced_render(primcount); };

//=======================================================================================================================================================================================================================
// Auxiliary function that populates a chunk of vertex and index buffers based on its thread id                                                                                                                          
//=======================================================================================================================================================================================================================
template<typename vertex_t> void torus_t::fill_vao_chunk(typename maps<vertex_t>::toral_func func, const torus_t::compute_data<vertex_t>& data, int thread_id)
{
    int vbo_index = data.vertices_per_thread * thread_id;
    int ibo_index = data.indices_per_thread * thread_id;

    glm::vec2 uv;
    uv.y = data.delta_y * data.strips_per_thread * thread_id;


    for (int q = 0; q < data.strips_per_thread; ++q)
    {
        uv.x = 0.0f;
        GLuint next_row_index = vbo_index + data.size_x;
        if (next_row_index == data.size_x * data.size_y) next_row_index = 0;

        for (int p = 0; p < data.size_x; ++p)
        {
            //===========================================================================================================================================================================================================
            // add two indices to the strip, fill vertices with data                                                                                                                                                     
            //===========================================================================================================================================================================================================
            data.indices[ibo_index++] = vbo_index;
            data.indices[ibo_index++] = next_row_index;
            data.vertices[vbo_index++] = func(uv);
            uv.x += data.delta_x;
            ++next_row_index;
        }                                                              

        //===============================================================================================================================================================================================================
        // complete the strip with the last two indices and finish it with primitive restart index = -1                                                                                                                  
        //===============================================================================================================================================================================================================
        data.indices[ibo_index++] = vbo_index - data.size_x;
        data.indices[ibo_index++] = next_row_index - data.size_x;
        data.indices[ibo_index++] = -1;
        uv.y += data.delta_y;
    }
}

template void torus_t::fill_vao_chunk<vertex_pnt2_t>(typename maps<vertex_pnt2_t>::toral_func, const torus_t::compute_data<vertex_pnt2_t>&, int);
template void torus_t::fill_vao_chunk<vertex_pft2_t>(typename maps<vertex_pft2_t>::toral_func, const torus_t::compute_data<vertex_pft2_t>&, int);
template void torus_t::fill_vao_chunk<vertex_pf_t  >(typename maps<vertex_pf_t  >::toral_func, const torus_t::compute_data<vertex_pf_t  >&, int);
template void torus_t::fill_vao_chunk<vertex_pn_t  >(typename maps<vertex_pn_t  >::toral_func, const torus_t::compute_data<vertex_pn_t  >&, int);
