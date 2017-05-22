//========================================================================================================================================================================================================================
// DEMO 068: Ambient Occlusion Effect Shader
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "vao.hpp"

std::default_random_engine generator;
std::normal_distribution<float> gaussRand(0.0, 1.0);

const int res_x = 1280;
const int res_y = 1024;

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


void check_status()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (GL_FRAMEBUFFER_COMPLETE == status)
    {
        debug_msg("GL_FRAMEBUFFER is COMPLETE.");
        return;
    }

    const char * msg;   
    switch (status)
    {
        case GL_FRAMEBUFFER_UNDEFINED:                     msg = "GL_FRAMEBUFFER_UNDEFINED."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         msg = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: msg = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        msg = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        msg = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER."; break;
        case GL_FRAMEBUFFER_UNSUPPORTED:                   msg = "GL_FRAMEBUFFER_UNSUPPORTED."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        msg = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      msg = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS."; break;
      default:
        msg = "Unknown Framebuffer error.";
    }

    exit_msg("FBO incomplete : %s", msg);
}

//=======================================================================================================================================================================================================================
// Setup 1 :: renderbuffer object + one color attachment
//=======================================================================================================================================================================================================================

struct fbo_rb_color_t
{
    GLuint fbo_id;
    GLuint rbo_id;
    GLuint texture_id;
    
    fbo_rb_color_t(GLsizei res_x, GLsizei res_y, GLenum internal_format, GLint wrap_mode, GLenum texture_unit)
    {
        debug_msg("Creating color FBO with renderbuffer and one %dx%d color attachment. Internal format :: %u", res_x, res_y, internal_format);

        glGenFramebuffers(1, &fbo_id);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

        glGenRenderbuffers(1, &rbo_id);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, res_x, res_y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_id);
        
        glActiveTexture(texture_unit);
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, res_x, res_y);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_id, 0);

        glDrawBuffer(GL_COLOR_ATTACHMENT0);    
        check_status();
    }
    
    void bind()
        { glBindFramebuffer(GL_FRAMEBUFFER, fbo_id); }
    
    ~fbo_rb_color_t() 
    {
        glDeleteTextures(1, &texture_id);
        glDeleteRenderbuffers(1, &rbo_id);
        glDeleteFramebuffers(1, &fbo_id);
    }

};

//=======================================================================================================================================================================================================================
// Setup 2 :: renderbuffer object + one color attachment
//=======================================================================================================================================================================================================================

struct fbo_color_t
{
    GLuint fbo_id;
    GLuint texture_id;
    
    fbo_color_t(GLsizei res_x, GLsizei res_y, GLenum internal_format, GLint wrap_mode, GLenum texture_unit)
    {
        debug_msg("Creating color FBO with one %dx%d color attachment. Internal format :: %u", res_x, res_y, internal_format);

        glGenFramebuffers(1, &fbo_id);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
        
        glActiveTexture(texture_unit);
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, res_x, res_y);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_id, 0);

        glDrawBuffer(GL_COLOR_ATTACHMENT0);    
        check_status();
    }
    
    void bind()
        { glBindFramebuffer(GL_FRAMEBUFFER, fbo_id); }
    
    ~fbo_color_t() 
    {
        glDeleteTextures(1, &texture_id);
        glDeleteFramebuffers(1, &fbo_id);
    }

};

