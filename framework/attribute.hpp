#ifndef __attribute_included_89275678943560875603475603456238476508237465894376
#define __attribute_included_89275678943560875603475603456238476508237465894376

#include <glm/gtx/string_cast.hpp>

#include "vao.hpp"
#include "edgeface.hpp"
#include "constants.hpp"

template<typename vertex_t, typename index_t = GLuint, GLenum mode = GL_TRIANGLES> struct attribute_data_t
{
    GLuint V;
    GLuint I;
    vertex_t* vertices;
    index_t* indices;

    attribute_data_t() : V(0), I(0), vertices(0), indices(0) {}

    attribute_data_t(GLuint V, GLuint I) : V(V), I(I)
        { allocate(V, I); }

    attribute_data_t(GLuint V, GLuint I, vertex_t* vertices, index_t* indices) : V(V), I(I), vertices(vertices), indices(indices) {}

    attribute_data_t(attribute_data_t&& other) : V(other.V), I(other.I), vertices(other.vertices), indices(other.indices)
    {
        other.vertices = 0;
        other.indices = 0;
    }

    attribute_data_t& operator = (attribute_data_t&& other)
    {
        V = other.V;
        I = other.I;
        std::swap(vertices, other.vertices);
        std::swap(indices, other.indices);
        return *this;
    }

    attribute_data_t(const attribute_data_t& other) = delete;
    attribute_data_t& operator = (const attribute_data_t&) = delete;

    void allocate(GLuint V, GLuint I)
    {
        vertices = (vertex_t*) malloc(V * sizeof(vertex_t));
        indices  = (index_t*)  malloc(I * sizeof(index_t));
    }

    vao_t generate_vao()
        { return vao_t(mode, vertices, V, indices, I); }

    void destroy()
    {
        V = 0;
        I = 0;
        free(vertices);
        free(indices);
    }

    ~attribute_data_t()
    {
        free(vertices);
        free(indices);
    }

};

template<typename vertex_t, typename index_t> glm::vec3* calculate_normals(attribute_data_t<vertex_t, index_t, GL_TRIANGLES>& data)
{
    glm::vec3* normals = (glm::vec3*) calloc(data.V, sizeof(glm::vec3));
    GLuint F = data.I / 3;

    for(GLuint i = 2; i < F; i += 3)
    {
        index_t a = data.indices[i - 2];
        index_t b = data.indices[i - 1];
        index_t c = data.indices[i - 0];

        glm::vec3& AB = data.vertices[b] - data.vertices[a];
        glm::vec3& BC = data.vertices[c] - data.vertices[b];
        glm::vec3 n = glm::cross(AB, BC);

        normals[a] += n;
        normals[b] += n;
        normals[c] += n;
    }

    for(GLuint v = 0; v < data.V; ++v)
        normals[v] = glm::normalize(normals[v]);

    return normals;
}

template<typename vertex_t, typename index_t> glm::vec3* calculate_normals(attribute_data_t<vertex_t, index_t, GL_TRIANGLE_STRIP>& data)
{
    glm::vec3* normals = (glm::vec3*) calloc(data.V, sizeof(glm::vec3));

    index_t i = 2;

    while(i < data.I)
    {
        index_t a = data.indices[i - 2];
        index_t b = data.indices[i - 1];

        glm::vec3 AB = data.vertices[b].position - data.vertices[a].position;
        bool right_handed = true;

        do
        {
            index_t c = data.indices[i];
            glm::vec3 BC = data.vertices[c].position - data.vertices[b].position;

            glm::vec3 n = glm::cross(AB, BC);

            if (right_handed)
            {
                normals[a] += n;
                normals[b] += n;
                normals[c] += n;
            }
            else
            {
                normals[a] -= n;
                normals[b] -= n;
                normals[c] -= n;
            }
            right_handed = !right_handed;

            AB = BC;
            a = b;
            b = c;
            ++i;
        }
        while (data.indices[i] != -1);

        i += 3;
    }

    return normals;
}

//=======================================================================================================================================================================================================================
// creating vertex adjacency list ...
//=======================================================================================================================================================================================================================
template<typename index_t, GLenum mode> struct unsorted_vertex_triples
{
    static glm::tvec3<index_t>* build(index_t* indices, GLuint I, GLuint& E);
};

//=======================================================================================================================================================================================================================
// ... for GL_TRIANGLES primitive and ...
//=======================================================================================================================================================================================================================
template<typename index_t> struct unsorted_vertex_triples<index_t, GL_TRIANGLES>
{
    static glm::tvec3<index_t>* build(index_t* indices, GLuint I, GLuint& E)
    {
        E = I;
        GLuint F = I / 3;
        glm::tvec3<index_t>* faces = (glm::tvec3<index_t>*) indices;
        glm::tvec3<index_t>* vertex_adjacency = (glm::tvec3<index_t>*) malloc(E * sizeof(glm::tvec3<index_t>));

        GLuint va = 0;
        for (GLuint f = 0; f < F; ++f)
        {
            glm::tvec3<index_t>& face = faces[f];
            vertex_adjacency[va++] = face;
            vertex_adjacency[va++] = glm::tvec3<index_t>(face.y, face.z, face.x);
            vertex_adjacency[va++] = glm::tvec3<index_t>(face.z, face.x, face.y);
        }
        return vertex_adjacency;
    }
};

