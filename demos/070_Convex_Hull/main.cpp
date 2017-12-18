//========================================================================================================================================================================================================================
// DEMO 070: Convex hull :: multiple solids
//========================================================================================================================================================================================================================
#include <algorithm>
#include <iostream>
#include <random>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "solid.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "hull3d.hpp"


struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    bool show_normals = false;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, time */)
    {
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
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

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
            show_normals = !show_normals;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("VAO Loader", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // init shader
    //===================================================================================================================================================================================================================
    glsl_program_t hull_shader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/convex_hull.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/convex_hull.fs"));
    hull_shader.enable();

    uniform_t uni_hs_model_matrix      = hull_shader["model_matrix"];
    uniform_t uni_hs_view_matrix       = hull_shader["view_matrix"];
    uniform_t uni_hs_projection_matrix = hull_shader["projection_matrix"];
    uniform_t uni_hs_light_position    = hull_shader["light_ws"];

    uni_hs_projection_matrix = window.camera.projection_matrix;
    glm::mat4 model_matrix = glm::mat4(1.0f);
    uni_hs_model_matrix = model_matrix;

    glsl_program_t normal_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/normals.vs"),
                                   glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/normals.gs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/normals.fs"));
    normal_renderer.enable();

    uniform_t uni_nr_model_matrix      = normal_renderer["model_matrix"];
    uniform_t uni_nr_view_matrix       = normal_renderer["view_matrix"];
    uniform_t uni_nr_projection_matrix = normal_renderer["projection_matrix"];

    uni_nr_projection_matrix = window.camera.projection_matrix;
    uni_nr_model_matrix = model_matrix;


    //===================================================================================================================================================================================================================
    // create point cloud
    //===================================================================================================================================================================================================================
    std::random_device rd;
    std::mt19937 randgen(rd());
    std::normal_distribution<double> gauss_dist;

    const int CLOUD_SIZE = 128;
    std::vector<glm::dvec3> points;
    points.resize(CLOUD_SIZE);

    for(int i = 0; i < CLOUD_SIZE; ++i)
    {
        glm::dvec3 v = glm::dvec3(gauss_dist(randgen), gauss_dist(randgen), gauss_dist(randgen));
        points[i] = v;
    }

    solid stone1;
    stone1.convex_hull(points);
    stone1.fill_buffers();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    //===================================================================================================================================================================================================================
    // program main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        glm::vec4 light_ws = glm::vec4(70.0f * glm::cos(0.8f * time), 70.0f * glm::sin(0.8f * time), 0.0f, 1.0f);

        hull_shader.enable();
        uni_hs_view_matrix = window.camera.view_matrix;
        uni_ls_light_ws = light_position;
        stone1.render();

        if (window.show_normals)
        {
            normal_renderer.enable();
            view_matrix_id_n = window.camera.view_matrix;
            stone1.render();
        }

        //===============================================================================================================================================================================================================
        // show back buffer
        //===============================================================================================================================================================================================================
        window.new_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}