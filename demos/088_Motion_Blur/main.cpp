//========================================================================================================================================================================================================================
// DEMO 087 : Halo around objects
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS
#define GLM_FORCE_NO_CTOR_INIT

#include <array>

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
#include "fbo.hpp"
#include "vertex.hpp"
#include "sampler.hpp"

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

template <unsigned int N> struct Factorial
{
    static const unsigned int value = Factorial<N - 1>::value * N;
};

template <> struct Factorial<0u>
{
    static const unsigned int value = 1;
};

template <unsigned N, unsigned K> struct Binomial
{
    static_assert(N > 0, "N must be non-zero");
    static_assert(K <= N, "K must be between 0 and N-1");
    static const unsigned value = (Factorial<N>::value) / (Factorial<N - K>::value * Factorial<K>::value);
};

template <typename T> static T Pow(T, std::integral_constant<unsigned int, 0u>)
{
    return T(1);
}

template <typename T, unsigned N> static T Pow(T v, std::integral_constant<unsigned, N>)
{
    return v * Pow<T>(v, std::integral_constant<unsigned, N - 1>());
}


// T - interpolated type
// P - parameter type [0, 1]
// N - degree, i.e. linear = 1, quadratic = 2, cubic = 3, ...
template <typename T, typename P, unsigned N> struct Bezier
{
    template <unsigned M, unsigned I> static P _bi(std::integral_constant<unsigned int, M>, std::integral_constant<unsigned int, I>, P t)
    {
        return Binomial<M, I>::value *
            Pow<P>(t, std::integral_constant<unsigned, I>()) *
            Pow<P>(P(1) - t, std::integral_constant<unsigned, M - I>());
    }

    // f(t)
    template <unsigned int I> static T _f(std::integral_constant<unsigned, 0> /*derivation*/, std::integral_constant<unsigned int, I> i, const T* v, P t)
    {
        std::integral_constant<unsigned int, N> n;
        return _bi(n, i, t) * v[I];
    }

    // f'(t)
    template <unsigned int I> static T _f(std::integral_constant<unsigned, 1> /*derivation*/, std::integral_constant<unsigned, I> i, const T* v, P t)
    {
        std::integral_constant<unsigned, N - 1> n_1;
        return _bi(n_1, i, t) * N * (v[I + 1] - v[I]);
    }

    // f''(t)
    template <unsigned I> static T _f(std::integral_constant<unsigned, 2> /*derivation*/, std::integral_constant<unsigned, I> i, const T* v, P t)
    {
        std::integral_constant<unsigned, N-2> n_2;
        return _bi(n_2, i, t) * N*(N - 1) * (v[I+2] - 2*v[I+1] + v[I]);
    }

    template <unsigned D> static T _sum(std::integral_constant<unsigned, D> d, std::integral_constant<unsigned, 0> i, const T* v, P t)
    {
        return _f(d, i, v, t);
    }

    template <unsigned D, unsigned I>
    static T _sum(
        std::integral_constant<unsigned, D> d,
        std::integral_constant<unsigned, I> i,
        const T* v,
        P t
    )
    {
        std::integral_constant<unsigned, I-1> i_1;
        return _sum(d, i_1, v, t) + _f(d, i, v, t);
    }

    /* PUBLIC */
    template <unsigned D> static T B(std::integral_constant<unsigned, D> d, const T* v, size_t s, P t)
    {
        (void) s;
        assert(s >= N);
        std::integral_constant<unsigned, N-D> n_d;
        return _sum(d, n_d, v, t);
    }

    static T Position(const T* v, size_t s, P t)
    {
        std::integral_constant<unsigned, 0> d;
        return B(d, v, s, t);
    }

    static T Derivative1(const T* v, size_t s, P t)
    {
        std::integral_constant<unsigned, 1> d;
        return B(d, v, s, t);
    }

    static T Derivative2(const T* v, size_t s, P t)
    {
        std::integral_constant<unsigned, 2> d;
        return B(d, v, s, t);
    }
};

