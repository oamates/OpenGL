#ifndef _closed_curve_included_974639572394765073560364893745893743852457547235
#define _closed_curve_included_974639572394765073560364893745893743852457547235

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "vertex.hpp"
#include "vao.hpp"
#include "constants.hpp"

//=======================================================================================================================================================================================================================
// structure that initializes buffer of 4x4 matrices of Frenet frames along a space curve
// * as an input takes a double precision function f on the unit circle in xy-plane f(t) = f(cos(t), sin(t))
// * computes tangent, principal normal and binormal vectors and packs it together with a position vector into a 4x4 matrix
// * rendering call uses GL_LINE_LOOP primitive, it is up to geometry shader to produce something beautiful from this data
// * it has access to the two neighbouring Frenet frames simultaneously so can generate cylindrical shell to produce a toral-like surface
//=======================================================================================================================================================================================================================

struct ccurve_t
{
    GLuint vao_id;
    vbo_t vbo;

    ccurve_t() {};

    ccurve_t(typename maps<glm::dvec3>::curve_dfunc func, int size)
        { generate_pf_vao(func, size); }

    template<typename vertex_t> void generate_vao(typename maps<vertex_t>::curve_func func, int size)
    {
        vertex_t* vertices = (vertex_t*) malloc(size * sizeof(vertex_t));

        double dt = constants::two_pi_d / size;
        double cs = cos(dt);
        double sn = sin(dt);

        glm::dvec2 arg = glm::dvec2(1.0, 0.0);
        vertices[0] = func(glm::vec2(arg));

        for (int i = 1; i < size; ++i)
        {
            double arg_x = arg.x;
            arg.x = arg.x * cs - arg.y * sn;
            arg.y = arg_x * sn + arg.y * cs;
            vertices[i] = func(glm::vec2(arg));
        }

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        vbo.init(vertices, size);
    }

    void generate_pf_vao(typename maps<glm::dvec3>::curve_dfunc func, int size)
    {
        vertex_pf_t* vertices = (vertex_pf_t*) malloc(size * sizeof(vertex_pf_t));

        double dt = constants::two_pi_d / size;

        glm::dvec3 f_m = func(glm::dvec2(glm::cos(dt), -glm::sin(dt)));
        glm::dvec3 f_0 = func(glm::dvec2(1.0, 0.0));
        glm::dvec3 df0 = f_0 - f_m;

        for (int i = 1; i <= size; ++i)
        {
            glm::dvec3 f_p = func(glm::dvec2(glm::cos(i * dt), glm::sin(i * dt)));
            glm::dvec3 df1 = f_p - f_0;

            glm::dvec3 t = glm::normalize(f_p - f_m);
            glm::dvec3 b = glm::normalize(glm::cross(df0, df1));
            glm::dvec3 n = glm::cross(b, t);

            vertices[i - 1] = vertex_pf_t(glm::vec3(f_0), glm::vec3(t), glm::vec3(n), glm::vec3(b));

            f_m = f_0;
            f_0 = f_p;
            df0 = df1;
        }

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        vbo.init(vertices, size);
    }

    void render()
    {
        glBindVertexArray(vao_id);
        vbo.render(GL_LINE_LOOP);
    }

    void instanced_render(GLsizei primcount)
    {
        glBindVertexArray(vao_id);
        vbo.instanced_render(GL_LINE_LOOP, primcount);
    }

    ~ccurve_t ()
        { glDeleteVertexArrays(1, &vao_id); }

};

#endif //_closed_curve_included_974639572394765073560364893745893743852457547235
