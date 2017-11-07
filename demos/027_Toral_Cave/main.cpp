//========================================================================================================================================================================================================================
// DEMO 027: Toral Cave
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "torus.hpp"

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

float noise_func(glm::vec3 v)
{
    return 0.65f * glm::perlin(0.15f * v) + 0.25f * glm::perlin(0.34f * v) + 0.07f * glm::perlin(0.67f * v);
}

vertex_pnt2_t toral_cave_func(const glm::vec2& uv)
{
    vertex_pnt2_t vertex;
    vertex.uv = uv;

    float cos_2piu = glm::cos(constants::two_pi * uv.x);
    float sin_2piu = glm::sin(constants::two_pi * uv.x);
    float cos_2piv = glm::cos(constants::two_pi * uv.y);
    float sin_2piv = glm::sin(constants::two_pi * uv.y);

    float R = 11.0;
    float r = 7.0f;

    glm::vec3 pos = glm::vec3((R + r * cos_2piu) * cos_2piv, (R + r * cos_2piu) * sin_2piv, sin_2piu);

    vertex.normal = glm::vec3(cos_2piu * cos_2piv, cos_2piu * sin_2piv, sin_2piu);
    vertex.position = pos + noise_func(pos) * vertex.normal;

    return vertex;
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("OBJ Loader", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Phong lighting shader program 
    //===================================================================================================================================================================================================================
    glsl_program_t simple_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/simple_light.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/simple_light.fs"));

    simple_light.enable();
    uniform_t uniform_projection_view_matrix = simple_light["projection_view_matrix"]; 
    uniform_t uniform_model_matrix           = simple_light["model_matrix"];
    uniform_t uniform_light_ws               = simple_light["light_ws"];
    uniform_t uniform_camera_ws              = simple_light["camera_ws"];
    uniform_t uniform_time                   = simple_light["time"];

    //===================================================================================================================================================================================================================
    // Creating toral mesh
    //===================================================================================================================================================================================================================
    torus_t cave;
    cave.generate_vao_mt<vertex_pnt2_t>(toral_cave_func, 40, 40);
    debug_msg("Toral cave generated");

    //===================================================================================================================================================================================================================
    // Global OpenGL settings
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(-1);

    uniform_model_matrix = glm::scale(glm::vec3(30.0f, 30.0f, 90.0f));
    const float light_radius = 140.0f;  

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================

    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        glm::vec4 light_ws = glm::vec4(light_radius * cos(0.5f * time), light_radius * sin(0.5f * time), -0.66f * light_radius, 1.0f);
        glm::vec4 camera_ws = glm::vec4(window.camera.position(), 1.0f);
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        uniform_time = time;
        uniform_light_ws = light_ws;
        uniform_camera_ws = camera_ws;
        uniform_projection_view_matrix = projection_view_matrix;
        cave.render();

        window.end_frame();
    }
    
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}