//=======================================================================================================================================================================================================================
// Setup 2 :: renderbuffer object + one color attachment
//=======================================================================================================================================================================================================================
GLuint create_noise_texture(GLenum texture_unit, int res_x, int res_y)
{
    GLuint noise_texture_id;
    glActiveTexture(texture_unit);
    glGenTextures(1, &noise_texture_id);
    glBindTexture(GL_TEXTURE_2D, noise_texture_id);

    glm::vec3* noise_vec3 = (glm::vec3*) malloc(res_x * res_y * sizeof(glm::vec3));

    int idx = 0;
    for(int y = 0; y < res_y; ++y)
        for(int x = 0; x < res_x; ++x)
            noise_vec3[idx++] = glm::normalize(glm::vec3(gaussRand(generator), gaussRand(generator), gaussRand(generator)));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, res_x, res_y, 0, GL_RGB, GL_FLOAT, noise_vec3);

    free(noise_vec3);
    return noise_texture_id;
}


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

    demo_window_t window("AO Effect Shader", 4, 3, 3, res_x, res_y, false);

    //===================================================================================================================================================================================================================
    // generate SSAO sample kernel points
    //===================================================================================================================================================================================================================
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;

    glm::vec3 ssao_kernel[32];

    for (GLuint i = 0; i < 32; ++i)
    {
        glm::vec3 v = glm::vec3(gaussRand(generator), gaussRand(generator), gaussRand(generator));
        v = glm::normalize(v);
        v *= gaussRand(generator);
        ssao_kernel[i] = v;
    }

    //===================================================================================================================================================================================================================
    // matrices
    //===================================================================================================================================================================================================================
    glm::mat4& projection_matrix = window.camera.projection_matrix;
    glm::vec2 focal_scale = glm::vec2(1.0f / projection_matrix[0][0], 1.0f / projection_matrix[1][1]);
    glm::mat4 model_matrix = glm::mat4(1.0f);

    //===================================================================================================================================================================================================================
    // compile shaders and load static uniforms
    //===================================================================================================================================================================================================================
    glsl_program_t geometry_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao_geometry.vs"), 
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_geometry.fs"));
    geometry_pass.enable();

    uniform_t uni_gp_vm_matrix        = geometry_pass["view_model_matrix"];
    uniform_t uni_gp_normal_vm_matrix = geometry_pass["normal_vm_matrix"];
    geometry_pass["projection_matrix"] = window.camera.projection_matrix;


    glsl_program_t ssao_compute(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao_compute.vs"), 
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_compute.fs"));
    ssao_compute.enable();
    ssao_compute["noise_tex"] = 0;
    ssao_compute["normal_cs_tex"] = 1;
    ssao_compute["projection_matrix"] = window.camera.projection_matrix;
    ssao_compute["samples"] = ssao_kernel;
    ssao_compute["focal_scale"] = focal_scale;
    ssao_compute["radius"] = 1.5f;
    ssao_compute["bias"] = 0.25f;


    glsl_program_t ssao_blur(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"), 
                             glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_blur.fs"));
    ssao_blur.enable();
    ssao_blur["ssao_input"] = 2;
    ssao_blur["texel_size"] = glm::vec2(1.0f / res_x, 1.0f / res_y);


    glsl_program_t lighting_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao_lighting.vs"), 
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_lighting.fs"));
    lighting_pass.enable();

    lighting_pass["ssao_blurred_tex"] = 3;
    lighting_pass["tb_tex"]           = 4;
    lighting_pass["model_matrix"]     = model_matrix;

    uniform_t uni_lp_pv_matrix        = lighting_pass["projection_view_matrix"];
    uniform_t uni_lp_normal_matrix    = lighting_pass["normal_matrix"];
    uniform_t uni_lp_camera_ws        = lighting_pass["camera_ws"];
    uniform_t uni_lp_light_ws         = lighting_pass["light_ws"];
    uniform_t uni_lp_draw_mode        = lighting_pass["draw_mode"];

    //===================================================================================================================================================================================================================
    // load model
    //===================================================================================================================================================================================================================
    vao_t model;
    model.init("../../../resources/models/vao/demon.vao");
    debug_msg("VAO Loaded :: \n\tvertex_count = %d. \n\tvertex_layout = %d. \n\tindex_type = %d. \n\tprimitive_mode = %d. \n\tindex_count = %d\n\n\n", 
              model.vbo.size, model.vbo.layout, model.ibo.type, model.ibo.mode, model.ibo.size);

    //===================================================================================================================================================================================================================
    // framebuffer object and textures for geometry rendering step
    //===================================================================================================================================================================================================================
    GLuint noise_tex_id = create_noise_texture(GL_TEXTURE0, res_x, res_y);

    fbo_rb_color_t geometry_fbo(res_x, res_y, GL_RGBA32F, GL_CLAMP_TO_EDGE, GL_TEXTURE1);
    fbo_color_t ssao_compute_fbo(res_x, res_y, GL_R32F, GL_CLAMP_TO_EDGE, GL_TEXTURE2);
    fbo_color_t ssao_blur_fbo(res_x, res_y, GL_R32F, GL_CLAMP_TO_EDGE, GL_TEXTURE3);

    //===================================================================================================================================================================================================================
    // load 2D texture for trilinear blending in lighting shader
    //================================  ===================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE4);
    GLuint tb_tex_id = image::png::texture2d("../../../resources/tex2d/marble.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
        glm::mat4 view_model_matrix = window.camera.view_matrix * model_matrix;
        glm::mat3 normal_vm_matrix = glm::inverse(glm::mat3(view_model_matrix));
        glm::mat3 normal_matrix = glm::inverse(glm::mat3(model_matrix));

        static char title[32];
        sprintf(title, "FPS: %2.1f", window.fps());
        window.set_title(title);

        //===============================================================================================================================================================================================================
        // 1. Geometry Pass: render scene's geometry / color data into gbuffer
        //===============================================================================================================================================================================================================
        geometry_fbo.bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        geometry_pass.enable();
        
        uni_gp_vm_matrix        = view_model_matrix;
        uni_gp_normal_vm_matrix = normal_vm_matrix;

        model.render();

        //===============================================================================================================================================================================================================
        // 2. Create SSAO texture
        //===============================================================================================================================================================================================================
        glBindVertexArray(vao_id);
        glDisable(GL_DEPTH_TEST);

        ssao_compute_fbo.bind();
        ssao_compute.enable();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // 3. Blur SSAO texture to remove noise
        //===============================================================================================================================================================================================================
        ssao_blur_fbo.bind();
        ssao_blur.enable();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // 4. Lighting Pass :: deferred Blinn-Phong lighting with added screen-space ambient occlusion
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lighting_pass.enable();

        uni_lp_pv_matrix = projection_view_matrix;
        uni_lp_normal_matrix = normal_matrix;
        uni_lp_camera_ws = camera_ws;
        uni_lp_light_ws = light_ws;
        uni_lp_draw_mode = window.draw_mode;

        model.render();

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}