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
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("VAO Loader", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // light
    //===================================================================================================================================================================================================================
    glm::vec3 lightPos(2.0f, 4.0f, -3.0f);
    glm::mat4 texProjMat = glm::infinitePerspective(constants::half_pi / 3.0f, 1.0f, 0.25f) *
                           glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const int samples = 150;

    //===================================================================================================================================================================================================================
    // init shaders
    //===================================================================================================================================================================================================================
    glsl_program_t volume_prog = glsl_program_t(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/volume.vs"),
                                                glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/volume.gs"),
                                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/volume.fs"));
    volume_prog.enable();
    volume_prog["ProjectionMatrix"] = window.camera.projection_matrix;
    volume_prog["SampleCount"] = samples;
    volume_prog["Size"] = glm::length(lightPos);
    volume_prog["TexProjectionMatrix"] = texProjMat;
    volume_prog["LightTex"] = 0;
    uniform_t uni_vp_camera_matrix = volume_prog["CameraMatrix"];
    uniform_t uni_vp_view_x = volume_prog["ViewX"];
    uniform_t uni_vp_view_y = volume_prog["ViewY"];
    uniform_t uni_vp_view_z = volume_prog["ViewZ"];

    glsl_program_t plane_prog(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/plane.vs"),
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/plane.fs"));
    plane_prog.enable();
    plane_prog["ProjectionMatrix"] = window.camera.projection_matrix;
    plane_prog["TexProjectionMatrix"] = texProjMat;
    uniform_t uni_pp_camera_matrix = plane_prog["CameraMatrix"];

    //===================================================================================================================================================================================================================
    // init geometry
    //===================================================================================================================================================================================================================

    GLuint volume_vao_id;
    glGenVertexArrays(1, &volume_vao_id);
    glBindVertexArray(volume_vao_id);

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

    GLuint volume_vbo_id;
    glGenBuffers(1, &volume_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, volume_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position), glm::value_ptr(position), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);


    GLuint plane_vao_id;
    glGenVertexArrays(1, &plane_vao_id);
    glBindVertexArray(plane_vao_id);

    glm::vec3 plane_vertices[] =
    {
        glm::vec3(-9.0f, -4.0f,  9.0f),
        glm::vec3(-9.0f, -4.0f, -9.0f),
        glm::vec3( 9.0f, -4.0f,  9.0f),
        glm::vec3( 9.0f, -4.0f, -9.0f)
    };

    GLuint plane_vbo_id;
    glGenBuffers(1, &plane_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, plane_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), glm::value_ptr(plane_vertices[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    const char* tex_file = "../../../resources/tex2d/flower_glass.png";
    glActiveTexture(GL_TEXTURE0);
    GLuint light_tex = image::stbi::texture2d(tex_file, 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_BORDER);

    //===================================================================================================================================================================================================================
    // common OpenGL settings
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.05f, 0.1f, 0.0f);
    glClearDepth(1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    //===================================================================================================================================================================================================================
    // program main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 cameraMatrix = window.camera.view_matrix;

        //===============================================================================================================================================================================================================
        // render floor
        //===============================================================================================================================================================================================================
        plane_prog.enable();
        uni_pp_camera_matrix = cameraMatrix;
        glBindVertexArray(plane_vao_id);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // render volumetric light
        //===============================================================================================================================================================================================================
        glEnable(GL_BLEND);

        volume_prog.enable();
        uni_vp_camera_matrix = cameraMatrix;
        uni_vp_view_x = glm::vec3(cameraMatrix[0][0], cameraMatrix[1][0], cameraMatrix[2][0]);
        uni_vp_view_y = glm::vec3(cameraMatrix[0][1], cameraMatrix[1][1], cameraMatrix[2][1]);
        uni_vp_view_z = glm::vec3(cameraMatrix[0][2], cameraMatrix[1][2], cameraMatrix[2][2]);

        glEnable(GL_BLEND);
        glBindVertexArray(volume_vao_id);
        glDrawArraysInstanced(GL_POINTS, 0, 1, samples);

        glDisable(GL_BLEND);

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
