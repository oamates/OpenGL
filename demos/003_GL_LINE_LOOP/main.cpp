//========================================================================================================================================================================================================================
// DEMO 003 : GL_LINE_LOOP primitive + geometry shader
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "ccurve.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
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


glm::dvec2 cmplx_mul(const glm::dvec2& z, const glm::dvec2& w)
{
    return glm::dvec2(z.x * w.x - z.y * w.y, z.x * w.y + z.y * w.x);
}

glm::dvec2 cmplx_sqr(const glm::dvec2& z)
{
    return glm::dvec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y);
}

template<unsigned int N> glm::dvec2 cmplx_power(glm::dvec2 z)
{
    const unsigned int M = N >> 1;
    return (N & 1) == 0 ? cmplx_sqr(cmplx_power<M>(z)) : 
                          cmplx_mul(cmplx_sqr(cmplx_power<M>(z)), z);
}

template<> glm::dvec2 cmplx_power<1>(glm::dvec2 z)
{
    return z;    
}

//=======================================================================================================================================================================================================================
// PQ - torus knot
//=======================================================================================================================================================================================================================
template<unsigned int P, unsigned int Q, int X, int Y, int Z> glm::dvec3 torus_knot(const glm::dvec2& w)
{
    const double scale = 150.0;
    const double R = 50.0;
    const double r = 25.0;

    glm::dvec2 w_p = cmplx_power<P>(w);
    glm::dvec2 w_q = cmplx_power<Q>(w);

    double radius = R + r * w_q.x;    
    double z = r * w_q.y;
    
    return scale * glm::dvec3(X, Y, Z) + glm::dvec3(radius * w_p.x, radius * w_p.y, z);
}

glm::dvec3 torus_func(const glm::dvec2& w)
{
    return 40.0 * glm::dvec3(0.0, w.x, w.y);
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("GL_LINE_LOOP", 8, 4, 0, 1920, 1080);

    //===================================================================================================================================================================================================================
    // initializing shader and uniforms
    //===================================================================================================================================================================================================================
    glsl_program_t tubes_program(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/tubes.vs"),
                                 glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/tubes.gs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/tubes.fs"));

    tubes_program.enable();

    uniform_t uniform_projection_view_matrix = tubes_program["projection_view_matrix"];
    uniform_t uniform_camera_ws              = tubes_program["camera_ws"]; 
    uniform_t uniform_light_ws               = tubes_program["light_ws"];
    uniform_t uniform_radius                 = tubes_program["radius"]; 
    uniform_t uniform_time                   = tubes_program["time"];
    uniform_t uniform_Ns                     = tubes_program["Ns"];                

    GLuint subroutine_index_cellular_turbulence = tubes_program.subroutine_index(GL_FRAGMENT_SHADER, "cellular_turbulence");
    GLuint subroutine_index_marble_color        = tubes_program.subroutine_index(GL_FRAGMENT_SHADER, "marble_color");
    GLuint subroutine_index_marble              = tubes_program.subroutine_index(GL_FRAGMENT_SHADER, "marble");
    GLuint subroutine_index_mercury             = tubes_program.subroutine_index(GL_FRAGMENT_SHADER, "mercury");
    GLuint subroutine_index_plasma              = tubes_program.subroutine_index(GL_FRAGMENT_SHADER, "plasma");
    GLuint subroutine_index_cellular_mercury    = tubes_program.subroutine_index(GL_FRAGMENT_SHADER, "cellular_mercury");
    GLuint subroutine_index_simplex_turbulence  = tubes_program.subroutine_index(GL_FRAGMENT_SHADER, "simplex_turbulence");

    //===================================================================================================================================================================================================================
    // Line data initialization : generate Frenet frame along smooth periodic (with period = 1) space-curve
    // and draw it as a GL_LINE_LOOP, geometry shader will do the rest
    //===================================================================================================================================================================================================================
    ccurve_t torus(torus_func, 256);

    ccurve_t knot0(torus_knot<1, 8, 0, 0, 1>, 1024);
    ccurve_t knot1(torus_knot<2, 3, 0, 1, 0>, 512);
    ccurve_t knot2(torus_knot<3, 2, 1, 0, 0>, 512);

    ccurve_t knot3(torus_knot<2, 5, 0, 0,-1>, 1024);
    ccurve_t knot4(torus_knot<3, 7, 0,-1, 0>, 1024);
    ccurve_t knot5(torus_knot<4, 5,-1, 0, 0>, 1024);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST enabled, GL_LESS - accept fragment if it closer to the camera than the former one
    // * CULL_FACE enabled 
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.0f, 0.08f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    const float light_radius = 20.0f;

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 camera_ws = window.camera.position(); 

        float cs0 = glm::cos(0.4135677f * time), sn0 = glm::sin(0.4135677f * time),
              cs1 = glm::cos(time), sn1 = glm::sin(time);

        glm::vec3 light_ws = glm::vec3(light_radius * sn1, light_radius * cs0, light_radius * cs1 - 0.5f * light_radius * sn0);

        uniform_time = time;
        uniform_light_ws = light_ws;
        uniform_camera_ws = camera_ws;
        uniform_projection_view_matrix = projection_view_matrix;
        
        uniform_Ns = 100.0f;
        uniform_radius = 14.5f;
        uniform_t::subroutine(GL_FRAGMENT_SHADER, &subroutine_index_cellular_turbulence);
        torus.render();

        uniform_radius = 8.0f;
        uniform_t::subroutine(GL_FRAGMENT_SHADER, &subroutine_index_marble_color);
        knot0.render();

        uniform_radius = 8.0f;
        uniform_t::subroutine(GL_FRAGMENT_SHADER, &subroutine_index_marble);
        knot1.render();

        uniform_radius = 10.5f + cs0;
        uniform_t::subroutine(GL_FRAGMENT_SHADER, &subroutine_index_mercury);
        knot2.render();

        uniform_radius = 11.5f + sn1;
        uniform_t::subroutine(GL_FRAGMENT_SHADER, &subroutine_index_plasma);
        knot3.render();

        uniform_radius = 8.0f;
        uniform_t::subroutine(GL_FRAGMENT_SHADER, &subroutine_index_cellular_mercury);
        knot4.render();

        uniform_radius = 8.0f;
        uniform_t::subroutine(GL_FRAGMENT_SHADER, &subroutine_index_simplex_turbulence);
        knot5.render();

        //===============================================================================================================================================================================================================
        // done : show back buffer
        //===============================================================================================================================================================================================================
        window.end_frame();
    } 

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}