//=======================================================================================================================================================================================================================
// ... for GL_TRIANGLE_STRIP primitive
//=======================================================================================================================================================================================================================
template<typename index_t> struct unsorted_vertex_triples<index_t, GL_TRIANGLE_STRIP>
{
    static glm::tvec3<index_t>* build(index_t* indices, GLuint I, GLuint& E)
    {
        GLuint strips = 0;
        for(GLuint i = 0; i < I; ++i)
            if (indices[i] == -1) ++strips;
        GLuint F = I - strips;
        E = F + F + F;
        glm::tvec3<index_t>* vertex_adjacency = malloc(E * sizeof(glm::tvec3<index_t>));

        GLuint va = 0;
        index_t i = 2;

        while(i < I)
        {
            index_t a = indices[i - 2];
            index_t b = indices[i - 1];
            bool right_handed = true;
            do
            {
                index_t c = indices[i];
                if (!right_handed)
                {
                    vertex_adjacency[va++] = glm::tvec3<index_t>(a, b, c);
                    vertex_adjacency[va++] = glm::tvec3<index_t>(b, c, a);
                    vertex_adjacency[va++] = glm::tvec3<index_t>(c, a, b);
                }
                else
                {
                    vertex_adjacency[va++] = glm::tvec3<index_t>(b, a, c);
                    vertex_adjacency[va++] = glm::tvec3<index_t>(a, c, b);
                    vertex_adjacency[va++] = glm::tvec3<index_t>(c, b, a);
                }
                right_handed = !right_handed;

                a = b;
                b = c;
                ++i;
            }
            while (indices[i] != -1);
            i += 3;
        }
        return vertex_adjacency;
    }
};

