//=======================================================================================================================================================================================================================
// Polyhedron structure methods implementation
//=======================================================================================================================================================================================================================

#include "log.hpp"
#include "polyhedron.hpp"
#include "constants.hpp"

#include <glm/gtx/string_cast.hpp>

void polyhedron::generate_pft2_vao(const glm::vec3* positions, const glm::vec3* normals, const glm::vec2* uvs, int vertex_count)
{
    vertex_pft2_t* vertices = (vertex_pft2_t*) malloc(vertex_count * sizeof(vertex_pft2_t));

    //===================================================================================================================================================================================================================
    // calculate tangent basis and fill the attribute buffer
    //===================================================================================================================================================================================================================
    for (int i = 0; i < vertex_count; i += 3)
    {
        glm::mat2x3 tangent_basis = glm::mat2x3(positions[i + 0] - positions[i + 1], positions[i + 0] - positions[i + 2]);
        glm::mat2 uvs_basis = glm::mat2(uvs[i + 0] - uvs[i + 1], uvs[i + 0] - uvs[i + 2]);
        glm::mat2x3 tangent_xy = tangent_basis * glm::inverse(uvs_basis);

        vertices[i + 0] = vertex_pft2_t(positions[i + 0], normals[i + 0], tangent_xy[0], tangent_xy[1], uvs[i + 0]);
        vertices[i + 1] = vertex_pft2_t(positions[i + 1], normals[i + 1], tangent_xy[0], tangent_xy[1], uvs[i + 1]);
        vertices[i + 2] = vertex_pft2_t(positions[i + 2], normals[i + 2], tangent_xy[0], tangent_xy[1], uvs[i + 2]);
    }

    //===================================================================================================================================================================================================================
    // Feed the data to OpenGL and set up buffer layout
    //===================================================================================================================================================================================================================
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    vbo.init(vertices, vertex_count);
    free(vertices);
}

//=======================================================================================================================================================================================================================
// Generates position + frame + texture coordinate buffer for one of the five regular plato solids
//=======================================================================================================================================================================================================================
void polyhedron::regular_pft2_vao(int V, int F, const glm::vec3* positions, const glm::vec3* normals, const int* faces, float scale, bool invert_normals)
{
    int E = V + F - 2;
    int Q = (2 * E) / F;
    GLuint vertex_count = 6 * E;

    //===================================================================================================================================================================================================================
    //                   V  F  E  Q
    // tetrahedron   ::  4  4  6  3
    // cube          ::  8  6 12  4
    // octahedron    ::  6  8 12  3
    // dodecahedron  :: 20 12 30  5
    // icosahedron   :: 12 20 30  3
    //===================================================================================================================================================================================================================

    debug_msg("V = %d. F = %d. Q = %d. vertex_count = %d", V, F, Q, vertex_count);

    vertex_pft2_t* vertices = (vertex_pft2_t*) malloc(vertex_count * sizeof(vertex_pft2_t));

    glm::vec2 polygon[5];
    const glm::vec2 square_center = glm::vec2(0.5f);

    if (Q != 4)
    {
        for (int i = 0; i < Q; ++i)
        {
            float alpha = (constants::two_pi * i) / Q;
            polygon[i] = glm::vec2(glm::cos(alpha), glm::sin(alpha));
        }
    }
    else
    {
        polygon[0] = glm::vec2( 1.0f,  1.0f);
        polygon[1] = glm::vec2(-1.0f,  1.0f);
        polygon[2] = glm::vec2(-1.0f, -1.0f);
        polygon[3] = glm::vec2( 1.0f, -1.0f);
    }

    int index = 0, buffer_index = 0;

    glm::mat2 uvs_basis = 0.25f * glm::mat2(polygon[0] - polygon[1], polygon[0] - polygon[2]);
    glm::mat2 uvs_basis_inv = glm::inverse(uvs_basis);

    for (int i = 0; i < F; ++i)
    {
        glm::vec3 face_center = positions[faces[index]];
        for (int j = 1; j < Q; ++j) face_center += positions[faces[index + j]];
        face_center *= (scale / Q);
        glm::mat2x3 tangent_basis = glm::mat2x3(positions[faces[index + 0]] - positions[faces[index + 1]], positions[faces[index + 0]] - positions[faces[index + 2]]);
        glm::mat2x3 tangent_xy = tangent_basis * uvs_basis_inv;
        glm::vec3 normal = invert_normals ? -normals[i] : normals[i];

        for (int j = 0; j < Q; ++j)
        {
            if (invert_normals)
            {
                vertices[buffer_index++] = vertex_pft2_t(scale * positions[faces[index + j]], normal, tangent_xy[0], tangent_xy[1], square_center + 0.5f * polygon[j]);
                vertices[buffer_index++] = vertex_pft2_t(face_center, normal, tangent_xy[0], tangent_xy[1], square_center);
            }
            else
            {
                vertices[buffer_index++] = vertex_pft2_t(face_center, normal, tangent_xy[0], tangent_xy[1], square_center);
                vertices[buffer_index++] = vertex_pft2_t(scale * positions[faces[index + j]], normal, tangent_xy[0], tangent_xy[1], square_center + 0.5f * polygon[j]);
            }
            int k = (j == Q - 1) ? 0 : j + 1;
            vertices[buffer_index++] = vertex_pft2_t(scale * positions[faces[index + k]], normal, tangent_xy[0], tangent_xy[1], square_center + 0.5f * polygon[k]);
        }
        index += Q;
    }

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    vbo.init(vertices, vertex_count);
    free(vertices);
}

