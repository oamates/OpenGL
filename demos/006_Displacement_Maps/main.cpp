//========================================================================================================================================================================================================================
// DEMO 006: Displacement Maps
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
    bool pause = false;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
          camera(10.0f, 0.5f, glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
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

        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE))
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        if ((key == GLFW_KEY_KP_SUBTRACT) && (action == GLFW_RELEASE))
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
            pause = !pause;
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
    sphere_tess["bump_tex"] = 1;

    //===================================================================================================================================================================================================================
    // Creating spherical mesh
    //===================================================================================================================================================================================================================
    sphere_t sphere;
    sphere.generate_quads_mt<vertex_t3_t>(sphere_func, 32);
    debug_msg("Sphere generated");

    glActiveTexture(GL_TEXTURE0);                                                           
    GLuint disp_tex_id = image::png::texture2d("../../../resources/tex2d/moon_disp_8192x4096.png");
    //GLuint disp_tex_id = image::png::texture2d("../../../resources/tex2d/moon_disp_2048x1024.png");

    glActiveTexture(GL_TEXTURE1);
    GLuint bump_tex_id = image::png::texture2d("../../../resources/tex2d/moon_normal_8192x4096.png");
    //GLuint bump_tex_id = image::png::texture2d("../../../resources/tex2d/moon_normal_2048x1024.png");

    //===================================================================================================================================================================================================================
    // Global OpenGL settings
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    float t = 0.0f;

    //===================================================================================================================================================================================================================
    // main rendering loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();
    
        const float light_radius = 2.17f;  

        if (!window.pause)
            t += window.frame_dt;

        float t0 = 0.427f * t;
        float t1 = 0.219f * t;
        float cs0 = glm::cos(t0);
        float sn0 = glm::sin(t0);
        float cs1 = glm::cos(t1);
        float sn1 = glm::sin(t1);

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 camera_ws = window.camera.position();
        glm::vec3 light_ws = light_radius * glm::vec3(cs0 * cs1, cs0 * sn1, sn0);

        uniform_projection_view_matrix = projection_view_matrix;
        uniform_camera_ws = camera_ws;
        uniform_light_ws = light_ws;

        sphere.render();
        window.end_frame();
    }
    
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}