// T - interpolated type
// W - weight type
// P - parameter type [0, 1]
// N - degree, i.e. linear = 1, quadratic = 2, cubic = 3, ...
template <typename T, typename W, typename P, unsigned N> struct NURBS
{
    template <unsigned I> static T _part(std::integral_constant<unsigned, I>, const T* v, const W* w, P t)
    {
        return Binomial<N, I>::value *
            Pow<P>(t, std::integral_constant<unsigned, I>()) *
            Pow<P>(P(1)-t, std::integral_constant<unsigned, N - I>()) * v[I] * w[I];
    }

    static T _sum(std::integral_constant<unsigned, 0> i, const T* v, const W* w, P t)
    {
        return _part(i, v, w, t);
    }

    template <unsigned I> static T _sum(std::integral_constant<unsigned, I> i, const T* v, const W* w, P t)
    {
        std::integral_constant<unsigned, I-1> i_1;
        return _sum(i_1, v, w, t) + _part(i, v, w, t);
    }

    template <unsigned I> static W _part(std::integral_constant<unsigned, I>, const W* w, P t)
    {
        return Binomial<N, I>::value *
            Pow<P>(t, std::integral_constant<unsigned, I>()) *
            Pow<P>(P(1)-t, std::integral_constant<unsigned, N-I>())*
            w[I];
    }

    static W _sum(std::integral_constant<unsigned, 0> i, const W* w, P t)
    {
        return _part(i, w, t);
    }

    template <unsigned I>
    static W _sum(std::integral_constant<unsigned, I> i, const W* w, P t)
    {
        std::integral_constant<unsigned, I-1> i_1;
        return _sum(i_1, w, t) + _part(i, w, t);
    }

    /* public */
    static T Calc(const T* v, const W* w, size_t s, P t)
    {
        assert(s >= N);
        std::integral_constant<unsigned, N> n;
        return _sum(n, v, w, t)*(W(1)/_sum(n, w, t));
    }
};


