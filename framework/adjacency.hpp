#ifndef _adjacency_included_130589460189463784356108734568107346589172568071346
#define _adjacency_included_130589460189463784356108734568107346589172568071346

#include "vao.hpp"
#include "log.hpp"

template<typename index_t> struct edge_t
{
    index_t a, b;                                                                                       // structure represents directed edge [ab]
    uint32_t face;                                                                                      // index of the face edge [ab] belongs to

    edge_t(index_t a, index_t b, uint32_t face) : a(a), b(b), face(face) {}
};

template<typename index_t> ibo_t build_adjacency_ibo(const glm::tvec3<index_t>* faces, uint32_t F);

template<typename index_t> vao_t build_adjacency_vao(const glm::vec3* vertices, const glm::tvec3<index_t>* faces, uint32_t V, uint32_t F)
{
    vao_t vao;
    glGenVertexArrays(1, &vao.id);
    glBindVertexArray(vao.id);
    vao.vbo.init<vertex_p_t>((vertex_p_t*) vertices, V);
    vao.ibo = build_adjacency_ibo<index_t>(faces, F);
    return vao;
}

template<typename index_t> ibo_t build_adjacency_ibo(const glm::tvec3<index_t>* faces, uint32_t F)
{
    uint32_t E = F + F + F;
    edge_t<index_t>* edges = (edge_t<index_t>*) malloc(E * sizeof(edge_t<index_t>));

    //========================================================================================================================================================================================================================
    // create edge structure array from the input data
    //========================================================================================================================================================================================================================
    for (uint32_t index = 0, f = 0; f < F; ++f)
    {
        index_t a = faces[f].x,
                b = faces[f].y,
                c = faces[f].z;
        edges[index++] = edge_t<index_t>(a, b, f);
        edges[index++] = edge_t<index_t>(b, c, f);
        edges[index++] = edge_t<index_t>(c, a, f);
    }

    //====================================================================================================================================================================================================================
    // quick sort edges lexicographically, leaving unfilled (-1) members untouched
    //====================================================================================================================================================================================================================
    const unsigned int STACK_SIZE = 32;                                                         // variables to emulate stack of sorting requests

    struct
    {
        edge_t<index_t>* l;                                                                     // left index of the sub-array that needs to be sorted
        edge_t<index_t>* r;                                                                     // right index of the sub-array to sort
    } _stack[STACK_SIZE];

    int sp = 0;                                                                                 // stack pointer, stack grows up not down
    _stack[sp].l = edges;
    _stack[sp].r = edges + E - 1;

    do
    {
        edge_t<index_t>* l = _stack[sp].l;
        edge_t<index_t>* r = _stack[sp].r;
        --sp;
        do
        {
            edge_t<index_t>* i = l;
            edge_t<index_t>* j = r;
            edge_t<index_t>* m = i + (j - i) / 2;
            index_t a = m->a;
            index_t b = m->b;
            do
            {
                while ((i->b < b) || ((i->b == b) && (i->a < a))) i++;                          // lexicographic compare and proceed forward if less
                while ((j->b > b) || ((j->b == b) && (j->a > a))) j--;                          // lexicographic compare and proceed backward if less

                if (i <= j)
                {
                    std::swap(i->a, j->a);
                    std::swap(i->b, j->b);
                    std::swap(i->face, j->face);
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

    //====================================================================================================================================================================================================================
    // fill adjacent edge indices, -1 will indicate boundary edges
    //====================================================================================================================================================================================================================
    ibo_t ibo;
    ibo.init(GL_TRIANGLES_ADJACENCY, (const index_t*)0, 6 * F);
    index_t* buffer = (index_t*) ibo_t::map();

    for (uint32_t index = 0, f = 0; f < F; ++f)
    {
        index_t a = faces[f].x, b = faces[f].y, c = faces[f].z;
        uint32_t l, r;

        //================================================================================================================================================================================================================
        // binary search edge, opposite to [ab] --> [ba]
        //================================================================================================================================================================================================================
        l = 0;
        r = E - 1;
        while (l <= r)
        {
            int32_t m = (r + l) / 2;

            if (edges[m].b < a) { l = m + 1; continue; }
            if (edges[m].b > a) { r = m - 1; continue; }
            if (edges[m].a < b) { l = m + 1; continue; }
            if (edges[m].a > b) { r = m - 1; continue; }

            //============================================================================================================================================================================================================
            // edge found, m is its index
            //============================================================================================================================================================================================================
            glm::tvec3<index_t> face = faces[edges[m].face];
            buffer[index++] = a;
            buffer[index++] = face.x + face.y + face.z - a - b;
            break;
        }

        //================================================================================================================================================================================================================
        // binary search edge, opposite to [bc] --> [cb]
        //================================================================================================================================================================================================================

        l = 0;
        r = E - 1;
        while (l <= r)
        {
            int32_t m = (r + l) / 2;

            if (edges[m].b < b) { l = m + 1; continue; }
            if (edges[m].b > b) { r = m - 1; continue; }
            if (edges[m].a < c) { l = m + 1; continue; }
            if (edges[m].a > c) { r = m - 1; continue; }

            //============================================================================================================================================================================================================
            // edge found, m is its index
            //============================================================================================================================================================================================================
            glm::tvec3<index_t> face = faces[edges[m].face];
            buffer[index++] = b;
            buffer[index++] = face.x + face.y + face.z - b - c;
            break;
        }

        //================================================================================================================================================================================================================
        // binary search edge, opposite to [ca] --> [ac]
        //================================================================================================================================================================================================================
        l = 0;
        r = E - 1;
        while (l <= r)
        {
            int32_t m = (r + l) / 2;

            if (edges[m].b < c) { l = m + 1; continue; }
            if (edges[m].b > c) { r = m - 1; continue; }
            if (edges[m].a < a) { l = m + 1; continue; }
            if (edges[m].a > a) { r = m - 1; continue; }

            //============================================================================================================================================================================================================
            // edge found, m is its index
            //============================================================================================================================================================================================================
            glm::tvec3<index_t> face = faces[edges[m].face];
            buffer[index++] = c;
            buffer[index++] = face.x + face.y + face.z - c - a;
            break;
        }
    }


    free(edges);
    ibo_t::unmap();
    return ibo;
}

#endif // _adjacency_included_130589460189463784356108734568107346589172568071346