//=======================================================================================================================================================================================================================
// vertex occlusion calculation routine
//=======================================================================================================================================================================================================================
template<typename vertex_t, typename index_t, GLenum mode> float* calculate_vertex_occlusion(attribute_data_t<vertex_t, index_t, mode>& data)
{
    //===================================================================================================================================================================================================================
    // calculate vertex adjacency data
    //===================================================================================================================================================================================================================
    GLuint E;
    glm::tvec3<index_t>* vertex_adjacency = unsorted_vertex_triples<index_t, mode>::build(data.indices, data.I, E);

    //====================================================================================================================================================================================================================
    // quick sort edges lexicographically so that the first component increases
    //====================================================================================================================================================================================================================
    const unsigned int STACK_SIZE = 32;                                                         // variables to emulate stack of sorting requests

    struct
    {
        glm::tvec3<index_t>* l;                                                                 // left index of the sub-array that needs to be sorted
        glm::tvec3<index_t>* r;                                                                 // right index of the sub-array to sort
    } _stack[STACK_SIZE];

    int sp = 0;                                                                                 // stack pointer, stack grows up, not down
    _stack[sp].l = vertex_adjacency;
    _stack[sp].r = vertex_adjacency + E - 1;

    do
    {
        glm::tvec3<index_t>* l = _stack[sp].l;
        glm::tvec3<index_t>* r = _stack[sp].r;
        --sp;
        do
        {
            glm::tvec3<index_t>* i = l;
            glm::tvec3<index_t>* j = r;
            glm::tvec3<index_t>* m = i + (j - i) / 2;
            index_t x = m->x;
            do
            {
                while (i->x < x) i++;                                                           // lexicographic compare and proceed forward if less
                while (j->x > x) j--;                                                           // lexicographic compare and proceed backward if less

                if (i <= j)
                {
                    std::swap(*i, *j);
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
    // occlusion calculaton
    //====================================================================================================================================================================================================================
    float* occlusions = (float*) malloc(data.V * sizeof(float));

    GLuint e = 0;
    for(GLuint v = 0; v < data.V; ++v)
    {
        GLuint degree = 0;
        while ((vertex_adjacency[e + degree].x == v) && (e + degree < E)) degree++;

        //================================================================================================================================================================================================================
        // sort edges incident to a given vertex
        //================================================================================================================================================================================================================
        GLuint next_c = vertex_adjacency[e].z;
        for (GLuint d = 1; d < degree - 1; ++d)
        {
            GLuint q;
            for(q = d; vertex_adjacency[q].y != next_c; q++);
            next_c = vertex_adjacency[q].z;
            if (q != d)
                std::swap(vertex_adjacency[d], vertex_adjacency[q]);
        }

        //================================================================================================================================================================================================================
        // compute spherical polygon area
        //================================================================================================================================================================================================================
        glm::vec3 AB = glm::normalize(data.vertices[vertex_adjacency[e + degree - 1].z].position - data.vertices[vertex_adjacency[e + degree - 1].y].position);

        double angle_sum = 0.0;

        for(GLuint d = 0; d < degree; ++d)
        {
            glm::vec3 oB = glm::normalize(data.vertices[vertex_adjacency[e + d].y].position - data.vertices[v].position);
            glm::vec3 BC = glm::normalize(data.vertices[vertex_adjacency[e + d].z].position - data.vertices[vertex_adjacency[e + d].y].position);

            double angle = glm::acos(glm::dot(AB, BC));
            angle_sum += (glm::dot(glm::cross(AB, BC), oB) > 0.0f) ? (constants::two_pi - angle) : angle;

            AB = BC;
        }
        occlusions[v] = constants::inv_two_pi_d * (angle_sum - constants::pi_d * double(degree - 2));
        e += degree;
    }


    return occlusions;
}

//=======================================================================================================================================================================================================================
// calculates vertex occlusion in place
// vertex_t structure must have occlusion field
//=======================================================================================================================================================================================================================
template<typename vertex_t, typename index_t, GLenum mode> void calculate_vertex_occlusion_in_place(attribute_data_t<vertex_t, index_t, mode>& data)
{
    //===================================================================================================================================================================================================================
    // calculate vertex adjacency data
    //===================================================================================================================================================================================================================
    GLuint E;
    glm::tvec3<index_t>* vertex_adjacency = unsorted_vertex_triples<index_t, mode>::build(data.indices, data.I, E);

    //====================================================================================================================================================================================================================
    // quick sort edges lexicographically so that the first component increases
    //====================================================================================================================================================================================================================
    const unsigned int STACK_SIZE = 32;                                                         // variables to emulate stack of sorting requests

    struct
    {
        glm::tvec3<index_t>* l;                                                                 // left index of the sub-array that needs to be sorted
        glm::tvec3<index_t>* r;                                                                 // right index of the sub-array to sort
    } _stack[STACK_SIZE];

    int sp = 0;                                                                                 // stack pointer, stack grows up, not down
    _stack[sp].l = vertex_adjacency;
    _stack[sp].r = vertex_adjacency + E - 1;

    do
    {
        glm::tvec3<index_t>* l = _stack[sp].l;
        glm::tvec3<index_t>* r = _stack[sp].r;
        --sp;
        do
        {
            glm::tvec3<index_t>* i = l;
            glm::tvec3<index_t>* j = r;
            glm::tvec3<index_t>* m = i + (j - i) / 2;
            index_t x = m->x;
            do
            {
                while (i->x < x) i++;                                                           // lexicographic compare and proceed forward if less
                while (j->x > x) j--;                                                           // lexicographic compare and proceed backward if less

                if (i <= j)
                {
                    std::swap(*i, *j);
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
    // occlusion calculaton
    //====================================================================================================================================================================================================================
    GLuint e = 0;
    for(GLuint v = 0; v < data.V; ++v)
    {
        GLuint degree = 0;
        while ((vertex_adjacency[e + degree].x == v) && (e + degree < E)) degree++;

        //================================================================================================================================================================================================================
        // sort edges incident to a given vertex
        //================================================================================================================================================================================================================
        GLuint next_c = vertex_adjacency[e].z;
        for (GLuint d = 1; d < degree - 1; ++d)
        {
            GLuint q;
            for(q = d; vertex_adjacency[e + q].y != next_c; q++);
            next_c = vertex_adjacency[e + q].z;
            if (q != d)
                std::swap(vertex_adjacency[e + d], vertex_adjacency[e + q]);
        }

        //================================================================================================================================================================================================================
        // compute spherical polygon area
        //================================================================================================================================================================================================================
        glm::vec3 AB = glm::normalize(data.vertices[vertex_adjacency[e + degree - 1].z].position - data.vertices[vertex_adjacency[e + degree - 1].y].position);

        double external_angle_sum = 0.0;

        for(GLuint d = 0; d < degree; ++d)
        {
            glm::vec3 oB = glm::normalize(data.vertices[vertex_adjacency[e + d].y].position - data.vertices[v].position);
            glm::vec3 BC = glm::normalize(data.vertices[vertex_adjacency[e + d].z].position - data.vertices[vertex_adjacency[e + d].y].position);


            glm::vec3 noAB = glm::cross(AB, oB);
            glm::vec3 noBC = glm::cross(BC, oB);

            double angle = glm::acos(glm::dot(noAB, noBC));

            if (glm::dot(glm::cross(AB, BC), oB) < 0.0f)
                external_angle_sum -= angle;
            else
                external_angle_sum += angle;

            AB = BC;
        }
        data.vertices[v].occlusion = 1.0 - constants::inv_two_pi_d * external_angle_sum;
        e += degree;
    }
}
#endif // __attribute_included_89275678943560875603475603456238476508237465894376