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
#include "fbo.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.5f);
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
    // phong lighting model shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t geometry_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/geometry.vs"), 
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/geometry.fs"));
    geometry_pass.enable();
    uniform_t uni_gp_pv_matrix = geometry_pass["projection_view_matrix"];
    uniform_t uni_gp_model_matrix = geometry_pass["model_matrix"];

    //===================================================================================================================================================================================================================
    // crystal raymarch shader
    //===================================================================================================================================================================================================================
    glsl_program_t crystal_raymarch(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/raymarch_crystal.vs"),
                                    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/raymarch_crystal.fs"));

    crystal_raymarch.enable();

    uniform_t uni_cm_pv_matrix     = crystal_raymarch["projection_view_matrix"];
    uniform_t uni_cm_camera_matrix = crystal_raymarch["camera_matrix"];
    uniform_t uni_cm_model_matrix  = crystal_raymarch["model_matrix"];
    uniform_t uni_cm_camera_ws     = crystal_raymarch["camera_ws"];         
    uniform_t uni_cm_light_ws      = crystal_raymarch["light_ws"];

    crystal_raymarch["backface_depth_tex"] = 0;
    crystal_raymarch["tb_tex"] = 1;
    crystal_raymarch["value_tex"] = 2;
    crystal_raymarch["inv_resolution"] = glm::vec2(1.0f / res_x, 1.0f / res_y);
    crystal_raymarch["focal_scale"] = glm::vec2(1.0f / window.camera.projection_matrix[0][0], 1.0f / window.camera.projection_matrix[1][1]);

    //===================================================================================================================================================================================================================
    // standard lighting shader
    //===================================================================================================================================================================================================================
    glsl_program_t lighting_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/lighting.vs"), 
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/lighting.fs"));
    lighting_pass.enable();
    lighting_pass["tb_tex"] = 3;

    uniform_t uni_lp_pvm_matrix       = lighting_pass["pvm_matrix"];
    uniform_t uni_lp_model_matrix     = lighting_pass["model_matrix"];
    uniform_t uni_lp_normal_matrix    = lighting_pass["normal_matrix"];
    uniform_t uni_lp_camera_ws        = lighting_pass["camera_ws"];
    uniform_t uni_lp_light_ws         = lighting_pass["light_ws"];
    uniform_t uni_lp_Ks               = lighting_pass["Ks"];
    uniform_t uni_lp_Ns               = lighting_pass["Ns"];
    uniform_t uni_lp_bf               = lighting_pass["bf"];
    uniform_t uni_lp_tex_scale        = lighting_pass["tex_scale"];

    //===================================================================================================================================================================================================================
    // create dodecahecron buffer
    //===================================================================================================================================================================================================================
    polyhedron dodecahedron;
    dodecahedron.regular_pnt2_vao(20, 12, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);

    //===================================================================================================================================================================================================================
    // Create FBO with depth texture attached for back faces rendering
    //===================================================================================================================================================================================================================
    fbo_depth_t backface_fbo(res_x, res_y, GL_TEXTURE0, GL_DEPTH_COMPONENT32, GL_LINEAR, GL_CLAMP_TO_EDGE);

    //===================================================================================================================================================================================================================
    // load demon model
    //===================================================================================================================================================================================================================
    vao_t demon_model;
    demon_model.init("../../../resources/models/vao/demon.vao");

    glm::mat4 demon_model_matrix = glm::scale(glm::translate(glm::vec3(0.0f, 0.51f, 0.0f)), glm::vec3(0.705f));
    glm::mat3 demon_normal_matrix = glm::mat3(1.0f);
    glm::mat4 crystal_model_matrix = glm::scale(glm::vec3(3.0f));

    //===================================================================================================================================================================================================================
    // light variables
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE1);
    GLuint tb_tex_id = image::png::texture2d("../../../resources/tex2d/crystalline.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    
    glActiveTexture(GL_TEXTURE2);
    GLuint noise_tex = glsl_noise::randomRGBA_shift_tex256x256(glm::ivec2(37, 17));

    glActiveTexture(GL_TEXTURE3);
    GLuint demon_tex_id = image::png::texture2d("../../../resources/tex2d/crystalline.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    //===================================================================================================================================================================================================================
    // main program loop : just clear the buffer in a loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();

        float time = window.frame_ts;
        glm::mat4& view_matrix = window.camera.view_matrix;

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::mat3 camera_matrix = glm::inverse(glm::mat3(view_matrix));
        glm::vec3 camera_ws = -camera_matrix * glm::vec3(view_matrix[3]);
        glm::vec3 light_ws = 7.0f * glm::vec3(glm::cos(time), 0.0f, glm::sin(time));

        //===============================================================================================================================================================================================================
        // render polyhedron back faces to depth texture
        //===============================================================================================================================================================================================================
        backface_fbo.bind(GL_FRAMEBUFFER);

        glClear(GL_DEPTH_BUFFER_BIT);

        geometry_pass.enable();
        uni_gp_pv_matrix = projection_view_matrix;

        glCullFace(GL_FRONT);
        uni_gp_model_matrix = crystal_model_matrix;
        dodecahedron.render();

        glCullFace(GL_BACK);
        uni_gp_model_matrix = demon_model_matrix;
        demon_model.render();

        //===============================================================================================================================================================================================================
        // use depth texture to raymarch through polyhedron
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_BACK);

        lighting_pass.enable();

        glm::mat4 demon_pvm_matrix = projection_view_matrix * demon_model_matrix;
        uni_lp_Ks = 1.02f;
        uni_lp_Ns = 44.0f;
        uni_lp_bf = 0.0127f;
        uni_lp_tex_scale = 1.1275f;
        uni_lp_pvm_matrix = demon_pvm_matrix;
        uni_lp_normal_matrix = demon_normal_matrix;
        uni_lp_model_matrix = demon_model_matrix;
        uni_lp_camera_ws = camera_ws;
        uni_lp_light_ws = light_ws;
        demon_model.render();

        crystal_raymarch.enable();

        uni_cm_pv_matrix = projection_view_matrix;
        uni_cm_model_matrix = crystal_model_matrix;
        uni_cm_camera_matrix = camera_matrix;
        uni_cm_camera_ws = camera_ws;
        uni_cm_light_ws = light_ws;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        dodecahedron.render();
        glDisable(GL_BLEND);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;

}



