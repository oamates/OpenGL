//========================================================================================================================================================================================================================
// DEMO 051 : Crystal RayMarch
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
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "glsl_noise.hpp"
#include "plato.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "polyhedron.hpp"
#include "image.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.01f);
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
    int res_x = 1920;
    int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Crystal RayMarch", 4, 3, 3, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // skybox and environment map shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t skybox_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/skybox.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/skybox.fs"));

    skybox_renderer.enable();
    uniform_t uni_sbox_pv_matrix = skybox_renderer["projection_view_matrix"];
    skybox_renderer["environment_tex"] = 0;

    //===================================================================================================================================================================================================================
    // Initialize cube buffer : vertices + indices
    //===================================================================================================================================================================================================================
    GLuint cube_vao_id, cube_vbo_id, cube_ibo_id;

    glGenVertexArrays(1, &cube_vao_id);
    glBindVertexArray(cube_vao_id);
    glGenBuffers(1, &cube_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_id);

    static const GLfloat cube_vertices[] =
    {
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f
    };

    static const GLushort cube_indices[] =
    {
        0, 1, 2, 3, 6, 7, 4, 5,
        2, 6, 0, 4, 1, 5, 3, 7
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &cube_ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    //===================================================================================================================================================================================================================
    // volume raymarch shader
    //===================================================================================================================================================================================================================
    glsl_program_t crystal_raymarch(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/raymarch_crystal.vs"),
                                    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/raymarch_crystal.fs"));

    crystal_raymarch.enable();

    uniform_t uni_cm_pv_matrix = crystal_raymarch["projection_view_matrix"];
    uniform_t uni_cm_camera_ws = crystal_raymarch["camera_ws"];         
    uniform_t uni_cm_light_ws  = crystal_raymarch["light_ws"];

    crystal_raymarch["scale"] = 0.5f;                                                             
    crystal_raymarch["tb_tex"] = 0;
    crystal_raymarch["value_tex"] = 1;

    //===================================================================================================================================================================================================================
    // create dodecahecron buffer
    //===================================================================================================================================================================================================================
    polyhedron dodecahedron;
    dodecahedron.regular_pnt2_vao(20, 12, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);

    glActiveTexture(GL_TEXTURE0);
    GLuint tb_tex_id = image::png::texture2d("../../../resources/tex2d/marble.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    
    glActiveTexture(GL_TEXTURE1);
    GLuint noise_tex = glsl_noise::randomRGBA_shift_tex256x256(glm::ivec2(37, 17));
    //===================================================================================================================================================================================================================
    // light variables
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    //===================================================================================================================================================================================================================
    // Loading DDS cubemap texture
    //===================================================================================================================================================================================================================
    image::dds::image_t image;
    GLuint tex = image::dds::vglLoadTexture("../../../resources/cubemap/cube.dds", &image);
    image::dds::vglUnloadImage(&image);

    //===================================================================================================================================================================================================================
    // main program loop : just clear the buffer in a loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = window.frame_ts;
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 camera_ws = window.camera.position();
        glm::vec3 light_ws = 7.0f * glm::vec3(glm::cos(time), 0.0f, glm::sin(time));

        //===============================================================================================================================================================================================================
        // render skybox
        //===============================================================================================================================================================================================================
        skybox_renderer.enable();
        glCullFace(GL_FRONT);
        uni_sbox_pv_matrix = projection_view_matrix;
        glBindVertexArray(cube_vao_id);
        glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_SHORT, 0);
        glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_SHORT, (void *)(8 * sizeof(GLushort)));

        //===============================================================================================================================================================================================================
        // raymarch through polyhedron
        //===============================================================================================================================================================================================================
        crystal_raymarch.enable();

        uni_cm_pv_matrix = projection_view_matrix;
        uni_cm_camera_ws = camera_ws;
        uni_cm_light_ws = light_ws;

        dodecahedron.render();

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}



