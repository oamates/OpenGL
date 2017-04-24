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
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "vertex.hpp"
#include "sphere.hpp"

#define MAX_FRAMEBUFFER_WIDTH 2048
#define MAX_FRAMEBUFFER_HEIGHT 2048

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
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

    vertex.position = 4.0f * uvw * inv_norm;
    vertex.normal = glm::normalize(uvw3);

    return vertex;
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Overdraw Count", 4, 4, 3, 1920, 1080, true);

    glsl_program_t scene_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/overdraw_count.vs"),
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/overdraw_count.fs"));

    uniform_t view_matrix       = scene_renderer["view_matrix"];
    uniform_t projection_matrix = scene_renderer["projection_matrix"];
    scene_renderer.enable();
    scene_renderer["base_size"] = 15.0f;

    glsl_program_t resolver(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/blit.vs"), 
                            glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blit.fs"));

    //===================================================================================================================================================================================================================
    // Create overdraw counter texture
    //===================================================================================================================================================================================================================
    GLuint overdraw_count_buffer;
    glGenTextures(1, &overdraw_count_buffer);
    glBindTexture(GL_TEXTURE_2D, overdraw_count_buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    //===================================================================================================================================================================================================================
    // Create buffer for clearing the head pointer texture
    //===================================================================================================================================================================================================================
    GLuint overdraw_count_clear_buffer;
    glGenBuffers(1, &overdraw_count_clear_buffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, overdraw_count_clear_buffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, MAX_FRAMEBUFFER_WIDTH * MAX_FRAMEBUFFER_HEIGHT * sizeof(GLuint), 0, GL_STATIC_DRAW);

    void* unpack_buffer_data = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    memset(unpack_buffer_data, 0x00, MAX_FRAMEBUFFER_WIDTH * MAX_FRAMEBUFFER_HEIGHT * sizeof(GLuint));
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

    //===================================================================================================================================================================================================================
    // Create VAO containing quad for the final blit
    //===================================================================================================================================================================================================================
    GLuint  quad_vbo;
    GLuint  quad_vao;

    glGenVertexArrays(1, &quad_vao);
    glBindVertexArray(quad_vao);

    static const GLfloat quad_verts[] =
    {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };

    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), quad_verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

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
        glClear(GL_COLOR_BUFFER_BIT);
        window.new_frame();

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, overdraw_count_clear_buffer);
        glBindTexture(GL_TEXTURE_2D, overdraw_count_buffer);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, window.res_x, window.res_y, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glBindImageTexture(0, overdraw_count_buffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

        scene_renderer.enable();
        view_matrix = window.camera.view_matrix;
        projection_matrix = window.camera.projection_matrix;

        minkowski_L4_ball.instanced_render(16 * 16 * 16);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
        resolver.enable();
        glBindVertexArray(quad_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        window.new_frame();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}