template <typename Type, typename Parameter, unsigned Order> struct BezierCurves
{
    ::std::vector<Type> _points;
    bool _connected;

    static bool Connected(const ::std::vector<Type>& points)
        { return ((points.size() - 1) % Order) == 0; }

    bool Connected(void) const                                              // Returns true if the individual curves are connected
        { return _connected; }

    static bool Separated(const ::std::vector<Type>& points)
        { return (points.size() % (Order + 1)) == 0; }

    bool Separated(void) const                                              // Returns true if the individual curves are connected
        { return !_connected; }

    static bool PointsOk(const ::std::vector<Type>& points)                 // Checks if the sequence of control points is OK for this curve type
    {
        if (points.empty())
            return false;
        return (Connected(points) || Separated(points));
    }

    // Creates the bezier curves from the control points
    // The number of points must be ((C * Order) + 1) or (C * (Order + 1)) where C is the number of curves (segments) in the sequence.
    // If both of the above are true then the curves are considered to be connected.
    BezierCurves(std::vector<Type>&& points)
        : _points(std::move(points)), _connected(Connected(_points))
        { assert(PointsOk(_points)); }

    // Creates the bezier curves from the control points
    // The number of points must be ((C * Order) + 1) and connected or (C * (Order + 1)) and not(connected),
    // where C is the number of curves (segments) in the sequence.
    BezierCurves(::std::vector<Type>&& points, bool connected)
        : _points(std::move(points)), _connected(connected)
    {
        assert(PointsOk(_points));
        assert(Connected(_points) == _connected);
    }

    // Creates the bezier curves from the control points
    // The number of points must be (C * Order + 1) where C is the number of curves (segments) in the sequence.
    // If both of the above are true then the curves are considered to be connected.
    BezierCurves(const ::std::vector<Type>& points)
        : _points(points), _connected(Connected(_points))
        { assert(PointsOk(_points)); }

    // Creates the bezier curves from the control points
    // The number of points must be ((C * Order) + 1) and connected or (C * (Order + 1)) and not(connected),
    // where C is the number of curves (segments) in the sequence.
    BezierCurves(const ::std::vector<Type>& points, bool connected)
        : _points(points), _connected(connected)
    {
        assert(PointsOk(_points));
        assert(Connected(_points) == _connected);
    }

    template <std::size_t N> BezierCurves(const std::array<Type, N>& points)
        : _points(points.begin(), points.end()), _connected(Connected(_points))
        { assert(PointsOk(_points)); }

    template <std::size_t N> BezierCurves(const ::std::array<Type, N>& points, bool connected)
        : _points(points.begin(), points.end()), _connected(connected)
    {
        assert(PointsOk(_points));
        assert(Connected(_points) == _connected);
    }

    unsigned int SegmentStep() const
    {
        assert(PointsOk(_points));
        return _connected ? Order : Order + 1;
    }

    unsigned int SegmentCount() const                                       // Returns the count of individual curves in the sequence
    {
        assert(PointsOk(_points));
        return _connected ? (unsigned int)((_points.size() - 1) / Order) : (unsigned int)(_points.size() / (Order + 1));
    }

    const std::vector<Type>& ControlPoints() const                          // Returns the contol points of the curve
        { return _points; }

    static Parameter Wrap(Parameter t)                                      // Wraps the parameter value to [0.0, 1.0]
    {
        const Parameter zero(0);
        const Parameter one(1);
        if (t < zero)
            t += std::floor(std::fabs(t))+one;
        else
            if (t > one)
                t -= std::floor(t);
        assert(t >= zero && t <= one);
        return t;
    }

    Type Position01(Parameter t) const                                      // Gets the point on the curve at position t (must be between 0.0, 1.0)
    {
        const Parameter zero(0);
        const Parameter one(1);

        if (t == one)
            t = zero;
        assert(t >= zero && t < one);

        Parameter toffs = t * SegmentCount();

        unsigned poffs = unsigned(toffs) * SegmentStep();

        assert(poffs < _points.size() - Order);
        Parameter t_sub = toffs - std::floor(toffs);
        return Bezier<Type, Parameter, Order>::Position(_points.data() + poffs, _points.size() - poffs, t_sub);
    }

    Type Position(Parameter t) const                                        // Gets the point on the curve at position t wrapped to [0.0, 1.0]
        { return Position01(Wrap(t)); }

    void Approximate(std::vector<Type>& dest, unsigned n) const             // Makes a sequence of points on the curve (n points per segment)
    {
        unsigned sstep = SegmentStep();
        unsigned s = SegmentCount();

        dest.resize(s * n + 1);

        auto p = dest.begin();
        const Parameter t_step = Parameter(1) / n;

        for(unsigned int i=0; i!=s; ++i)
        {
            unsigned int poffs = i*sstep;
            Parameter t_sub = Parameter(0);
            const Type* data = _points.data() + poffs;
            std::size_t size = _points.size() - poffs;
            for(unsigned int j = 0; j != n; ++j)
            {
//                typedef  bezier;
                assert(p != dest.end());
                *p = Type(Bezier<Type, Parameter, Order>::Position(data, size, t_sub));
                ++p;
                t_sub += t_step;
            }
        }
        assert(p != dest.end());
        *p = _points.back();
        ++p;
        assert(p == dest.end());
    }

    std::vector<Type> Approximate(unsigned n) const                         // Returns a sequence of points on the curve (n points per segment)
    {
        std::vector<Type> result;
        Approximate(result, n);
        return result;
    }

    BezierCurves<Type, Parameter, Order - 1> Derivative(void) const         // Returns a derivative of this curve
    {
        unsigned sstep = SegmentStep();
        unsigned s = SegmentCount();

        std::vector<Type> new_points(s * Order);
        typename std::vector<Type>::iterator p = new_points.begin();

        for(unsigned int i = 0; i != s; ++i)
        {
            for(unsigned int j = 0; j != Order; ++j)
            {
                unsigned int k = i * sstep + j;
                assert(p != new_points.end());
                *p = (_points[k + 1] - _points[k]) * Order;
                ++p;
            }
        }
        assert(p == new_points.end());
        return BezierCurves<Type, Parameter, Order - 1>(std::move(new_points), false);
    }
};

