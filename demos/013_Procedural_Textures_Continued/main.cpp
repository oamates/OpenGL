//========================================================================================================================================================================================================================
// DEMO 003 : Procedural Textures
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "vao.hpp"
#include "vertex.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
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

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{

    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Procedural Textures", 8, 4, 0, 1920, 1080);

    //===================================================================================================================================================================================================================
    // procedural texturing shader compilation
    //===================================================================================================================================================================================================================
    glsl_program_t procedural_texturing(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/procedural_texturing.vs"),
                                        glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/procedural_texturing.fs"));
    procedural_texturing.enable();

    uniform_t uniform_shift     = procedural_texturing["shift"];
    uniform_t uniform_pv_matrix = procedural_texturing["projection_view_matrix"];
    uniform_t uniform_time      = procedural_texturing["time"];

    //===================================================================================================================================================================================================================
    // initialize simple (positions only) VAO of regular cube
    //===================================================================================================================================================================================================================
    vertex_p_t vertices [] =
    {
        vertex_p_t(glm::vec3(-1.0f, -1.0f, -1.0f)), 
        vertex_p_t(glm::vec3(-1.0f,  1.0f, -1.0f)),
        vertex_p_t(glm::vec3(-1.0f, -1.0f,  1.0f)),
        vertex_p_t(glm::vec3(-1.0f,  1.0f,  1.0f)), 
        vertex_p_t(glm::vec3( 1.0f, -1.0f, -1.0f)),
        vertex_p_t(glm::vec3( 1.0f,  1.0f, -1.0f)),
        vertex_p_t(glm::vec3( 1.0f, -1.0f,  1.0f)), 
        vertex_p_t(glm::vec3( 1.0f,  1.0f,  1.0f))
    };
    GLubyte indices[] = {0,1,3, 0,3,2, 2,3,6, 3,7,6, 4,1,0, 4,5,1, 2,4,0, 2,6,4, 1,5,3, 5,7,3, 7,5,6, 6,5,4};

    vao_t cube_vao(GL_TRIANGLES, vertices, 8, indices, 36);

    //===================================================================================================================================================================================================================
    // GL_FRAGMENT_SHADER subroutines
    //===================================================================================================================================================================================================================

    const GLuint SUBROUTINE_COUNT = 8;
    const char* subroutine_names[SUBROUTINE_COUNT] = 
    {
        "simplex_turbulence",
        "cellular_turbulence",
        "mercury",
        "plasma",
        "cellular_mercury",
        "marble",
        "marble_color",
        "flow_color"
    };

    GLuint subroutine_index[SUBROUTINE_COUNT];

    for (int i = 0; i < SUBROUTINE_COUNT; ++i)
        subroutine_index[i] = procedural_texturing.subroutine_index(GL_FRAGMENT_SHADER, subroutine_names[i]);


    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST enabled
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.0f, 0.08f, 0.0f);
    glEnable(GL_DEPTH_TEST);    

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        uniform_time = (float) window.frame_ts;
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        uniform_pv_matrix = projection_view_matrix;

        for (GLuint i = 0; i < SUBROUTINE_COUNT; ++i)
        {
            uniform_shift = vertices[i].position;
            uniform_t::subroutine(GL_FRAGMENT_SHADER, &subroutine_index[i]);
            cube_vao.render();
        }

        //===============================================================================================================================================================================================================
        // done : show back buffer and process events
        //===============================================================================================================================================================================================================
        window.end_frame();
    } 

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}