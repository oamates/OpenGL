//========================================================================================================================================================================================================================
// DEMO 048: Hyperbolic Space Tesselation
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "gl_aux.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "hyperbolic.hpp"

struct demo_window_t : public glfw_window_t
{
    hyperbolic_camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */),
          camera(0.0625, 0.125)
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
         
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 4.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Hyperbolic Space", 4, 4, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Creating shaders and uniforms
    //===================================================================================================================================================================================================================
    glsl_program_t hyperbolic(glsl_shader_t(GL_VERTEX_SHADER,          "glsl/hyperbolic.vs" ),
                              glsl_shader_t(GL_TESS_CONTROL_SHADER,    "glsl/hyperbolic.tcs"),
                              glsl_shader_t(GL_TESS_EVALUATION_SHADER, "glsl/hyperbolic.tes"),
                              glsl_shader_t(GL_GEOMETRY_SHADER,        "glsl/hyperbolic.gs" ),
                              glsl_shader_t(GL_FRAGMENT_SHADER,        "glsl/hyperbolic.fs" ));                                            

    hyperbolic.enable();
    uniform_t uniform_view_matrix = hyperbolic["view_matrix"];
    hyperbolic["pentagon_tex"] = 0;

    //===================================================================================================================================================================================================================
    // Right-angled hyperbolic dodecahedron triangulation initialization
    //===================================================================================================================================================================================================================
    GLuint vao_id = hyperbolic::dodecahedron::vao();
    GLuint ibo_id = hyperbolic::dodecahedron::ibo();
    GLuint ssbo_id = hyperbolic::dodecahedron::generate_SSBO5();

    //===================================================================================================================================================================================================================
    // Camera, view_matrix and projection_matrix initialization
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    glActiveTexture(GL_TEXTURE0);
    GLuint pentagon_texture_id = image::png::texture2d("../../../resources/tex2d/pentagon_green.png");
    glBindTexture(GL_TEXTURE_2D, pentagon_texture_id);

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================

    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();
        glm::mat4 view_matrix = glm::mat4(window.camera.view_matrix);
        uniform_view_matrix = view_matrix;
        glDrawElementsInstanced(GL_PATCHES, 180, GL_UNSIGNED_BYTE, 0, 57741);
        window.end_frame();
    }
    
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}