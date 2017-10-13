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
#include "gl_info.hpp"
#include "imgui_window.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "glsl_noise.hpp"
#include "image.hpp"

const float z_near = 1.0f;

struct demo_window_t : public imgui_window_t
{
    camera_t camera;

    bool split_screen = false;
    int aa_mode = 0;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : imgui_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), z_near);
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

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE))
            split_screen = !split_screen;

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
            aa_mode = (aa_mode + 1) % 3;
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
    ray_marcher["z_near"] = z_near;    

    glm::vec2 focal_scale = window.camera.focal_scale();
    float pixel_size = (2.0f * focal_scale.x) / window.res_x;

    debug_msg("Camera focal scale = %s", glm::to_string(focal_scale).c_str());
    debug_msg("Pixel size = %f", pixel_size);

    ray_marcher["focal_scale"] = focal_scale;
    ray_marcher["pixel_size"] = pixel_size;
    ray_marcher["tb_tex"] = 2;
    ray_marcher["bump_tb_tex"] = 3;


    uniform_t uni_rm_camera_matrix = ray_marcher["camera_matrix"];
    uniform_t uni_rm_camera_ws = ray_marcher["camera_ws"];
    uniform_t uni_rm_light_ws = ray_marcher["light_ws"];
    uniform_t uni_rm_split_screen = ray_marcher["split_screen"];

    glsl_program_t quad_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/quad.fs"));
    quad_renderer.enable();
    quad_renderer["raymarch_tex"] = 1;
    quad_renderer["focal_scale"] = focal_scale;
    quad_renderer["pixel_size"] = glm::vec2(1.0f / window.res_x, 1.0f / window.res_y);
    quad_renderer["z_near"] = z_near;
    uniform_t uni_aa_mode = quad_renderer["aa_mode"];

    //===================================================================================================================================================================================================================
    // create output textures, load texture for trilinear blend shading and generate noise texture
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE1);
    GLuint output_image;
    glGenTextures(1, &output_image);
    glBindTexture(GL_TEXTURE_2D, output_image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, window.res_x, window.res_y);
    glBindImageTexture(0, output_image, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);



    glActiveTexture(GL_TEXTURE2);
    GLuint tb_tex_id = image::png::texture2d_luma("../../../resources/tex2d/nature/rocks/1.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    GLfloat max_anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);

    glActiveTexture(GL_TEXTURE3);
    GLuint bump_tex_id = image::png::texture2d_luma("../../../resources/tex2d/nature/rocks/bump1.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);


    GLuint sampler_id;
    glGenSamplers(1, &sampler_id);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameterf(sampler_id, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);
    glBindSampler(2, sampler_id);

    //glActiveTexture(GL_TEXTURE3);
    //GLuint noise_tex = glsl_noise::randomRGBA_shift_tex256x256(glm::ivec2(37, 17));

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);

    //===================================================================================================================================================================================================================
    // main program loop : just clear the buffer in a loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        glm::mat4 cmatrix4x4 = window.camera.camera_matrix();
        glm::mat3 camera_matrix = glm::mat3(cmatrix4x4);
        glm::vec3 camera_ws = glm::vec3(cmatrix4x4[3]);
        glm::vec3 light_ws = 1.25f * glm::vec3(glm::cos(0.25f * time), glm::sin(0.25f * time), 0.43f);

        //===============================================================================================================================================================================================================
        // Render scene
        //===============================================================================================================================================================================================================
        ray_marcher.enable();
        uni_rm_camera_matrix = camera_matrix;
        uni_rm_camera_ws = camera_ws;
        uni_rm_light_ws = light_ws;
        uni_rm_split_screen = int (window.split_screen ? 1 : 0);

        glDispatchCompute(window.res_x / 8, window.res_y / 8, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);

        quad_renderer.enable();
        uni_aa_mode = window.aa_mode;
        glBindVertexArray(vao_id);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glDeleteVertexArrays(1, &vao_id);

    glfw::terminate();
    return 0;
}