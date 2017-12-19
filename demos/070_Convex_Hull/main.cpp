//========================================================================================================================================================================================================================
// DEMO 070: Convex hull
//========================================================================================================================================================================================================================
#include <random>
#include <cstdlib>

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
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, time */),
          camera(32.0f, 0.5f, glm::lookAt(glm::vec3(7.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
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
    // init shaders
    //===================================================================================================================================================================================================================
    glsl_program_t hull_shader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/convex_hull.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/convex_hull.fs"));
    hull_shader.enable();
    uniform_t uni_hs_pv_matrix    = hull_shader["projection_view_matrix"];
    uniform_t uni_hs_model_matrix = hull_shader["model_matrix"];
    uniform_t uni_hs_camera_ws    = hull_shader["camera_ws"];
    uniform_t uni_hs_light_ws     = hull_shader["light_ws"];

    glm::mat4 model_matrix = glm::mat4(1.0f);
    uni_hs_model_matrix = model_matrix;

    glsl_program_t normal_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/normals.vs"),
                                   glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/normals.gs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/normals.fs"));
    normal_renderer.enable();
    uniform_t uni_nr_pv_matrix    = normal_renderer["projection_view_matrix"];
    uniform_t uni_nr_model_matrix = normal_renderer["model_matrix"];
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
        glm::dvec3 v = glm::dvec3(2.0, 3.0, 5.0) * glm::dvec3(gauss_dist(randgen), gauss_dist(randgen), gauss_dist(randgen));
        points[i] = v;
    }

    solid stone;
    stone.convex_hull(points);
    stone.fill_buffers();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    //===================================================================================================================================================================================================================
    // program main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float t = 0.8f * window.frame_ts;
        glm::vec3 light_ws = 7.0f * glm::vec3(glm::cos(t), glm::sin(t), 0.0f);
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 camera_ws = window.camera.position();

        hull_shader.enable();
        uni_hs_pv_matrix = projection_view_matrix;
        uni_hs_camera_ws = camera_ws;
        uni_hs_light_ws = light_ws;
        stone.render();

        if (window.show_normals)
        {
            normal_renderer.enable();
            uni_nr_pv_matrix = projection_view_matrix;
            stone.render();
        }

        //===============================================================================================================================================================================================================
        // show back buffer
        //===============================================================================================================================================================================================================
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}