// A closed smooth cubic Bezier spline passing through all input points
template <typename Type, typename Parameter> struct CubicBezierLoop : public BezierCurves<Type, Parameter, 3>
{
    template <typename StdRange> static std::vector<Type> _make_cpoints(const StdRange& points, Parameter r)
    {
        std::size_t i = 0, n = points.size();
        assert(n != 0);
        std::vector<Type> result(n * 3 + 1);
        typename std::vector<Type>::iterator ir = result.begin();

        while(i != n)
        {
            unsigned int a = (unsigned int)(( n + i - 1 ) % n);
            unsigned int b = (unsigned int)(i);
            unsigned int c = (unsigned int)(( i + 1 ) % n);
            unsigned int d = (unsigned int)(( i + 2 ) % n);
            assert(ir != result.end());
            *ir = points[b];
            ++ir;
            assert(ir != result.end());
            *ir = Type(points[b] + (points[c] - points[a]) * r);
            ++ir;
            assert(ir != result.end());
            *ir = Type(points[c] + (points[b] - points[d]) * r);
            ++ir;
            ++i;
        }
        assert(ir != result.end());
        *ir = points[0]; ++ir;
        assert(ir == result.end());
        return result;
    }

    // Creates a loop passing through the sequence of the input points
    CubicBezierLoop(const ::std::vector<Type>& points, Parameter r = Parameter(1) / Parameter(3))
        : BezierCurves<Type, Parameter, 3>(_make_cpoints(points, r))
    { }

    template <std::size_t N> CubicBezierLoop(const ::std::array<Type, N>& points, Parameter r = Parameter(1) / Parameter(3))
        : BezierCurves<Type, Parameter, 3>(_make_cpoints(points, r))
    { }
};


struct instances_t
{
    static std::vector<glm::vec3> make_positions()
    {
        std::vector<glm::vec3> pos_data;
        pos_data.reserve(17);
        pos_data.push_back(glm::vec3( 10.0f,  0.0f, 70.0f));
        pos_data.push_back(glm::vec3( 60.0f,  0.0f, 60.0f));
        pos_data.push_back(glm::vec3( 95.0f,  0.0f,  0.0f));
        pos_data.push_back(glm::vec3( 60.0f,  0.0f,-60.0f));
        pos_data.push_back(glm::vec3( 10.0f,  0.0f,-70.0f));
        pos_data.push_back(glm::vec3(-30.0f, 20.0f,-70.0f));
        pos_data.push_back(glm::vec3(-70.0f, 40.0f,-70.0f));
        pos_data.push_back(glm::vec3(-90.0f, 40.0f,-30.0f));
        pos_data.push_back(glm::vec3(-30.0f, 40.0f, 10.0f));
        pos_data.push_back(glm::vec3( 40.0f, 40.0f, 10.0f));
        pos_data.push_back(glm::vec3( 90.0f,  0.0f,  0.0f));
        pos_data.push_back(glm::vec3( 50.0f,-40.0f,-10.0f));
        pos_data.push_back(glm::vec3(  0.0f,-40.0f,-10.0f));
        pos_data.push_back(glm::vec3(-50.0f,-40.0f,-10.0f));
        pos_data.push_back(glm::vec3(-90.0f,-40.0f, 30.0f));
        pos_data.push_back(glm::vec3(-70.0f,-40.0f, 70.0f));
        pos_data.push_back(glm::vec3(-30.0f,-20.0f, 70.0f));
        return pos_data;
    }

    static std::vector<glm::vec3> make_normals()
    {
        std::vector<glm::vec3> nml_data;
        nml_data.reserve(17);
        nml_data.push_back(glm::vec3(  0.0f,  1.0f,  0.0f));
        nml_data.push_back(glm::vec3(  0.0f,  1.0f, -1.0f));
        nml_data.push_back(glm::vec3( -1.0f,  0.0f,  0.0f));
        nml_data.push_back(glm::vec3(  0.0f,  1.0f,  2.0f));
        nml_data.push_back(glm::vec3(  0.0f,  1.0f,  1.0f));
        nml_data.push_back(glm::vec3(  1.0f,  1.0f,  0.5f));
        nml_data.push_back(glm::vec3(  0.0f,  1.0f,  1.0f));
        nml_data.push_back(glm::vec3(  1.0f,  0.0f,  0.0f));
        nml_data.push_back(glm::vec3(  0.0f,  1.0f, -1.0f));
        nml_data.push_back(glm::vec3(  0.0f,  1.0f,  0.0f));
        nml_data.push_back(glm::vec3(  1.0f,  0.0f,  0.0f));
        nml_data.push_back(glm::vec3(  0.0f, -1.0f,  1.0f));
        nml_data.push_back(glm::vec3(  0.0f,  0.0f,  1.0f));
        nml_data.push_back(glm::vec3(  0.0f,  1.0f,  1.0f));
        nml_data.push_back(glm::vec3(  1.0f,  0.0f,  0.0f));
        nml_data.push_back(glm::vec3(  0.0f,  1.0f, -1.0f));
        nml_data.push_back(glm::vec3( -1.0f,  1.0f,  0.0f));
        for(int i = 0; i < 17; ++i)
            nml_data[i] = glm::normalize(nml_data[i]);
        return nml_data;
    }

