//==============================================================================================================================================================================================
// DEMO 029: SSAO Effect Shader
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "vao.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;
    int draw_mode = 0;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
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

        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE))
            draw_mode = (draw_mode + 1) % 5;

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

    demo_window_t window("SSAO Effect Shader", 4, 3, 3, 1280, 1024, false);

    //===================================================================================================================================================================================================================
    // generate SSAO sample kernel points
    //===================================================================================================================================================================================================================
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    glm::vec3 ssao_kernel[64];

    for (GLuint i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        GLfloat scale = GLfloat(i) / 64.0;

        scale = 0.1f + 0.9f * scale * scale;
        sample *= scale;
        ssao_kernel[i] = sample;
    }

    //===================================================================================================================================================================================================================
    // compile shaders and load static uniforms
    //===================================================================================================================================================================================================================
    glsl_program_t geometry_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao_geometry.vs"), 
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_geometry.fs"));
    geometry_pass.enable();
    uniform_t uni_gp_pv_matrix = geometry_pass["projection_view_matrix"];
    uniform_t uni_gp_model_matrix = geometry_pass["model_matrix"];


    glsl_program_t ssao_compute(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"), 
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_compute.fs"));
    ssao_compute.enable();
    ssao_compute["position_tex"] = 0;
    ssao_compute["normal_tex"] = 1;
    ssao_compute["noise_tex"] = 2;
    ssao_compute["samples"] = ssao_kernel;
    uniform_t uni_sc_pv_matrix = ssao_compute["projection_view_matrix"];
    uniform_t uni_sc_camera_ws = ssao_compute["camera_ws"];


    glsl_program_t ssao_blur(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"), 
                             glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_blur.fs"));
    ssao_blur.enable();
    ssao_blur["ssao_input"] = 3;


    glsl_program_t lighting_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"), 
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_lighting.fs"));
    lighting_pass.enable();
    lighting_pass["position_tex"] = 0;
    lighting_pass["normal_tex"] = 1;
    lighting_pass["ssao_blurred_tex"] = 4;
    lighting_pass["tb_tex"] = 5;


    uniform_t uni_lp_camera_ws = lighting_pass["camera_ws"];
    uniform_t uni_lp_light_ws  = lighting_pass["light_ws"];
    uniform_t uni_lp_draw_mode = lighting_pass["draw_mode"];


    // "../../../resources/models/vao/ashtray.vao";
    // "../../../resources/models/vao/azog.vao",
    // "../../../resources/models/vao/bust.vao",
    // "../../../resources/models/vao/chubby_girl.vao",
    // "../../../resources/models/vao/demon.vao",    
    // "../../../resources/models/vao/dragon.vao",   
    // "../../../resources/models/vao/female_01.vao",
    // "../../../resources/models/vao/female_02.vao",
    // "../../../resources/models/vao/female_03.vao",
    // "../../../resources/models/vao/king_kong.vao",
    // "../../../resources/models/vao/predator.vao", 
    // "../../../resources/models/vao/skull.vao",    
    // "../../../resources/models/vao/trefoil.vao"     */


    vao_t model;
    model.init("../../../resources/models/vao/demon.vao");
    debug_msg("VAO Loaded :: \n\tvertex_count = %d. \n\tvertex_layout = %d. \n\tindex_type = %d. \n\tprimitive_mode = %d. \n\tindex_count = %d\n\n\n", 
              model.vbo.size, model.vbo.layout, model.ibo.type, model.ibo.mode, model.ibo.size);

    //===================================================================================================================================================================================================================
    // framebuffer object and textures for geometry rendering step
    //===================================================================================================================================================================================================================
    GLuint geometry_fbo_id;
    glGenFramebuffers(1, &geometry_fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, geometry_fbo_id);

    GLuint position_tex_id;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &position_tex_id);
    glBindTexture(GL_TEXTURE_2D, position_tex_id);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, position_tex_id, 0);

    GLuint normal_tex_id;
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &normal_tex_id);
    glBindTexture(GL_TEXTURE_2D, normal_tex_id);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normal_tex_id, 0);

    GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    GLuint depth_rbo_id;
    glGenRenderbuffers(1, &depth_rbo_id);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo_id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window.res_x, window.res_y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo_id);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        exit_msg("GBuffer Framebuffer not complete!");

    //===================================================================================================================================================================================================================
    // noise texture
    //===================================================================================================================================================================================================================
    glm::vec3 ssaoNoise[16];
    for (GLuint i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
        ssaoNoise[i] = noise;
    }

    glActiveTexture(GL_TEXTURE2);
    GLuint noise_tex_id; 
    glGenTextures(1, &noise_tex_id);
    glBindTexture(GL_TEXTURE_2D, noise_tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    //===================================================================================================================================================================================================================
    // framebuffer object and texture for SSAO compute step
    //===================================================================================================================================================================================================================
    GLuint ssao_compute_fbo_id;
    glGenFramebuffers(1, &ssao_compute_fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_compute_fbo_id);

    GLuint ssao_compute_tex_id;
    glActiveTexture(GL_TEXTURE3);
    glGenTextures(1, &ssao_compute_tex_id);
    glBindTexture(GL_TEXTURE_2D, ssao_compute_tex_id);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ssao_compute_tex_id, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        exit_msg("SSAO Framebuffer not complete!");

    //===================================================================================================================================================================================================================
    // framebuffer object and texture for SSAO blur step
    //===================================================================================================================================================================================================================
    GLuint ssao_blur_fbo_id;
    glGenFramebuffers(1, &ssao_blur_fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo_id);

    GLuint ssao_blur_tex_id;
    glActiveTexture(GL_TEXTURE4);
    glGenTextures(1, &ssao_blur_tex_id);
    glBindTexture(GL_TEXTURE_2D, ssao_blur_tex_id);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ssao_blur_tex_id, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        exit_msg("SSAO Blur Framebuffer not complete!");

    //===================================================================================================================================================================================================================
    // load 2D texture for trilinear blending in lighting shader
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE5);
    GLuint tb_tex_id = image::png::texture2d("../../../resources/tex2d/marble.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glViewport(0, 0, window.res_x, window.res_y);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while (!window.should_close())
    {
        window.new_frame();

        float time = window.frame_ts;
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 camera_ws = window.camera.position();
        glm::vec3 light_ws = glm::vec3(2.0f * glm::cos(time), 4.0f, -2.0f * glm::sin(time));

        static char title[32];
        sprintf(title, "FPS: %2.1f", window.fps());
        window.set_title(title);

        //===============================================================================================================================================================================================================
        // 1. Geometry Pass: render scene's geometry/color data into gbuffer
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, geometry_fbo_id);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        geometry_pass.enable();
        uni_gp_pv_matrix = projection_view_matrix;
        uni_gp_model_matrix = glm::mat4(1.0f);
        model.render();

        //===============================================================================================================================================================================================================
        // 2. Create SSAO texture
        //===============================================================================================================================================================================================================
        glBindVertexArray(vao_id);
        glDisable(GL_DEPTH_TEST);

        glBindFramebuffer(GL_FRAMEBUFFER, ssao_compute_fbo_id);
        ssao_compute.enable();
        uni_sc_pv_matrix = projection_view_matrix;
        uni_sc_camera_ws = camera_ws;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // 3. Blur SSAO texture to remove noise
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo_id);
        ssao_blur.enable();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // 4. Lighting Pass :: deferred Blinn-Phong lighting with added screen-space ambient occlusion
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lighting_pass.enable();

        uni_lp_camera_ws = camera_ws;
        uni_lp_light_ws = light_ws;
        uni_lp_draw_mode = window.draw_mode;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}