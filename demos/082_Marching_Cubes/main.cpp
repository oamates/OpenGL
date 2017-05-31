//========================================================================================================================================================================================================================
// DEMO 082 : CPU Marching cubes algorithm
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>

#include "log.hpp"
#include "gl_info.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "isosurface.hpp"
#include "image.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;
    bool wireframe_mode = false;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
    }

    //===================================================================================================================================================================================================================
    // mouse handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
            wireframe_mode = !wireframe_mode;

        glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};


glm::dvec3 tri(const glm::dvec3& x)
{
    glm::dvec3 q = glm::abs(glm::fract(x) - glm::dvec3(0.5));
    return glm::clamp(q, 0.05, 0.45);
}

double sdf(const glm::dvec3& p)
{
    glm::dvec3 pp = 8.0 * p;
    glm::dvec3 op = tri(1.1 * pp + tri(1.1 * glm::dvec3(pp.z, pp.x, pp.y)));
    glm::dvec3 q = pp + (op - glm::dvec3(0.25)) * 0.3;
    q = glm::cos(0.444 * q + glm::sin(1.112 * glm::dvec3(pp.z, pp.x, pp.y)));
    return glm::length(q) - 1.05;
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("CPU Marching cubes algorithm", 4, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Load trilinear blend shader which produces nice 3-dimensional material texture from arbitrary 2 dimensional input
    //===================================================================================================================================================================================================================
    glsl_program_t trilinear_blend(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/tb.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/tb.fs"));

    trilinear_blend.enable();
    uniform_t uniform_model_matrix           = trilinear_blend["model_matrix"];
    uniform_t uniform_projection_view_matrix = trilinear_blend["projection_view_matrix"];
    uniform_t uniform_camera_ws              = trilinear_blend["camera_ws"];
    uniform_t uniform_light_ws               = trilinear_blend["light_ws"];
    uniform_t uniform_Ka                     = trilinear_blend["Ka"];
    uniform_t uniform_Kd                     = trilinear_blend["Kd"];
    uniform_t uniform_Ks                     = trilinear_blend["Ks"];
    uniform_t uniform_Ns                     = trilinear_blend["Ns"];
    uniform_t uniform_bf                     = trilinear_blend["bf"];
    trilinear_blend["tb_tex2d"] = 0;

    glActiveTexture(GL_TEXTURE0);
    GLuint stone_tex = image::png::texture2d("../../../resources/tex2d/moss.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);


    //===================================================================================================================================================================================================================
    // light variables
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    isosurface cave;
    cave.generate_vao(sdf);


    //===================================================================================================================================================================================================================
    // main program loop : just clear the buffer in a loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        //===============================================================================================================================================================================================================
        // clear back buffer, process events and update timer
        //===============================================================================================================================================================================================================
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        float time = glfw::time();
        const float light_radius = 43.0f;
        glm::vec3 light_ws = glm::vec3(light_radius * cos(0.5f * time), 25.0f, light_radius * sin(0.5f * time));
        glm::vec3 camera_ws = window.camera.position();

        //===============================================================================================================================================================================================================
        // Render the output of marching cubes algorithm
        //===============================================================================================================================================================================================================
        uniform_projection_view_matrix = projection_view_matrix;
        uniform_light_ws = light_ws;
        uniform_camera_ws = camera_ws;

        uniform_model_matrix = glm::mat4(1.0f);
        uniform_Ka = glm::vec3(0.15f);
        uniform_Kd = glm::vec3(0.77f);
        uniform_Ks = glm::vec3(0.33f);
        uniform_Ns = 20.0f;
        uniform_bf = 0.2875f;

        cave.render();

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}