    CubicBezierLoop<glm::vec3, double> path_pos;
    CubicBezierLoop<glm::vec3, double> path_nml;
    GLuint count;

    instances_t(GLuint ubo_id, GLuint n = 256)
        : path_pos(make_positions(), 0.25)
        , path_nml(make_normals(), 0.25)
    {
        count = 2 * n;
        // inst_count x 4x4 matrices
        std::vector<glm::mat4> model_matrix(count);

        double step = 1.0 / n;

        int index = 0;

        for (GLuint i = 0; i != n; ++i)
        {
            glm::vec3 pos = path_pos.Position(i * step);
            glm::vec3 tgt = glm::normalize(path_pos.Position((i + 1) * step) - path_pos.Position((i - 1) * step));
            glm::vec3 tmp = path_nml.Position(i * step);
            glm::vec3 nml = glm::normalize(tmp - tgt * glm::dot(tmp, tgt));
            glm::vec3 btg = glm::cross(nml, tgt);

            for(GLuint j = 0; j != 2; ++j)
            {
                const GLfloat s[2] = {-3.0f, 3.0f};
                model_matrix[index++] = glm::mat4(
                                            glm::vec4(btg, 0.0f),
                                            glm::vec4(nml, 0.0f),
                                            glm::vec4(tgt, 0.0f),
                                            glm::vec4(pos + btg * s[j], 1.0f)
                                        );
            }
        }

        glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * model_matrix.size(), model_matrix.data(), GL_STATIC_DRAW);
    }

    glm::vec3 Position(double t) const
    {
        return path_pos.Position(t);
    }

    glm::vec3 Normal(double t) const
    {
        return path_nml.Position(t);
    }

    glm::mat4 MakeMatrix(double t, double dt) const
    {
        glm::vec3 pos = Position(t);
        glm::vec3 tgt = glm::normalize(Position(t + dt) - Position(t - dt));
        glm::vec3 tmp = Normal(t + dt);
        float dtt = glm::dot(tmp, tgt);
        glm::vec3 nml = glm::normalize(tmp - tgt * dtt);
        glm::vec3 btg = glm::cross(nml, tgt);

        return glm::transpose(
            glm::mat4(
                glm::vec4(btg, 0.0f),
                glm::vec4(nml, 0.0f),
                glm::vec4(tgt, 0.0f),
                glm::vec4(pos, 1.0f)
            )
        );
    }
};


struct blur_fbo_t
{
    GLuint tex_id[2];                                       // Array<Texture> tex;
    GLuint fbo_id;                                          // Framebuffer fbo;
    GLuint rbo_id;                                          // Renderbuffer rbo;
    GLuint res_x, res_y;

    sampler_t tex0_sampler;
    sampler_t tex1_sampler;

