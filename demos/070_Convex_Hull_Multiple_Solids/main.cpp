//========================================================================================================================================================================================================================
// DEMO 070: Convex Hull :: Multiple Solids
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS
#define GLM_FORCE_NO_CTOR_INIT

#include <random>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/noise.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "hull3d.hpp"
#include "shader.hpp"
#include "solid.hpp"

extern bool show_normals;


int main(int argc, char *argv[])
{
    const int res_x = 1920;
    const int res_y = 1080;
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Convex Hull :: Multiple Solids", 8, 3, 3, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // init shader
    //===================================================================================================================================================================================================================
    glClearColor(0.05f, 0.0f, 0.15f, 0.0f);                                                                                 // set dark blue background

    // ==================================================================================================================================================================================================================
    // init camera
    // ==================================================================================================================================================================================================================
    init_camera(window);
    const float two_pi = 6.283185307179586476925286766559;
    glm::mat4 projection_matrix = glm::infinitePerspective (two_pi / 6.0f, float(res_x) / float(res_y), 0.1f);                                      // projection matrix : 60° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units

    // ==================================================================================================================================================================================================================
    // init shader
    // ==================================================================================================================================================================================================================
    glsl_program hull_shader(glsl_shader(GL_VERTEX_SHADER,   "glsl/convex_hull.vs"),
                             glsl_shader(GL_FRAGMENT_SHADER, "glsl/convex_hull.fs"));
    hull_shader.enable();
    uniform_t uni_hs_model_matrix      = hull_shader["model_matrix"];
    uniform_t uni_hs_view_matrix       = hull_shader["view_matrix"];
    uniform_t uni_hs_projection_matrix = hull_shader["projection_matrix"];
    uniform_t uni_hs_light_ws          = hull_shader["light_ws"];

    glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glm::mat4 model_matrix = glm::mat4(1.0f);
    glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, glm::value_ptr(model_matrix));

    glsl_program_t normal_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/normals.vs"),
                                   glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/normals.gs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/normals.fs"));
    normal_renderer.enable();

    GLint model_matrix_id_n = normal_renderer.uniform_id("model_matrix");
    GLint view_matrix_id_n = normal_renderer.uniform_id("view_matrix");
    GLint projection_matrix_id_n = normal_renderer.uniform_id("projection_matrix");

    glUniformMatrix4fv(projection_matrix_id_n, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniformMatrix4fv(model_matrix_id_n, 1, GL_FALSE, glm::value_ptr(model_matrix));


    // ==================================================================================================================================================================================================================
    // create point cloud
    // ==================================================================================================================================================================================================================

//  std::random_device rd;
    std::mt19937 randgen(215387276);
    std::normal_distribution<double> gauss_dist;

    const int CLOUD_SIZE = 32;
    std::vector<glm::dvec3> points;
    points.resize(CLOUD_SIZE);

    solid stone;


    for(int i = 0; i < CLOUD_SIZE; ++i)
    {
        glm::dvec3 v = 5.0 * glm::normalize(glm::dvec3(gauss_dist(randgen), gauss_dist(randgen), gauss_dist(randgen)));
        points[i] = v;
    };

    stone.convex_hull(points);
    stone.fill_buffers();

    int vindex, tindex;
    vindex = 0;
    tindex = 0;
    double max_x = stone.support_hc(glm::dvec3( 1.0, 0.0, 0.0), vindex, tindex);
    debug_msg("max_x = %f. vindex = %d. tindex = %d", max_x, vindex, tindex);

    vindex = 0;
    tindex = 0;
    double min_x = stone.support_hc(glm::dvec3(-1.0, 0.0, 0.0), vindex, tindex);
    debug_msg("max_x = %f. vindex = %d. tindex = %d", min_x, vindex, tindex);

    vindex = 0;
    tindex = 0;
    double max_y = stone.support_hc(glm::dvec3(0.0,  1.0, 0.0), vindex, tindex);
    debug_msg("max_y = %f. vindex = %d. tindex = %d", max_y, vindex, tindex);

    vindex = 0;
    tindex = 0;
    double min_y = stone.support_hc(glm::dvec3(0.0, -1.0, 0.0), vindex, tindex);
    debug_msg("max_y = %f. vindex = %d. tindex = %d", min_y, vindex, tindex);

    vindex = 0;
    tindex = 0;
    double max_z = stone.support_hc(glm::dvec3(0.0, 0.0,  1.0), vindex, tindex);
    debug_msg("max_z = %f. vindex = %d. tindex = %d", max_z, vindex, tindex);

    vindex = 0;
    tindex = 0;
    double min_z = stone.support_hc(glm::dvec3(0.0, 0.0, -1.0), vindex, tindex);
    debug_msg("max_z = %f. vindex = %d. tindex = %d", min_z, vindex, tindex);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // ==================================================================================================================================================================================================================
    // main loop
    // ==================================================================================================================================================================================================================


    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                                                                 // clear the screen

        float t = glfwGetTime();
        glm::vec4 light_position = glm::vec4(70.0f * glm::cos(0.8f * t), 70.0f * glm::sin(0.8f * t), 0.0f, 1.0f);

        hull_shader.enable();
        glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(view_matrix));
        glUniform4fv(light_position_id, 1, glm::value_ptr(light_position));
        stone.render();

        if (show_normals)
        {
            normal_renderer.enable();
            glUniformMatrix4fv(view_matrix_id_n, 1, GL_FALSE, glm::value_ptr(view_matrix));
            stone.render();
        };

        glfwSwapBuffers(window);                                                                                            // swap buffers
        glfwPollEvents();
    };
    glfwTerminate();                                                                                                        // close OpenGL window and terminate GLFW
    return 0;
}