//========================================================================================================================================================================================================================
// DEMO 045 : Edge Flipper
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
#include "shader.hpp"
#include "camera.hpp"
#include "vertex.hpp"
#include "momenta.hpp"
#include "vao.hpp"
#include "he_manifold.hpp"
#include "hqs_model.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;
    GLenum mode = GL_FILL;
    bool render_original = true;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.001f);
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

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
        {
            mode = (mode == GL_FILL) ? GL_LINE : GL_FILL;
            glPolygonMode(GL_FRONT_AND_BACK, mode);
        }

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE))
        {
            render_original = !render_original;
            debug_msg("Rendering mode = %s", render_original ? "original" : "flipped");
        }
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }

    void on_scroll(double xoffset, double yoffset) override
    {
        const float max_speed = 8.0;
        const float min_speed = 0.03125;

        float factor = exp(0.125 * yoffset);
        float speed = factor * camera.linear_speed;

        if ((speed <= max_speed) && (speed >= min_speed))
            camera.linear_speed = speed;
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

    demo_window_t window("Edge Flipper", 4, 3, 3, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // load demon model
    //===================================================================================================================================================================================================================
    glsl_program_t model_render(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/model_render.vs"),
                                glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/model_render.gs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/model_render.fs"));

    model_render.enable();
    uniform_t uni_pv_matrix = model_render["projection_view_matrix"];
    uniform_t uni_camera_ws = model_render["camera_ws"];
    uniform_t uni_light_ws  = model_render["light_ws"];

    //===================================================================================================================================================================================================================
    // load model and build it edge-face structure
    //===================================================================================================================================================================================================================

    hqs_model_t model("../../../resources/manifolds/demon.obj");
    model.normalize(1.0);

    he_manifold_t<GLuint> manifold(model.faces.data(), model.F, model.positions.data(), model.V);
    model.normals.resize(model.V);
    manifold.normals = model.normals.data();
    manifold.calculate_angle_weighted_normals();

    vao_t model_ori_vao = model.create_vao();

/*
    double threshold = glm::cos(constants::pi_d * (1.0 - 1.0 / 16.0)); // angles greater than pi * (1 - 1/64) ~ 177.1875 degrees

    for(int i = 0; i < 32; ++i)
    {
        manifold.find_folded_edges(threshold);
        manifold.flip_folded_edges(threshold);
        debug_msg("\n\n\n");
    }

    manifold.find_folded_edges(threshold);
    debug_msg("\n\n\n");
    manifold.validate();
*/

/*
    debug_msg("\n\n\n");
    double threshold = glm::cos(constants::pi_d * (1.0 - 1.0 / 32.0));

    for(int i = 0; i < 32; ++i)
    {
        manifold.find_degenerate_faces(threshold);
        manifold.flip_degenerate_faces(threshold);
        debug_msg("\n\n\n");
    }

    manifold.find_degenerate_faces(threshold);
    manifold.validate();
*/
    
    vao_t model_flp_vao = model.create_vao();
    manifold.export_obj("demon.obj", false);
    manifold.export_vao("demon.vao", true);

    manifold.calculate_angle_weighted_normals();
    manifold.test_normals();

    glEnable(GL_DEPTH_TEST);
                                                         
    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        float time = window.frame_ts;
        const float light_radius = 1.707f;
        glm::vec3 light_ws = glm::vec3(light_radius * cos(0.5f * time), 2.0f, light_radius * sin(0.5f * time));
        glm::vec3 camera_ws = window.camera.position();

        uni_pv_matrix = projection_view_matrix;
        uni_camera_ws = camera_ws;  
        uni_light_ws = light_ws;

        if (window.render_original)
            model_ori_vao.render();
        else
            model_flp_vao.render();

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================

    glfw::terminate();
    return 0;
}
