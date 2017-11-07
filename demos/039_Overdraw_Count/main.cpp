//========================================================================================================================================================================================================================
// DEMO 039 : Overdraw count
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "vertex.hpp"
#include "sphere.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
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

vertex_pnt3_t minkowski_L4_support_func(const glm::vec3& uvw)
{
    vertex_pnt3_t vertex;    
    vertex.uvw = uvw;

    glm::vec3 uvw2 = uvw * uvw;
    glm::vec3 uvw3 = uvw2 * uvw;

    float inv_norm = 1.0f / sqrt(glm::length(uvw2));
    float inv_der_norm = 1.0f / glm::length(uvw3);

    vertex.position = 0.5f * uvw * inv_norm;
    vertex.normal = glm::normalize(uvw3);

    return vertex;
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    int res_x = 1920;
    int res_y = 1080;
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Overdraw Count", 4, 4, 4, res_x, res_y, true);

    glsl_program_t scene_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/overdraw_count.vs"),
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/overdraw_count.fs"));

    scene_renderer.enable();
    uniform_t uni_sr_pv_matrix = scene_renderer["projection_view_matrix"];
    scene_renderer["base_size"] = 1.0f;

    glsl_program_t resolver(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/blit.vs"), 
                            glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blit.fs"));

    //===================================================================================================================================================================================================================
    // Create overdraw counter texture
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);

    GLuint counter_tex;
    glGenTextures(1, &counter_tex);
    glBindTexture(GL_TEXTURE_2D, counter_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, res_x, res_y);
    glBindImageTexture(0, counter_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    //===================================================================================================================================================================================================================
    // fake VAO for quad rendering
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);

    //===================================================================================================================================================================================================================
    // VAO to render
    //===================================================================================================================================================================================================================
    //===================================================================================================================================================================================================================
    // create dodecahecron buffer
    //===================================================================================================================================================================================================================

    sphere_t minkowski_L4_ball;
    minkowski_L4_ball.generate_vao_mt<vertex_pnt3_t>(minkowski_L4_support_func, 4);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(minkowski_L4_ball.vao.ibo.pri);

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.03f, 0.01f, 0.09f, 1.0f);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT);
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        GLuint zero = 0;
        glClearTexImage(counter_tex, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
        scene_renderer.enable();
        uni_sr_pv_matrix = projection_view_matrix;

        minkowski_L4_ball.instanced_render(16 * 16 * 16);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        resolver.enable();
        glBindVertexArray(vao_id);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        window.end_frame();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}