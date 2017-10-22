//========================================================================================================================================================================================================================
// DEMO 006: Displacement Maps
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "sphere.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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

vertex_t3_t sphere_func(const glm::vec3& direction)
{
    vertex_t3_t vertex;
    vertex.uvw = normalize(direction);
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

    demo_window_t window("Displacement Maps", 4, 4, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Phong lighting shader program 
    //===================================================================================================================================================================================================================
    glsl_program_t sphere_tess(glsl_shader_t(GL_VERTEX_SHADER,          "glsl/sphere_tess.vs"),
                               glsl_shader_t(GL_TESS_CONTROL_SHADER,    "glsl/sphere_tess.tcs"),
                               glsl_shader_t(GL_TESS_EVALUATION_SHADER, "glsl/sphere_tess.tes"),
                               glsl_shader_t(GL_FRAGMENT_SHADER,        "glsl/sphere_tess.fs"));


    sphere_tess.enable();
    uniform_t uniform_projection_view_matrix = sphere_tess["projection_view_matrix"]; 
    uniform_t uniform_light_ws               = sphere_tess["light_ws"];
    uniform_t uniform_camera_ws              = sphere_tess["camera_ws"];
    sphere_tess["disp_tex"] = 0;

    //===================================================================================================================================================================================================================
    // Creating spherical mesh
    //===================================================================================================================================================================================================================
    sphere_t sphere;
    sphere.generate_vao_mt<vertex_t3_t>(sphere_func, 32);
    debug_msg("Sphere generated");

    //===================================================================================================================================================================================================================
    // Global OpenGL settings
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(-1);
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    const float light_radius = 4.5f;  

    glActiveTexture(GL_TEXTURE0);
    GLuint pentagon_texture_id = image::png::texture2d("../../../resources/tex2d/moon_disp_8192x4096.png");
    glBindTexture(GL_TEXTURE_2D, pentagon_texture_id);


    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================

    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = glfw::time();

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        glm::vec3 camera_ws = glm::vec4(window.camera.position(), 1.0f);
        glm::vec3 light_ws = glm::vec4(light_radius * cos(0.5f * time), light_radius * sin(0.5f * time), -0.66f * light_radius, 1.0f);

        uniform_projection_view_matrix = projection_view_matrix;
        uniform_camera_ws = camera_ws;
        uniform_light_ws = light_ws;

        sphere.render(GL_PATCHES);
        window.end_frame();
    }
    
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}