//=======================================================================================================================================================================================================================
// Generates position + frame + texture coordinate buffer for one of the five regular plato solids
//=======================================================================================================================================================================================================================
void polyhedron::regular_pnt2_vao(int V, int F, const glm::vec3* positions, const glm::vec3* normals, const int* faces, float scale, bool invert_normals)
{
    int E = V + F - 2;
    int Q = (2 * E) / F;
    GLuint vertex_count = 6 * E;

    debug_msg("V = %d. F = %d. Q = %d. vertex_count = %d", V, F, Q, vertex_count);

    vertex_pnt2_t* vertices = (vertex_pnt2_t*) malloc(vertex_count * sizeof(vertex_pnt2_t));

    glm::vec2 polygon[5];
    const glm::vec2 square_center = glm::vec2(0.5f);

    if (Q != 4)
    {
        for (int i = 0; i < Q; ++i)
        {
            float alpha = (constants::two_pi * i) / Q;
            polygon[i] = glm::vec2(glm::cos(alpha), glm::sin(alpha));
        }
    }
    else
    {
        polygon[0] = glm::vec2( 1.0f,  1.0f);
        polygon[1] = glm::vec2(-1.0f,  1.0f);
        polygon[2] = glm::vec2(-1.0f, -1.0f);
        polygon[3] = glm::vec2( 1.0f, -1.0f);
    }

    int index = 0, buffer_index = 0;

    for (int i = 0; i < F; ++i)
    {
        glm::vec3 face_center = positions[faces[index]];
        for (int j = 1; j < Q; ++j) face_center += positions[faces[index + j]];
        face_center *= (scale / Q);

        glm::vec3 normal = invert_normals ? -normals[i] : normals[i];
        for (int j = 0; j < Q; ++j)
        {
            if (invert_normals)
            {
                vertices[buffer_index++] = vertex_pnt2_t(scale * positions[faces[index + j]], normal, square_center + 0.5f * polygon[j]);
                vertices[buffer_index++] = vertex_pnt2_t(face_center, normal, square_center);
            }
            else
            {
                vertices[buffer_index++] = vertex_pnt2_t(face_center, normal, square_center);
                vertices[buffer_index++] = vertex_pnt2_t(scale * positions[faces[index + j]], normal, square_center + 0.5f * polygon[j]);
            }
            int k = (j == Q - 1) ? 0 : j + 1;
            vertices[buffer_index++] = vertex_pnt2_t(scale * positions[faces[index + k]], normal, square_center + 0.5f * polygon[k]);
        };
        index += Q;
    }

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    vbo.init(vertices, vertex_count);
    free(vertices);
}

//=======================================================================================================================================================================================================================
// Generates position + normal buffer for one of the five regular plato solids
//=======================================================================================================================================================================================================================
void polyhedron::regular_pn_vao(int V, int F, const glm::vec3* positions, const glm::vec3* normals, const int* faces, float scale, bool invert_normals)
{
    int E = V + F - 2;
    int Q = (2 * E) / F;
    GLuint vertex_count = 6 * V - 12;
    vertex_pn_t* vertices = (vertex_pn_t*) malloc(vertex_count * sizeof(vertex_pn_t));
    // debug_msg("V = %d. F = %d. E = %d. Q = %d. vertex_count = %d", V, F, E, Q, vertex_count);

    int index = 0, buffer_index = 0;

    for (int i = 0; i < F; ++i)
    {
        glm::vec3 normal = invert_normals ? -normals[i] : normals[i];
        for (int j = 0; j < Q - 2; ++j)
        {
            vertices[buffer_index++] = vertex_pn_t(scale * positions[faces[index]], normal);
            if (invert_normals)
            {
                vertices[buffer_index++] = vertex_pn_t(scale * positions[faces[index + j + 2]], normal);
                vertices[buffer_index++] = vertex_pn_t(scale * positions[faces[index + j + 1]], normal);
            }
            else
            {
                vertices[buffer_index++] = vertex_pn_t(scale * positions[faces[index + j + 1]], normal);
                vertices[buffer_index++] = vertex_pn_t(scale * positions[faces[index + j + 2]], normal);
            }
        };
        index += Q;
    }

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    vbo.init(vertices, vertex_count);
    free(vertices);
}


void polyhedron::render()
{
    glBindVertexArray(vao_id);
    vbo.render(GL_TRIANGLES);
}

void polyhedron::instanced_render(GLsizei primcount)
{
    glBindVertexArray(vao_id);
    vbo.instanced_render(GL_TRIANGLES, primcount);
}

polyhedron::~polyhedron()
    { glDeleteVertexArrays(1, &vao_id); }