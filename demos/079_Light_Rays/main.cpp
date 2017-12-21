//========================================================================================================================================================================================================================
// DEMO 079: Volumetric Light
//========================================================================================================================================================================================================================
#include <random>
#include <cstdlib>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "image.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
          camera(8.0f, 0.5f, glm::lookAt(glm::vec3(4.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
    {
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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

int main(int argc, char *argv[])
{
    const int res_x = 1920;
    const int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("VAO Loader", 8, 3, 3, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // load models
    //===================================================================================================================================================================================================================
    std::ifstream stream;
    OpenResourceFile(stream, "models", "large_fan", ".obj");

    shapes::ObjMesh mesh_loader(stream, shapes::ObjMesh::LoadingOptions(false).Normals());
    shapes::ShapeWrapper meshes(List("Position")("Normal").Get(), mesh_loader);
    GLuint fan_index = mesh_loader.GetMeshIndex("Fan");

    const int shadow_tex_unit = 0;          // TextureUnitSelector shadow_tex_unit
    const int light_tex_unit = 1;           // TextureUnitSelector light_tex_unit

    glm::vec3 light_position = glm::vec3(0.0f, 0.0f, -100.0f);
    glm::mat4 projection = window.camera.projection_matrix;
    glm::mat4 lt_proj = glm::perspective(constants::two_pi / 30.0f, 1.0f, 85.0f, 110.0f);               // matrices
    glm::mat4 light = glm::lookAt(light_position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 light_matrix = lt_proj * light;

    //===================================================================================================================================================================================================================
    // load shaders
    //===================================================================================================================================================================================================================
    glsl_shader_t vert_shader = glsl_shader_t(GL_VERTEX_SHADER, "glsl/vertex.vs");

    glsl_program_t mask_prog(vert_shader, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/mask.fs"));
    mask_prog.enable();

    uniform_t uni_mp_light_position    = mask_prog["LightPosition"];
    uniform_t uni_mp_projection_matrix = mask_prog["ProjectionMatrix"];
    uniform_t uni_mp_camera_matrix     = mask_prog["CameraMatrix"];
    uniform_t uni_mp_model_matrix      = mask_prog["ModelMatrix"];
    uniform_t uni_mp_light_matrix      = mask_prog["LightMatrix"];
    uniform_t uni_mp_shadow_map        = mask_prog["ShadowMap"];

    uni_mp_projection_matrix = projection;
    uni_mp_light_position = light_position;
    uni_mp_light_matrix = light_matrix;
    uni_mp_shadow_map = shadow_tex_unit;

    VertexArray mask_vao(meshes.VAOForProgram(mask_prog));

    glsl_program_t draw_prog(vert_shader, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/render.fs"));
    draw_prog.enable();

    uniform_t uni_dp_light_position    = draw_prog["LightPosition"];
    uniform_t uni_dp_light_screen_pos  = draw_prog["LightScreenPos"];
    uniform_t uni_dp_screen_size       = draw_prog["ScreenSize"];
    uniform_t uni_dp_projection_matrix = draw_prog["ProjectionMatrix"];
    uniform_t uni_dp_camera_matrix     = draw_prog["CameraMatrix"];
    uniform_t uni_dp_model_matrix      = draw_prog["ModelMatrix"];
    uniform_t uni_dp_light_matrix      = draw_prog["LightMatrix"];
    uniform_t uni_dp_shadow_map        = draw_prog["ShadowMap"];
    uniform_t uni_dp_light_map         = draw_prog["LightMap"];

    uni_dp_projection_matrix = projection;
    uni_dp_light_position = light_position;
    uni_dp_screen_size = glm::vec2(res_x, res_y);
    uni_dp_light_matrix = light_matrix;
    uni_dp_shadow_map = shadow_tex_unit;
    uni_dp_light_map = light_tex_unit;

    VertexArray draw_vao(meshes.VAOForProgram(draw_prog))

    GLuint light_rbo;                       // Renderbuffer light_rbo;
    glGenRenderbuffers(1, &light_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, light_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, light_rbo);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);

    GLuint size = 512;

    glActiveTexture(GL_TEXTURE0 + shadow_tex_unit);                                                     // setup the texture
    GLuint shadow_map;                                                                                  // Texture shadow_map;
    glGenTextures(1, &shadow_map);
    glBindTexture(GL_TEXTURE_2D, shadow_map);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, size, size);

    glsl_program_t shadow_prog(vert_shader, glsl_shader_t(GL_FRAGMENT_SHADER, "shadow.fs"));            // create shadow program
    shadow_prog.enable();

    uniform_t uni_sp_projection_matrix = shadow_prog["ProjectionMatrix"];
    uniform_t uni_sp_camera_matrix     = shadow_prog["CameraMatrix"];
    uniform_t uni_sp_model_matrix      = shadow_prog["ModelMatrix"];

    uni_sp_projection_matrix = lt_proj;                                                                 // setup the matrices
    uni_sp_camera_matrix = light;
    uni_sp_model_matrix = glm::mat4(1.0f);

    VertexArray vao = meshes.VAOForProgram(shadow_prog);                                                // VAO for the meshes in shadow program
    vao.Bind();

    GLuint fbo;                                                                                         // Framebuffer fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_map, 0);

    GLuint rbo;                                                                                         // Renderbuffer rbo;
    glGenRenderbuffers(1, rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, size, size);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    glViewport(0, 0, size, size);                                                                       // setup and clear the viewport
    glClear(GL_DEPTH_BUFFER_BIT);

    glPolygonOffset(1.0, 1.0);                                                                          // draw the meshes
    glEnable(GL_POLYGON_OFFSET_FILL);
    meshes.Draw();
    glDisable(GL_POLYGON_OFFSET_FILL);
    glFinish();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);                                                          // bind the default framebuffer

    glActiveTexture(GL_TEXTURE0 + light_tex_unit);
    GLuint light_mask;                      // Texture light_mask;
    glGenTextures(1, &light_mask);
    glBindTexture(GL_TEXTURE_RECTANGLE, light_mask);
    glTexStorage2D(GL_TEXTURE_RECTANGLE, 1, GL_RED, width, height);

    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint light_fbo;                                                                                   // Framebuffer light_fbo;
    glGenFramebuffers(1, &light_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light_fbo);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_mask, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, light_rbo);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, light_rbo);

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glViewport(0, 0, res_x, res_y);

    //===================================================================================================================================================================================================================
    // program main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();

        glm::mat4 camera = window.camera.view_matrix;

        glm::mat4 mm_identity = glm::mat4(1.0f);
        glm::mat4 mm_rotation = glm::rotate() ModelMatrixf::RotationZ(FullCircles(time / 7.0));

        //===============================================================================================================================================================================================================
        // render the light mask
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light_fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mask_vao.bind();
        mask_prog.enable();
        uni_mp_camera_matrix = camera;

        /* with uni_mp_model_matrix --> mm_identity for non-fan meshes, and uni_mp_model_matrix --> mm_rotation for fan_index mesh */
        meshes.Draw(drawing_driver);

        //===============================================================================================================================================================================================================
        // render to screen
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        draw_vao.bind();

        draw_prog.enable();
        glm::vec4 lsp = projection * camera * glm::vec4(light_position, 1.0);

        uni_dp_light_screen_pos = lsp.xyz() / lsp.w();
        uni_dp_camera_matrix = camera;

        /* with uni_dp_model_matrix --> mm_identity for non-fan meshes, and uni_dp_model_matrix --> mm_rotation for fan_index mesh */
        meshes.Draw(drawing_driver);

        //===============================================================================================================================================================================================================
        // show back buffer
        //===============================================================================================================================================================================================================
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}