    blur_fbo_t(int res_x, int res_y)
        : res_x(res_x), res_y(res_y),
          tex0_sampler(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE),
          tex1_sampler(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
    {
        glGenTextures(2, tex_id);
        for (GLuint u = 0; u != 2; ++u)
        {
            glActiveTexture(GL_TEXTURE0 + u);
            glBindTexture(GL_TEXTURE_RECTANGLE, tex_id[u]);
            glTexStorage2D(GL_TEXTURE_RECTANGLE, 1, GL_RGBA8, res_x, res_y);
        }

        glGenFramebuffers(1, &fbo_id);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);

        glGenRenderbuffers(1, &rbo_id);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, res_x, res_y);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_id);

        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_id[0], 0);
        check_status();

        tex0_sampler.bind(0);
        tex1_sampler.bind(1);
    }


    void bind()
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);
    }

    void accumulate()
    {
        glActiveTexture(GL_TEXTURE1);
        glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA8, 0, 0, res_x, res_y, 0);
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
    // compile shaders
    //===================================================================================================================================================================================================================
    glsl_program_t draw_prog(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/draw.vs"),
                             glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/draw.fs"));

    uniform_t projection_matrix = draw_prog["ProjectionMatrix"];
    uniform_t camera_matrix     = draw_prog["CameraMatrix"];
    uniform_t model_matrix      = draw_prog["SingleModelMatrix"];
    uniform_t single_model      = draw_prog["SingleModel"];                                     // int
    uniform_t checker_tex       = draw_prog["CheckerTex"];                                      // UniformSampler
    //uniform_t model_block = draw_prog["ModelBlock"];                                          // UniformSampler
    //uniform_t UniformBlock model_block;
    draw_prog.enable();
    projection_matrix = window.camera.projection_matrix;
    checker_tex = 2;

    glsl_program_t blur_prog(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/blur.vs"),
                             glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blur.fs"));

    uniform_t screen_size     = blur_prog["ScreenSize"];                                        // Uniform<Vec2f>
    uniform_t current_frame   = blur_prog["CurrentFrame"];
    uniform_t previous_frames = blur_prog["PreviousFrames"];                                    // UniformSampler
    uniform_t splitter        = blur_prog["Splitter"];                                          // Uniform<GLfloat>

    blur_prog.enable();
    screen_size = glm::vec2(res_x, res_y);
    current_frame = 0;
    previous_frames = 1;




    //===================================================================================================================================================================================================================
    // load cube texture --> unit 2
    //===================================================================================================================================================================================================================
    GLuint tex_id;
    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    int width, height, bpp;
    unsigned char* src_data = stbi_load("../../../resources/tex2d/copper.png", &width, &height, &bpp, 0);

    if (!src_data)
        exit_msg("Cannot load texture: ../../../resources/tex2d/copper.png");

    GLenum format = (bpp == 1) ? GL_RED : (bpp == 2) ? GL_RG : (bpp == 3) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, src_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    free(src_data);

    sampler_t sampler(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT)
    sampler.bind(2);




    glClearColor(0.04f, 0.01f, 0.09f, 0.0f);
    glClearDepth(1.0f);

    shapes::ShapeWrapper cube;
    shapes::ShapeWrapper arrow;

    shapes::ShapeWrapper screen;
    blur_fbo_t blur_fbo;


    //===================================================================================================================================================================================================================
    // model matrices uniform block setup
    //===================================================================================================================================================================================================================
    GLuint ubo_id;
    glGenBuffers(1, &ubo_id);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(matrices), 0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_id);
    draw_prog.bind_ubo("ModelBlock", 0);

    instances_t instances(ubo_id);

/*
    MotionBlurExample()
        , instances()
        , cube(List("Position")("Normal")("TexCoord").Get(), shapes::Cube(), draw_prog)
        , arrow(List("Position")("Normal").Get(), ArrowShape(), draw_prog)
        , screen(List("Position")("TexCoord").Get(), shapes::Screen(), blur_prog)
    {
    }
*/


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

        int samples = 8;
        double frame_time = clock.Now().Seconds();
        double interval = clock.Interval().Seconds();
        double step = interval / samples;

        for(int s = 0; s != samples; ++s)
        {
            double time = frame_time + s * step;


            blur_fbo.bind();                                            // draw objects off-screen

            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            double pos = time / 20.0;

            draw_prog.enable();
            camera_matrix = view_matrix;
            single_model = 0;

            cube_vao.bind();
            cube_vao.instanced_render(instances.count);

            single_model = 1;

            arrow.Use();
            model_matrix = instances.MakeMatrix(pos - 0.007 + glm::sin(time / 13.0) * 0.007, 0.001);
            arrow.Draw();

            model_matrix = instances.MakeMatrix(pos + 0.013 + glm::sin(time / 7.0) * 0.007, 0.001);
            arrow.Draw();

            // motion blur
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glDisable(GL_DEPTH_TEST);

            blur_prog.Use();

            screen.Use();
            screen.Draw();

            blur_fbo.accumulate();
        }

        splitter = (glm::sin(frame_time / 20.0) * 0.5 + 0.5) * res_x;

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



class ArrowShape
 : public ResourceFile
 , public shapes::ObjMesh
{
    ArrowShape()
        : ResourceFile("models", "arrow_z", ".obj")
        , shapes::ObjMesh(stream(), LoadingOptions(false).Normals())
    { }
};

