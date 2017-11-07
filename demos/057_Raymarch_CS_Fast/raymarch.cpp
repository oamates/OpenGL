//========================================================================================================================================================================================================================
// DEMO 056 : Ray Marching with Compute Shader
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "imgui_window.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "glsl_noise.hpp"
#include "image.hpp"

struct demo_window_t : public imgui_window_t
{
    camera_t camera;

    bool hell = true;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : imgui_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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

        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE))
            hell = !hell;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }

    void update_ui()
    {
        //===============================================================================================================================================================================================================
        // show a simple fps window.
        //===============================================================================================================================================================================================================
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Ray Marching with Compute Shader", 4, 4, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // ray march compute shader
        //===================================================================================================================================================================================================================
    glsl_program_t ray_marcher(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/ray_marcher.cs"));
    ray_marcher.enable();
    ray_marcher["focal_scale"] = glm::vec2(1.0f / window.camera.projection_matrix[0][0], 1.0f / window.camera.projection_matrix[1][1]);
    ray_marcher["tb_tex"] = 1;
    ray_marcher["noise_tex"] = 2;
    uniform_t uni_rm_camera_matrix = ray_marcher["camera_matrix"];
    uniform_t uni_rm_camera_ws = ray_marcher["camera_ws"];
    uniform_t uni_rm_light_ws = ray_marcher["light_ws"];

    glsl_program_t quad_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/quad.fs"));
    quad_renderer.enable();
    quad_renderer["raymarch_tex"] = 3;

    //===================================================================================================================================================================================================================
    // create/load noise and ambient output textures
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE1);
    GLuint tb_tex_id = image::png::texture2d("../../../resources/tex2d/clay.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    glActiveTexture(GL_TEXTURE2);
    GLuint noise_tex = glsl_noise::randomRGBA_shift_tex256x256(glm::ivec2(37, 17));

    //===================================================================================================================================================================================================================
    // create output textures, load texture for trilinear blend shading and generate noise texture
    //===================================================================================================================================================================================================================
    const int BUFFER_SIZE = 3;
    GLuint output_image[BUFFER_SIZE];
    glActiveTexture(GL_TEXTURE3);
    glGenTextures(BUFFER_SIZE, output_image);

    for (int i = 0; i < BUFFER_SIZE; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, output_image[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, window.res_x, window.res_y);
    }



    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    int compute_index = BUFFER_SIZE - 1;
    int render_index = 0;

    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        glm::mat4 cmatrix4x4 = window.camera.camera_matrix();
        glm::mat3 camera_matrix = glm::mat3(cmatrix4x4);
        glm::vec3 camera_ws = glm::vec3(cmatrix4x4[3]);
        glm::vec3 light_ws = glm::vec3(75.0f * glm::cos(0.25f * time), 100.0f, 75.0f * glm::sin(0.25f * time));

        //===============================================================================================================================================================================================================
        // Render scene
        //===============================================================================================================================================================================================================
        ray_marcher.enable();
        glBindImageTexture(0, output_image[compute_index], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

        uni_rm_camera_matrix = camera_matrix;
        uni_rm_camera_ws = camera_ws;
        uni_rm_light_ws = light_ws;

        glDispatchCompute(window.res_x / 20, window.res_y / 20, 1);

        quad_renderer.enable();

        glBindVertexArray(vao_id);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, output_image[render_index]);

        //===============================================================================================================================================================================================================
        // After end_frame call ::
        //  - GL_DEPTH_TEST is disabled
        //  - GL_CULL_FACE is disabled
        //  - GL_SCISSOR_TEST is enabled
        //  - GL_BLEND is enabled -- blending mode GL_SRC_ALPHA/GL_ONE_MINUS_SRC_ALPHA with blending function GL_FUNC_ADD
        //  - VAO binding is destroyed
        //===============================================================================================================================================================================================================
        window.end_frame();
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_BLEND);

        compute_index++;
        if (compute_index == BUFFER_SIZE)
            compute_index = 0;
        render_index++;
        if (render_index == BUFFER_SIZE)
            render_index = 0;
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glDeleteVertexArrays(1, &vao_id);

    glfw::terminate();
    return 0;
}