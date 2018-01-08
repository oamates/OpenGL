//========================================================================================================================================================================================================================
// DEMO 087 : Halo around objects
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "gl_aux.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "vao.hpp"
#include "vertex.hpp"


struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
          camera(64.0f, 0.5f, glm::lookAt(glm::vec3(8.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

struct spiral_sphere_t
{
    const double _radius, _thickness;
    const int _bands, _divisions, _segments;
    const int _vertex_count;

    GLuint vao_id;
    GLuint buffers[3];

    spiral_sphere_t()
        : _radius(1.0), _thickness(0.1), _bands(4), _divisions(8), _segments(48), vao_id(0),
          _vertex_count((_bands * 2) * (_divisions + 1) * (_segments + 1) + (_bands * 2) * (_segments + 1))
    { }

    spiral_sphere_t(double radius, double thickness, int bands, int divisions, int segments)
        : _radius(radius), _thickness(thickness), _bands(bands), _divisions(divisions), _segments(segments), vao_id(0),
          _vertex_count((_bands * 2) * (_divisions + 1) * (_segments + 1) + (_bands * 2) * (_segments + 1))
    { }

    ~spiral_sphere_t()
    {
        if (vao_id)
        {
            glDeleteBuffers(3, buffers);
            glDeleteVertexArrays(1, &vao_id);
        }
    }

    template<typename real_t> void _make_positions(std::vector<glm::tvec3<real_t>>& dest, int& k, double sign, double radius) const
    {
        double b_leap = constants::pi_d / double(_bands);
        double b_step = b_leap / double(_divisions);
        double s_step = constants::pi_d / double(_segments);
        double m = sign * radius;

        for (int b = 0; b != _bands; ++b)
        {
            for (int d = 0; d != (_divisions + 1); ++d)
            {
                double b_offs = 0.0;
                for (int s = 0; s != (_segments + 1); ++s)
                {
                    double b_angle = 2 * b * b_leap + d * b_step + b_offs;
                    double cb = glm::cos(b_angle);
                    double sb = glm::sin(b_angle);

                    double s_angle = s * s_step;
                    double cs = glm::cos(s_angle);
                    double ss = glm::sin(s_angle);

                    dest[k++] = glm::tvec3<real_t> (m * ss * cb, m * cs, -m * ss * sb);
                    b_offs += ss * s_step;
                }
            }
        }
    }

    template<typename real_t> void _make_tangents(std::vector<glm::tvec3<real_t>>& dest, int& k, double sign) const
    {
        double b_leap = constants::pi_d / double(_bands);
        double b_step = b_leap / double(_divisions);
        double s_step = constants::pi_d / double(_segments);
        double m = sign;

        for (int b = 0; b != _bands; ++b)
        {
            for (int d = 0; d != (_divisions + 1); ++d)
            {
                double b_offs = 0.0;
                for (int s = 0; s != (_segments + 1); ++s)
                {
                    double b_angle = 2 * b * b_leap + d * b_step + b_offs;
                    double cb = glm::cos(b_angle);
                    double sb = glm::sin(b_angle);

                    double s_angle = s * s_step;
                    double ss = glm::sin(s_angle);

                    dest[k++] = glm::tvec3<real_t> (-m * sb, 0.0, -m * cb);
                    b_offs += ss * s_step;
                }
            }
        }
    }

    template<typename real_t> void _make_bitangents(std::vector<glm::tvec3<real_t>>& dest, int& k, double sign) const
    {
        double b_leap = constants::pi_d / double(_bands);
        double b_step = b_leap / double(_divisions);
        double s_step = constants::pi_d / double(_segments);
        double m = sign;

        for (int b = 0; b != _bands; ++b)
        {
            for (int d = 0; d != (_divisions + 1); ++d)
            {
                double b_offs = 0.0;
                for (int s = 0; s != (_segments + 1); ++s)
                {
                    double b_angle = 2 * b * b_leap + d * b_step + b_offs;
                    double cb = glm::cos(b_angle);
                    double sb = glm::sin(b_angle);

                    double s_angle = s * s_step;
                    double cs = glm::cos(s_angle);
                    double ss = glm::sin(s_angle);

                    glm::dvec3 t = glm::dvec3(-m * sb, 0.0, -m * cb);
                    glm::dvec3 n = glm::dvec3(m * ss * cb, m * cs, -m * ss * sb);
                    dest[k++] = glm::tvec3<real_t> (glm::cross(n, t));
                    b_offs += ss * s_step;
                }
            }
        }
    }

    template<typename real_t> void _make_uv_coords(std::vector<glm::tvec2<real_t>>& dest, int& k) const
    {
        double b_leap = 0.5 / double(_bands);
        double b_step = b_leap / double(_divisions);
        double s_step = 1.0 / double(_segments);
        double u = 0.0;

        for (int b = 0; b != _bands; ++b)
        {
            for (int d = 0; d != (_divisions + 1); ++d)
            {
                double v = 1.0;
                for (int s = 0; s != (_segments + 1); ++s)
                {
                    dest[k++] = glm::tvec2<real_t> (u, v);
                    v -= s_step;
                }
                u += b_step;
            }
            u += b_leap;
        }
    }

    template<typename real_t> void _make_side_verts(std::vector<glm::tvec3<real_t>>& dest, int& k) const
    {
        double b_leap = constants::pi_d / double(_bands);
        double b_slip = b_leap * _thickness * 0.5;
        double s_step = constants::pi_d / double(_segments);
        double m = _radius + _thickness * 0.5;
        double g = -1.0;

        for (int b = 0; b != _bands * 2; ++b)
        {
            double b_offs = 0.0;
            for (int s = 0; s != (_segments + 1); ++s)
            {
                double b_angle = b * b_leap + b_offs + g * b_slip;
                double cb = glm::cos(b_angle);
                double sb = glm::sin(b_angle);

                double s_angle = s * s_step;
                double cs = glm::cos(s_angle);
                double ss = glm::sin(s_angle);

                dest[k++] = glm::tvec3<real_t> (m * ss * cb, m * cs, -m * ss * sb);
                b_offs += ss * s_step;
            }
            g = -g;
        }
    }

    template<typename real_t> void _make_side_norms(std::vector<glm::tvec3<real_t>>& dest, int& k) const
    {
        double b_leap = constants::pi_d / double(_bands);
        double s_step = constants::pi_d / double(_segments);
        double m = 1.0;

        for (int b = 0; b != _bands * 2; ++b)
        {
            double b_offs = 0.0;
            for (int s = 0; s != (_segments + 1); ++s)
            {
                double b_angle = b * b_leap + b_offs;
                double cb = glm::cos(b_angle);
                double sb = glm::sin(b_angle);

                double s_angle = s * s_step;
                double ss = glm::sin(s_angle);

                dest[k++] = glm::tvec3<real_t> (-m * sb, 0.0, m * cb);
                b_offs += ss * s_step;
            }
            m = -m;
        }
    }

    template<typename real_t> void _make_side_tgts(std::vector<glm::tvec3<real_t>>& dest, int& k) const
    {
        double b_leap = constants::pi_d / double(_bands);
        double s_step = constants::pi_d / double(_segments);
        double m = -1.0;

        for(int b = 0; b != _bands * 2; ++b)
        {
            double b_offs = 0.0;
            for(int s = 0; s != (_segments + 1); ++s)
            {
                double b_angle = b * b_leap + b_offs;
                double cb = std::cos(b_angle);
                double sb = std::sin(b_angle);

                double s_angle = s * s_step;
                double cs = std::cos(s_angle);
                double ss = std::sin(s_angle);

                dest[k++] = glm::tvec3<real_t> (-m * ss * cb, m * cs, -m * ss * sb);
                b_offs += ss * s_step;
            }
            m = -m;
        }
    }

    template<typename real_t> void _make_side_btgs(std::vector<glm::tvec3<real_t>>& dest, int& k) const
    {
        double b_leap = constants::pi_d / double(_bands);
        double s_step = constants::pi_d / double(_segments);

        double m = 1.0;
        for (int b = 0; b != _bands * 2; ++b)
        {
            double b_offs = 0.0;
            for (int s = 0; s != (_segments + 1); ++s)
            {
                double b_angle = b * b_leap + b_offs;
                double cb = glm::cos(b_angle);
                double sb = glm::sin(b_angle);

                double s_angle = s * s_step;
                double cs = glm::cos(s_angle);
                double ss = glm::sin(s_angle);

                glm::dvec3 t = glm::dvec3(-m * ss * cb, m * cs, -m * ss * sb);
                glm::dvec3 n = glm::dvec3(m * sb, 0.0, -m * cb);
                dest[k++] = glm::tvec3<real_t> (glm::cross(n, t));
                b_offs += ss * s_step;
            }
            m = -m;
        }
    }

    template<typename real_t> void _make_side_uvs(std::vector<glm::tvec2<real_t>>& dest, int& k) const
    {
        double b_leap = 0.5 / double(_bands);
        double b_slip = b_leap * _thickness * 0.5;
        double s_step = 1.0 / double(_segments);
        double g = -1.0;

        for (int b = 0; b != _bands * 2; ++b)
        {
            double b_offs = 0.0;
            double v = 1.0;
            for (int s = 0; s != (_segments + 1); ++s)
            {
                dest[k++] = glm::tvec2<real_t> (b * b_leap + b_offs + g * b_slip, v);
                v -= s_step;
            }
            g = -g;
        }
    }

    template<typename real_t> std::vector<glm::tvec3<real_t>> _positions() const
    {
        std::vector<glm::tvec3<real_t>> dest(_vertex_count);
        int k = 0;
        _make_positions(dest, k, 1.0, _radius);
        _make_positions(dest, k, 1.0, _radius + _thickness);
        _make_side_verts(dest, k);
        assert(k == dest.size());
        return dest;
    }

    template<typename real_t> std::vector<glm::tvec3<real_t>> _normals() const
    {
        std::vector<glm::tvec3<real_t>> dest(_vertex_count);
        int k = 0;
        _make_positions(dest, k, -1.0, 1.0);
        _make_positions(dest, k,  1.0, 1.0);
        _make_side_norms(dest, k);
        assert(k == dest.size());
        return dest;
    }

    template<typename real_t> std::vector<glm::tvec3<real_t>> _tangents() const
    {
        std::vector<glm::tvec3<real_t>> dest(_vertex_count);
        int k = 0;
        _make_tangents(dest, k, -1.0);
        _make_tangents(dest, k,  1.0);
        _make_side_tgts(dest, k);
        assert(k == dest.size());
        return dest;
    }

    template<typename real_t> std::vector<glm::tvec3<real_t>> _bitangents() const
    {
        std::vector<glm::tvec3<real_t>> dest(_vertex_count);
        int k = 0;
        _make_bitangents(dest, k, -1.0);
        _make_bitangents(dest, k,  1.0);
        _make_side_btgs(dest, k);
        assert(k == dest.size());
        return dest;
    }

    template<typename real_t> std::vector<glm::tvec2<real_t>> _tex_coords() const
    {
        std::vector<glm::tvec2<real_t>> dest(_vertex_count);
        int k = 0;
        _make_uv_coords(dest, k);
        _make_uv_coords(dest, k);
        _make_side_uvs(dest, k);
        assert(k == dest.size());
        return dest;
    }

    std::vector<GLuint> _indices() const
    {
        const unsigned int m = (_bands * 2) * (_divisions * 2) * (_segments + 1) + (_bands * 8) * (_segments + 1);

        std::vector<GLuint> indices(m);
        unsigned int k = 0;
        unsigned int eoffs, offs = 0;
        const unsigned int edge = _segments + 1;
        const unsigned int band = edge * (_divisions + 1);
        const unsigned int surface = _bands * band;

        for (unsigned int n = 0; n != 2; ++n)
        {
            unsigned int edge1 = n ? edge : 0;
            unsigned int edge2 = n ? 0 : edge;
            for (unsigned int b = 0; b != _bands; ++b)
            {
                for (unsigned int d = 0; d != _divisions; ++d)
                {
                    for (unsigned int s = 0; s != edge; ++s)
                    {
                        indices[k++] = offs + s + edge1;
                        indices[k++] = offs + s + edge2;
                    }
                    offs += edge;
                }
                offs += edge;
            }
        }

        offs = 0;
        eoffs = 2 * surface;

        for (unsigned int b = 0; b != _bands; ++b)
        {
            for (unsigned int s = 0; s != edge; ++s)
            {
                indices[k++] = eoffs + s;
                indices[k++] = offs + s;
            }
            offs += band;
            eoffs += edge * 2;
        }

        offs = _divisions * edge;
        eoffs = 2 * surface + edge;

        for (unsigned int b = 0; b != _bands; ++b)
        {
            for (unsigned int s = 0; s != edge; ++s)
            {
                indices[k++] = offs + s;
                indices[k++] = eoffs + s;
            }
            offs += band;
            eoffs += edge * 2;
        }

        offs = surface;
        eoffs = 2 * surface;

        for (unsigned int b = 0; b != _bands; ++b)
        {
            for (unsigned int s = 0; s != edge; ++s)
            {
                indices[k++] = offs + s;
                indices[k++] = eoffs + s;
            }
            offs += band;
            eoffs += edge * 2;
        }

        offs = surface + _divisions * edge;
        eoffs = 2 * surface + edge;

        for (unsigned int b = 0; b != _bands; ++b)
        {
            for (unsigned int s = 0; s != edge; ++s)
            {
                indices[k++] = eoffs + s;
                indices[k++] = offs + s;
            }
            offs += band;
            eoffs += edge * 2;
        }

        assert(k == indices.size());
        return indices;
    }


    void generate_pn_vao()
    {
        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);

        GLuint buffers[3];
        glGenBuffers(3, buffers);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        std::vector<glm::vec3> positions = _positions<float>();
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
        std::vector<glm::vec3> normals = _normals<float>();
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
        std::vector<GLuint> indices = _indices();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    }

    void render() const
    {
        glBindVertexArray(vao_id);

        const GLuint edge = _segments + 1;
        size_t offset = 0;

        for (int n = 0; n != 2; ++n)
        {
            for (int b = 0; b != _bands; ++b)
            {
                for (int d = 0; d != _divisions; ++d)
                {
                    glDrawElements(GL_TRIANGLE_STRIP, edge * 2, GL_UNSIGNED_INT, (const GLvoid *) ((char *) 0 + (offset)));
                    offset += edge * 2;
                }
            }
        }

        for(int n = 0; n != 4; ++n)
        {
            for(int b = 0; b != _bands; ++b)
            {
                glDrawElements(GL_TRIANGLE_STRIP, edge * 2, GL_UNSIGNED_INT, (const GLvoid *) ((char*) 0 + (offset)));
                offset += edge * 2;
            }
        }
    }
};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    const int res_x = 1920;
    const int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Halo", 4, 3, 3, res_x, res_y);

    //===================================================================================================================================================================================================================
    // shader compilation
    //===================================================================================================================================================================================================================
    glm::vec3 light_ws(2.0f, 2.5f, 9.0f);
    glm::mat4 projection_matrix = window.camera.projection_matrix;

    glsl_program_t shape_program(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/shape.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/shape.fs"));
    shape_program.enable();
    uniform_t uni_sp_view_matrix  = shape_program["view_matrix"];
    uniform_t uni_sp_model_matrix = shape_program["model_matrix"];
    shape_program["projection_matrix"] = projection_matrix;
    shape_program["light_ws"] = light_ws;

    glsl_program_t plane_program(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/plane.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/plane.fs"));
    plane_program.enable();
    uniform_t uni_pp_view_matrix = plane_program["view_matrix"];
    plane_program["projection_matrix"] = projection_matrix;
    plane_program["light_ws"] = light_ws;

    glsl_program_t halo_program(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/halo.vs"),
                                glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/halo.gs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/halo.fs"));
    halo_program.enable();
    uniform_t uni_hp_view_matrix = halo_program["view_matrix"];
    uniform_t uni_hp_model_matrix = halo_program["model_matrix"];
    halo_program["projection_matrix"] = projection_matrix;

    //===================================================================================================================================================================================================================
    // geometric objects initialization
    //===================================================================================================================================================================================================================
    spiral_sphere_t spiral_sphere;
    spiral_sphere.generate_pn_vao();

    vertex_pn_t plane_data[] =
    {
        vertex_pn_t(glm::vec3(-9.0f, 0.0f,  9.0f), glm::vec3(-0.1f, 1.0f,  0.1f)),
        vertex_pn_t(glm::vec3(-9.0f, 0.0f, -9.0f), glm::vec3(-0.1f, 1.0f, -0.1f)),
        vertex_pn_t(glm::vec3( 9.0f, 0.0f,  9.0f), glm::vec3( 0.1f, 1.0f,  0.1f)),
        vertex_pn_t(glm::vec3( 9.0f, 0.0f, -9.0f), glm::vec3( 0.1f, 1.0f, -0.1f))
    };

    GLuint plane_vao_id;
    glGenVertexArrays(1, &plane_vao_id);
    glBindVertexArray(plane_vao_id);
    vbo_t plane_vbo(plane_data, sizeof(plane_data) / sizeof(vertex_pn_t));

    //===================================================================================================================================================================================================================
    // global OpeGL state
    //===================================================================================================================================================================================================================
    glClearColor(0.04f, 0.01f, 0.09f, 0.0f);
    glClearDepth(1.0f);
    glClearStencil(0);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        //===============================================================================================================================================================================================================
        // clear back buffer, process events and update timer
        //===============================================================================================================================================================================================================
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        glm::mat4 view_matrix = window.camera.view_matrix;
        glm::mat4 model_matrix = glm::translate(glm::vec3(0.0f, 2.5f, 0.0f)) *
                                 glm::rotate(0.14f * constants::two_pi * time, glm::vec3(1.0f, 1.0f, 1.0f));

        //===============================================================================================================================================================================================================
        // render base plane
        //===============================================================================================================================================================================================================
        plane_program.enable();
        uni_pp_view_matrix = view_matrix;
        glBindVertexArray(plane_vao_id);
        plane_vbo.render(GL_TRIANGLE_STRIP);

        //===============================================================================================================================================================================================================
        // render object
        //===============================================================================================================================================================================================================
        shape_program.enable();
        uni_sp_view_matrix = view_matrix;
        uni_sp_model_matrix = model_matrix;
        spiral_sphere.render();

        //===============================================================================================================================================================================================================
        // render halo
        //===============================================================================================================================================================================================================
        halo_program.enable();
        uni_hp_view_matrix = view_matrix;
        uni_hp_model_matrix = model_matrix;

        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        spiral_sphere.render();
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);

        //===============================================================================================================================================================================================================
        // done : increment frame counter and show back buffer
        //===============================================================================================================================================================================================================
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}
