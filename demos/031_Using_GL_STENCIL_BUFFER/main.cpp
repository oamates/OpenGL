//========================================================================================================================================================================================================================
// DEMO 031: GL_STENCIL_BUFFER
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
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "plato.hpp"
#include "polyhedron.hpp"


struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    bool mouse_look = true;
    bool pick_requested = false;
    int pick_x, pick_y;
    unsigned char picked_id = 0;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods)
    {
        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
        {
            if (mouse_look)
            {
                mouse_look = false;
                enable_cursor();
            }
            else
            {
                mouse_look = true;
                disable_cursor();
            }
        }
        else
        {
            if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
            else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
            else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
            else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);
        }
    }
    
    void on_mouse_move() override
    {
        if (!mouse_look)
            return;
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
    
    void on_mouse_button(int button, int action, int mods) 
    {
        if (mouse_look)
            return;
        if ((button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_RELEASE) && !mouse_look)
        {
            glm::dvec2 cursor = cursor_position();
            pick_x = cursor.x;
            pick_y = res_y - cursor.y;
            pick_requested = (pick_x >= 0) && (pick_x < res_x) && (pick_y >= 0) && (pick_y < res_y);
        }
    }
};

struct motion3d_t
{
    glm::vec3 shift;
    glm::vec4 rotor;
};

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("GL_STENCIL_BUFFER Picker", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // phong lighting model shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t simple_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_light.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_light.fs"));

    simple_light.enable();

    uniform_t uniform_projection_view_matrix = simple_light["projection_view_matrix"];
    uniform_t uniform_camera_ws              = simple_light["camera_ws"];
    uniform_t uniform_light_ws               = simple_light["light_ws"];
    uniform_t uniform_time                   = simple_light["time"];
    uniform_t uniform_shift                  = simple_light["shift"];
    uniform_t uniform_rotor                  = simple_light["rotor"];
    uniform_t uniform_solid_scale            = simple_light["solid_scale"];

    simple_light["diffuse_texture"] = 0;
    simple_light["normal_texture"] = 1;
    uniform_solid_scale = 0.5f;

    //===================================================================================================================================================================================================================
    // initialize buffers : vertex + tangent frame + texture coordinates 
    // for five regular plato solids and load the diffuse + bump textures for each
    //===================================================================================================================================================================================================================

    polyhedron dodecahedron;
    dodecahedron.regular_pft2_vao(20, 12, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);
 
    glActiveTexture(GL_TEXTURE0);
    GLuint cube_diffuse_texture_id = image::png::texture2d("../../../resources/plato_tex2d/pentagon.png");
    glActiveTexture(GL_TEXTURE1);
    GLuint cube_normal_texture_id = image::png::texture2d("../../../resources/plato_tex2d/pentagon_bump.png");

    const int SIZE_X = 3, SIZE_Y = 5, SIZE_Z = 17;
    const int TOTAL_SIZE = SIZE_X * SIZE_Y * SIZE_Z;

    glm::vec3 midpoint = glm::vec3(0.5f * SIZE_X - 0.5f, 0.5f * SIZE_Y - 0.5f, 0.5f * SIZE_Z - 0.5f);

    motion3d_t data[TOTAL_SIZE];

    const float cell_size = 1.375f;

    int index = 0;
    for (int p = 0; p < SIZE_X; ++p) for (int q = 0; q < SIZE_Y; ++q) for (int r = 0; r < SIZE_Z; ++r)
    {
        data[index].shift = cell_size * (glm::vec3(float(p), float(q), float(r)) - midpoint);
        data[index].rotor = glm::vec4(glm::sphericalRand(1.0f), glm::gaussRand(0.0f, 1.0f));
        index++;
    }

    //===================================================================================================================================================================================================================
    // Global GL settings :
    // DEPTH and STENCIL test enabled, CULL_FACE enabled
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    const float light_radius = 75.0f;

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        glm::vec3 light_ws = glm::vec3(0.0f, light_radius * cos(0.5f * time), light_radius * sin(0.5f * time));
        glm::vec3 camera_ws = window.camera.position();
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        uniform_projection_view_matrix = projection_view_matrix;
        uniform_camera_ws = camera_ws;
        uniform_light_ws = light_ws;
        uniform_time = time;

        for (int i = 0; i < TOTAL_SIZE; i++)
        {
            if (window.picked_id == i + 1)
                uniform_solid_scale = 0.75f;                
            uniform_shift = data[i].shift;
            uniform_rotor = data[i].rotor;
            glStencilFunc(GL_ALWAYS, i + 1, -1);
            dodecahedron.render();
            if (window.picked_id == i + 1)
                uniform_solid_scale = 0.5f;
        }

        if (window.pick_requested)
        {
            glReadPixels(window.pick_x, window.pick_y, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &window.picked_id);
            debug_msg("ID read from stencil buffer : %u", (unsigned int) window.picked_id);
            window.pick_requested = false;
        }

        window.end_frame();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}