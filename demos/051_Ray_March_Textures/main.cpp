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

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
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
// Setup 5 :: framebuffer with a single depth
//=======================================================================================================================================================================================================================
struct fbo_depth_t
{
    GLsizei res_x, res_y;

    GLuint fbo_id;
    GLuint texture_id;
    
    fbo_depth_t(GLsizei res_x, GLsizei res_y, GLenum internal_format, GLint wrap_mode, GLenum texture_unit)
        : res_x(res_x), res_y(res_y)
    {
        debug_msg("Creating FBO with one %dx%d depth attachment. Internal format :: %u", res_x, res_y, internal_format);

        glGenFramebuffers(1, &fbo_id);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

        glActiveTexture(texture_unit);
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
    
        glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, res_x, res_y);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_id, 0);

        check_status();
    }
    
    void bind()
        { glBindFramebuffer(GL_FRAMEBUFFER, fbo_id); }
    
    ~fbo_depth_t() 
    {
        glDeleteTextures(1, &texture_id);
        glDeleteFramebuffers(1, &fbo_id);
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

    //===================================================================================================================================================================================================================
    // crystal raymarch shader
    //===================================================================================================================================================================================================================
    glsl_program_t crystal_raymarch(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/raymarch_crystal.vs"),
                                    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/raymarch_crystal.fs"));

    crystal_raymarch.enable();

    uniform_t uni_cm_pv_matrix     = crystal_raymarch["projection_view_matrix"];
    uniform_t uni_cm_camera_matrix = crystal_raymarch["camera_matrix"];
    uniform_t uni_cm_camera_ws     = crystal_raymarch["camera_ws"];         
    uniform_t uni_cm_light_ws      = crystal_raymarch["light_ws"];

    crystal_raymarch["backface_depth_tex"] = 0;
    crystal_raymarch["tb_tex"] = 1;

    crystal_raymarch["inv_resolution"] = glm::vec2(1.0f / res_x, 1.0f / res_y);
    crystal_raymarch["focal_scale"] = glm::vec2(1.0f / window.camera.projection_matrix[0][0], 1.0f / window.camera.projection_matrix[1][1]);

    //===================================================================================================================================================================================================================
    // create dodecahecron buffer
    //===================================================================================================================================================================================================================
    polyhedron dodecahedron;
    dodecahedron.regular_pnt2_vao(20, 12, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);

    //===================================================================================================================================================================================================================
    // Create FBO with depth texture attached for back faces rendering
    //===================================================================================================================================================================================================================
    fbo_depth_t backface_fbo(res_x, res_y, GL_DEPTH_COMPONENT32, GL_CLAMP_TO_EDGE, GL_TEXTURE0);

    //===================================================================================================================================================================================================================
    // light variables
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE1);
    GLuint tb_tex_id = image::png::texture2d("../../../resources/tex2d/sapphire.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    //===================================================================================================================================================================================================================
    // main program loop : just clear the buffer in a loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        glm::mat4& view_matrix = window.camera.view_matrix;

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::mat3 camera_matrix = glm::inverse(glm::mat3(view_matrix));
        glm::vec3 camera_ws = glm::vec3(view_matrix[3]);
        glm::vec3 light_ws = 7.0f * glm::vec3(glm::cos(time), 0.0f, glm::sin(time));

        //===============================================================================================================================================================================================================
        // render polyhedron back faces to depth texture
        //===============================================================================================================================================================================================================
        backface_fbo.bind();

        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);

        geometry_pass.enable();
        uni_gp_pv_matrix = projection_view_matrix;
        dodecahedron.render();

        //===============================================================================================================================================================================================================
        // use depth texture to raymarch through polyhedron
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_BACK);

        crystal_raymarch.enable();

        uni_cm_pv_matrix = projection_view_matrix;
        uni_cm_camera_matrix = camera_matrix;
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



