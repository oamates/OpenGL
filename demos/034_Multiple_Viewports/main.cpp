//========================================================================================================================================================================================================================
// DEMO 034 : Multiple Viewports
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
#include "surface.hpp"
#include "plato.hpp"
#include "image.hpp"

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

vertex_pft2_t torus_func(const glm::vec2& uv)
{
    vertex_pft2_t vertex;

    float cos_2piv = glm::cos(constants::two_pi * uv.x);
    float sin_2piv = glm::sin(constants::two_pi * uv.x);
    float cos_2piu = glm::cos(constants::two_pi * uv.y);
    float sin_2piu = glm::sin(constants::two_pi * uv.y);

    float R = 2.7f;
    float r = 0.97f;

    vertex.position = glm::vec3((R + r * cos_2piu) * cos_2piv, (R + r * cos_2piu) * sin_2piv, r * sin_2piu);

    vertex.tangent_x = glm::vec3(-sin_2piv, cos_2piv, 0.0f);
    vertex.tangent_y = glm::vec3(-sin_2piu * cos_2piv, -sin_2piu * sin_2piv, cos_2piu);
    vertex.normal = glm::vec3(cos_2piu * cos_2piv, cos_2piu * sin_2piv, sin_2piu);
    vertex.uv = uv;

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
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Multiple Viewports", 8, 4, 1, 1920, 1080);

    //===================================================================================================================================================================================================================
    // create programs : one for particle compute, the other for render
    //===================================================================================================================================================================================================================
    glsl_program_t base_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/base_light.vs"),
                              glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/base_light.gs"),
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/base_light.fs"));

    base_light.enable();
    uniform_t uniform_projection_matrix = base_light["projection_matrix"];
    uniform_t uniform_view_matrix       = base_light["view_matrix"];
    uniform_t uniform_camera_ws         = base_light["camera_ws"];
    uniform_t uniform_light_ws          = base_light["light_ws"];

    base_light["diffuse_tex"] = 0;
    base_light["normal_tex"] = 1;


    //===================================================================================================================================================================================================================
    // generate random texture
    //===================================================================================================================================================================================================================
    surface_t torus;
    torus.generate_vao<vertex_pft2_t>(torus_func, 37, 67);

    glActiveTexture(GL_TEXTURE0);
    GLuint torus_diffuse_texture_id = image::png::texture2d("../../../resources/tex2d/torus.png");

    glActiveTexture(GL_TEXTURE1);
    GLuint torus_bump_texture_id = image::png::texture2d("../../../resources/tex2d/torus_bump.png");

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * PRIMITIVE_RESTART as surface uses this
    // * 4 different Viewports each having different view matrix
    //===================================================================================================================================================================================================================
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(torus.vao.ibo.pri);
    glClearColor(0.01f, 0.00f, 0.05f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    const float half_x = 0.5f * window.res_x;
    const float half_y = 0.5f * window.res_y;

    const GLfloat viewports[] = 
    {
          0.0f,   0.0f, half_x, half_y,
        half_x,   0.0f, half_x, half_y,
          0.0f, half_y, half_x, half_y,
        half_x, half_y, half_x, half_y,
    };

    glViewportArrayv(0, 4, viewports);

    uniform_projection_matrix = window.camera.projection_matrix;
    const float light_radius = 6.0;

    glm::mat4 vm[4] = 
    {
        glm::lookAt(
            glm::vec3(0.0f),
            plato::tetrahedron::vertices[0],
            glm::vec3(0.0f, 1.0f, 0.0f)
        ),
        glm::lookAt(
            glm::vec3(0.0f),
            plato::tetrahedron::vertices[1],
            glm::vec3(0.0f, 1.0f, 0.0f)
        ),
        glm::lookAt(
            glm::vec3(0.0f),
            plato::tetrahedron::vertices[2],
            glm::vec3(0.0f, 1.0f, 0.0f)
        ),
        glm::lookAt(
            glm::vec3(0.0f),
            plato::tetrahedron::vertices[3],
            glm::vec3(0.0f, 1.0f, 0.0f)
        )        
    };

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        glm::vec3 light_ws = light_radius * glm::vec3(0.0f, glm::cos(0.5f * time), glm::sin(0.5f * time));

        glm::mat4 view_matrix[4] = 
        {
            window.camera.view_matrix * vm[0], 
            window.camera.view_matrix * vm[1], 
            window.camera.view_matrix * vm[2], 
            window.camera.view_matrix * vm[3]
        };

        glm::vec3 camera_ws[4] =
        {
            -glm::inverse(glm::mat3(view_matrix[0])) * glm::vec3(view_matrix[0][3]),
            -glm::inverse(glm::mat3(view_matrix[1])) * glm::vec3(view_matrix[1][3]),
            -glm::inverse(glm::mat3(view_matrix[2])) * glm::vec3(view_matrix[2][3]),
            -glm::inverse(glm::mat3(view_matrix[3])) * glm::vec3(view_matrix[3][3])
        };

        uniform_view_matrix = view_matrix;
        uniform_camera_ws = camera_ws;
        uniform_light_ws = light_ws;

        torus.render();

        window.end_frame();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}