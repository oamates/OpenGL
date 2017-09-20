//=======================================================================================================================================================================================================================
// DEMO 049 : GLFW + OpenGL 3.3 + ImGui Example
//=======================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "imgui_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "glsl_noise.hpp"
#include "image.hpp"

struct demo_window_t : public imgui_window_t
{
    camera_t camera;
    bool pause = true;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : imgui_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
          camera(5.0, 0.125, glm::mat4(1.0f))
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

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE))
            pause = !pause;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }

    void update_ui() override
    {
        //===============================================================================================================================================================================================================
        // show a simple fps window.
        //===============================================================================================================================================================================================================
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }

};


glm::vec3 tri(const glm::vec3& x)
{
    return glm::abs(glm::fract(x) - glm::vec3(0.5f));
}

float potential(const glm::vec3& p)
{    
    glm::vec3 q = p;
    glm::vec3 oq = tri(1.1f * q + tri(1.1f * glm::vec3(q.z, q.x, q.y)));
    float ground = q.z + 3.5f + glm::dot(oq, glm::vec3(0.067));
    q += (oq - glm::vec3(0.25f)) * 0.3f;
    q = glm::cos(0.444f * q + glm::sin(1.112f * glm::vec3(q.z, q.x, q.y)));
    float canyon = 0.95f * (glm::length(p) - 1.05f);
    float sphere = 11.0f - glm::length(p);
    return glm::min(glm::min(ground, canyon), sphere);
}

glm::vec3 gradient(const glm::vec3& p)
{
    const float delta = 0.075f;

    const glm::vec3 dX = glm::vec3(delta, 0.0f, 0.0f);
    const glm::vec3 dY = glm::vec3(0.0f, delta, 0.0f);
    const glm::vec3 dZ = glm::vec3(0.0f, 0.0f, delta);

    glm::vec3 dF = glm::vec3
    (
        potential(p + dX) - potential(p - dX),
        potential(p + dY) - potential(p - dY),
        potential(p + dZ) - potential(p - dZ)
    );

    return glm::normalize(dF);
}

glm::vec3 move(glm::vec3& position, float& p, float dt)
{
    do
    {
        glm::vec3 v = glm::sphericalRand(1.0f);
        position = position + v * dt;
        p = potential(position);
    }
    while(p < 0.5);
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("GLFW + OpenGL 3.3 + ImGui Example", 4, 3, 3, 1920, 1080);


    //===================================================================================================================================================================================================================
    // Shader and uniform variables initialization
    //===================================================================================================================================================================================================================
    glsl_program_t ray_marcher(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ray_marcher.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/canyon.fs"));
    ray_marcher.enable();
    uniform_t uni_rm_camera_matrix = ray_marcher["camera_matrix"];
    uniform_t uni_rm_camera_ws = ray_marcher["camera_ws"];
    uniform_t uni_rm_light_ws = ray_marcher["light_ws"];
    uniform_t uni_rm_time = ray_marcher["time"];

    glm::vec2 focal_scale = glm::vec2(1.0f / window.camera.projection_matrix[0][0], 1.0f / window.camera.projection_matrix[1][1]);
    ray_marcher["focal_scale"] = focal_scale;
    ray_marcher["value_tex"] = 0;
    ray_marcher["stone_tex"] = 1;

    //===================================================================================================================================================================================================================
    // Load noise textures - for very fast 2d noise calculation
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint noise_tex = glsl_noise::randomRGBA_shift_tex256x256(glm::ivec2(37, 17));
    glActiveTexture(GL_TEXTURE1);
    GLuint stone_tex = image::png::texture2d("../../../resources/tex2d/clay.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);   

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);

    glm::vec3 light_ws;
    float p;
    do
    {
        light_ws = glm::sphericalRand(3.0f);
        p = potential(light_ws);
    }
    while(p < 0.5);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        //===============================================================================================================================================================================================================
        // glClear is not necessary here as we cover the whole framebuffer
        //===============================================================================================================================================================================================================
        window.new_frame();

        //===============================================================================================================================================================================================================
        // render the raymarch scene
        //===============================================================================================================================================================================================================
        glBindVertexArray(vao_id);

        ray_marcher.enable();

        float time = window.frame_ts;
        float dt = window.frame_dt;

        if (!window.pause)
            move(light_ws, p, dt);

        glm::mat4 cmatrix4x4 = glm::inverse(window.camera.view_matrix);
        glm::mat3 camera_matrix = glm::mat3(cmatrix4x4);
        glm::vec3 camera_ws = glm::vec3(cmatrix4x4[3]);

        uni_rm_time = time;
        uni_rm_camera_matrix = camera_matrix;
        uni_rm_camera_ws = camera_ws;
        uni_rm_light_ws = light_ws;

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

    glfw::terminate();
    return 0;
}
