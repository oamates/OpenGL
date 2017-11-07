//========================================================================================================================================================================================================================
// DEMO 042: VAO loader
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "vao.hpp"
#include "vertex.hpp"

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

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("VAO Loader", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Load standard Blinn-Phong shader : no texture coordinates
    //===================================================================================================================================================================================================================
    glsl_program_t blinn_phong(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/blinn-phong.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blinn-phong.fs"));

    blinn_phong.enable();

    uniform_t uniform_projection_view_matrix = blinn_phong["projection_view_matrix"];
    uniform_t uniform_camera_ws              = blinn_phong["camera_ws"];
    uniform_t uniform_light_ws               = blinn_phong["light_ws"];
    uniform_t uniform_Ka                     = blinn_phong["Ka"];
    uniform_t uniform_Kd                     = blinn_phong["Kd"];
    uniform_t uniform_Ks                     = blinn_phong["Ks"];
    uniform_t uniform_Ns                     = blinn_phong["Ns"];

    //===================================================================================================================================================================================================================
    // Global OpenGL state : since there are no depth writes, depth buffer needs not be cleared
    //===================================================================================================================================================================================================================
    glClearColor(0.015f, 0.005f, 0.045f, 1.0f);
    glEnable(GL_DEPTH_TEST);   
    glDisable(GL_CULL_FACE);
    GLuint diffuse_texture_id = image::png::texture2d("../../../resources/tex2d/rock.png");

    const char* vao_list[] = 
    {
        "../../../resources/models/vao/pnt2/chalet/chalet.vao",
//        "../../../resources/models/vao/azog.vao",
//        "../../../resources/models/vao/bust.vao",
//        "../../../resources/models/vao/chubby_girl.vao",
//        "../../../resources/models/vao/demon.vao",
//        "../../../resources/models/vao/dragon.vao",
//        "../../../resources/models/vao/female_01.vao",
//        "../../../resources/models/vao/female_02.vao",
//        "../../../resources/models/vao/female_03.vao",
//        "../../../resources/models/vao/king_kong.vao",
//        "../../../resources/models/vao/predator.vao",
//        "../../../resources/models/vao/skull.vao",
//          "../../../resources/models/vao/trefoil.vao"
    };

    const int MODEL_COUNT = sizeof(vao_list) / sizeof(const char *);

    const float light_radius = 75.0f;

    vao_t models[MODEL_COUNT];
    for (int i = 0; i < MODEL_COUNT; ++i)
    {
        vao_t& model = models[i];
        debug_msg("Loading %s ... \n", vao_list[i]);
        model.init(vao_list[i]);
        debug_msg("VAO Loaded :: \n\tvertex_count = %d. \n\tvertex_layout = %d. \n\tindex_type = %d. \n\tprimitive_mode = %d. \n\tindex_count = %d\n\n\n", 
                  model.vbo.size, model.vbo.layout, model.ibo.type, model.ibo.mode, model.ibo.size);

    }
    debug_msg("Done ... \nGL_UNSIGNED_INT = %d.\nGL_TRIANGLES = %d", GL_UNSIGNED_INT, GL_TRIANGLES);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 light_ws = glm::vec3(200.0f    + light_radius * cos(time), 350.0f, light_radius * sin(time));
        glm::vec3 camera_ws = window.camera.position();

        uniform_projection_view_matrix = projection_view_matrix;

        uniform_light_ws = light_ws;
        uniform_camera_ws = camera_ws;

        glm::vec3 Ka = glm::vec3(0.17f);
        glm::vec3 Kd = glm::vec3(0.50f);
        glm::vec3 Ks = glm::vec3(0.33f);
        float Ns = 20.0f;
        
        uniform_Ka = Ka;
        uniform_Kd = Kd;
        uniform_Ks = Ks;
        uniform_Ns = Ns;

        for (int i = 0; i < MODEL_COUNT; ++i) models[i].render();

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================    return 0;
    glfw::terminate();
    return 0;
}                               