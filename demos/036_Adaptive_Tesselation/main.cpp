//========================================================================================================================================================================================================================
// DEMO 036: Adaptive Tesselation
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    GLenum mode = GL_FILL;
    bool animate = true;

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

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
        {
            mode = (mode == GL_FILL) ? GL_LINE : GL_FILL;
            glPolygonMode(GL_FRONT_AND_BACK, mode);
        };

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE)) animate = !animate;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

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

    demo_window_t window("Adaptive Tesselation", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Tesselation shader
    //===================================================================================================================================================================================================================
    glsl_program_t tesselator(glsl_shader_t(GL_VERTEX_SHADER,          "glsl/simple.vs" ),
                              glsl_shader_t(GL_TESS_CONTROL_SHADER,    "glsl/simple.tcs"),
                              glsl_shader_t(GL_TESS_EVALUATION_SHADER, "glsl/simple.tes"),
                              glsl_shader_t(GL_FRAGMENT_SHADER,        "glsl/simple.fs" )); 

    tesselator.enable();
    uniform_t uniform_projection_view_matrix = tesselator["projection_view_matrix"];
    uniform_t uniform_camera_ws              = tesselator["camera_ws"];
    uniform_t uniform_base_scale             = tesselator["base_scale"];
    uniform_t uniform_time                   = tesselator["time"];

    uniform_base_scale = 3000.0f;

    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST enabled, GL_LESS - accept fragment if it closer to the camera than the former one
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glPatchParameteri(GL_PATCH_VERTICES, 1);


    float time = 0.0f;
    glActiveTexture(GL_TEXTURE0);
    GLuint water_texture_id = image::png::texture2d("../../../resources/tex2d/water2.png");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);


    //===================================================================================================================================================================================================================
    // the main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        if (window.animate)
        {
            time += 0.01;
            uniform_time = time;
        }

        glm::vec3 camera_ws = window.camera.position();
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        uniform_camera_ws = camera_ws;
        uniform_projection_view_matrix = projection_view_matrix;
        glDrawArraysInstanced(GL_PATCHES, 0, 1, 0x1000);

        window.end_frame();
    }                                  

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}