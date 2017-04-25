//========================================================================================================================================================================================================================
// DEMO 045 : Compute Shader SSAO
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "model.hpp"
#include "shadowmap.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
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

const unsigned int TEXTURE_SIZE = 1024;

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

    demo_window_t window("Compute Shader SSAO", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Loading models
    //===================================================================================================================================================================================================================
    model demon;
    demon.load_vi("res/demon.obj");

    model pedestal;
    pedestal.load_vnti("res/pedestal/pedestal1.obj");

    // ==================================================================================================================================================================================================================
    // Set up camera and projection matrix
    // ==================================================================================================================================================================================================================
    float fov = constants::two_pi / 6.0f;
    float aspect_ratio = window.aspect();
    float znear = 1.0f;
    glm::vec2 scale_xy = -glm::vec2(glm::tan(fov / 2.0f));
    scale_xy.x *= aspect_ratio;

    //===================================================================================================================================================================================================================
    // shader program for geometry pass
    //===================================================================================================================================================================================================================
    glsl_program_t geometry(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/geometry_pass.vs"),
                            glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/geometry_pass.fs"));
    geometry.enable();
    uniform_t geometry_pass_mvp_matrix = geometry["mvp_matrix"];

    //===================================================================================================================================================================================================================
    // shader program for screen-space ambient occlusion pass
    //===================================================================================================================================================================================================================
    glsl_program_t ssao(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao.vs"),
                        glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao.fs"));
    ssao.enable();

    ssao["scale_xy"] = scale_xy;
    ssao["projection_matrix"] = window.camera.projection_matrix;

    const unsigned int MAX_KERNEL_SIZE = 512;
    glm::vec3 rnd[MAX_KERNEL_SIZE];
    GLint ssao_spherical_rand = ssao["spherical_rand"];
    for (int i = 0; i < 512; ++i)
    {
        rnd[i] = glm::sphericalRand(1.0f);
        debug_msg("spherical_rand[%d] = %s", i, glm::to_string(rnd[i]).c_str());
    };
    glUniform3fv(ssao_spherical_rand, 512, glm::value_ptr(rnd[0]));

    


    //===================================================================================================================================================================================================================
    // shader program for ssao image blur 
    //===================================================================================================================================================================================================================
    glsl_program_t blur(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/blur.vs"),
                        glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blur.fs"));
    blur.enable();

    //===================================================================================================================================================================================================================
    // lighting shader program 
    //===================================================================================================================================================================================================================
    glsl_program_t simple_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/simple_light.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/simple_light.fs"));

    simple_light.enable();

    uniform_t simple_light_view_matrix  = simple_light["view_matrix"];
    uniform_t simple_light_model_matrix = simple_light["model_matrix"];
    simple_light["projection_matrix"] = window.camera.projection_matrix;
    glm::mat4 demon_model_matrix = glm::translate(glm::vec3(0.0f, 19.0f, 0.0f)) * glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    glm::mat4 pedestal_model_matrix = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(4.0f, 4.0f, 4.0f));

    depth_map zbuffer(window.res_x, window.res_y);
    color_map ssao_buffer(window.res_x, window.res_y);
    color_map blur_buffer(window.res_x, window.res_y);

    //===================================================================================================================================================================================================================
    // generate quad VAO
    //===================================================================================================================================================================================================================
    GLuint quad_vao_id, quad_vbo_id;
    
    glm::vec2 quad_data [] = 
    {
        glm::vec2(-1.0f, -1.0f),
        glm::vec2( 1.0f, -1.0f),
        glm::vec2( 1.0f,  1.0f),
        glm::vec2(-1.0f,  1.0f)
    };

    glGenVertexArrays(1, &quad_vao_id);
    glBindVertexArray(quad_vao_id);

    glGenBuffers(1, &quad_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), glm::value_ptr(quad_data[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    debug_msg("projection_matrix = %s", glm::to_string(window.camera.projection_matrix).c_str());

    while(!window.should_close())
    {
        window.new_frame();

        glm::mat4 mvp_matrix;

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);                                                                                   // dark blue background

        //===============================================================================================================================================================================================================
        // geometry pass
        //===============================================================================================================================================================================================================
        geometry.enable();
        zbuffer.bind();
    
        glClear(GL_DEPTH_BUFFER_BIT);   

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        mvp_matrix = projection_view_matrix * demon_model_matrix;
        geometry_pass_mvp_matrix = mvp_matrix;
        demon.render();

        mvp_matrix = projection_view_matrix * pedestal_model_matrix;
        geometry_pass_mvp_matrix = mvp_matrix;
        pedestal.render();

        glBindVertexArray(quad_vao_id);

        //===============================================================================================================================================================================================================
        // ssao pass
        //===============================================================================================================================================================================================================
        ssao.enable();
        ssao_buffer.bind();
        glClear(GL_COLOR_BUFFER_BIT);
        zbuffer.bind_texture(GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        //===============================================================================================================================================================================================================
        // blur pass
        //===============================================================================================================================================================================================================
        blur.enable();
        blur_buffer.bind();
        glClear(GL_COLOR_BUFFER_BIT);
        ssao_buffer.bind_texture(GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        //===============================================================================================================================================================================================================
        // lighting pass
        //===============================================================================================================================================================================================================
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.01f, 0.0f, 0.05f, 0.0f);                                                                                 // dark blue background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        simple_light.enable();
        blur_buffer.bind_texture(GL_TEXTURE0);

        simple_light_view_matrix = window.camera.view_matrix;
        simple_light_model_matrix = demon_model_matrix;
        demon.render();
        simple_light_model_matrix = pedestal_model_matrix;
        pedestal.render();

        window.end_frame();
    }
    
    glfw::terminate();                                                                                                        // close OpenGL window and terminate GLFW
    return 0;
}