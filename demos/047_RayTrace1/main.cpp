//========================================================================================================================================================================================================================
// DEMO 047 : Ray Tracer
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "imgui_window.hpp"
#include "camera.hpp"
#include "shader.hpp"

struct sphere_t
{
    glm::vec3 center;                           // sphere center
    float radius;                               // sphere radius
    glm::vec3 albedo;                           // surface albedo color
    float transparency;                         // surface transparency
    glm::vec3 emission;                         // surface emission color
    float energy;                               // emission energy
    float reflectivity;                         // surface reflectivity
    float ior;                                  // surface index of refraction
    float pad0;                                 // padding0
    float pad1;                                 // padding1
};

sphere_t spheres[] = 
{
    { .center = glm::vec3( 0.0f, -10007.7f,   0.0f), .radius = 10000.0f, .albedo = glm::vec3(0.80f, 0.71f, 0.07f), .transparency = 0.07f, .emission = glm::vec3(0.01, 0.20, 0.04), .energy = 0.71f, .reflectivity = 0.27f, .ior = 1.17f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3(10.0f,      1.7f,   5.0f), .radius =     4.0f, .albedo = glm::vec3(1.33f, 1.24f, 0.23f), .transparency = 0.41f, .emission = glm::vec3(0.07, 0.12, 0.11), .energy = 0.21f, .reflectivity = 0.67f, .ior = 1.27f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3( 3.0f,      2.3f, -20.0f), .radius =     2.0f, .albedo = glm::vec3(0.90f, 0.26f, 0.46f), .transparency = 0.23f, .emission = glm::vec3(0.05, 0.06, 0.09), .energy = 0.23f, .reflectivity = 0.47f, .ior = 1.37f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3(-3.0f,      0.7f,  -1.0f), .radius =     3.0f, .albedo = glm::vec3(0.65f, 0.77f, 0.97f), .transparency = 0.33f, .emission = glm::vec3(0.03, 0.11, 0.17), .energy = 0.44f, .reflectivity = 0.37f, .ior = 1.11f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3( 1.5f,     -1.5f,  10.0f), .radius =     3.0f, .albedo = glm::vec3(0.90f, 0.31f, 0.90f), .transparency = 0.51f, .emission = glm::vec3(0.21, 0.10, 0.06), .energy = 0.09f, .reflectivity = 0.41f, .ior = 1.41f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3(13.0f,      2.1f,  -5.0f), .radius =     2.2f, .albedo = glm::vec3(0.77f, 0.47f, 0.05f), .transparency = 0.59f, .emission = glm::vec3(0.91, 0.19, 0.11), .energy = 0.41f, .reflectivity = 0.35f, .ior = 1.21f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3(-5.0f,     -1.5f, -15.0f), .radius =     4.7f, .albedo = glm::vec3(0.21f, 1.12f, 0.17f), .transparency = 0.71f, .emission = glm::vec3(0.01, 0.20, 0.04), .energy = 0.21f, .reflectivity = 0.43f, .ior = 1.25f, .pad0 = 0.0f, .pad1 = 0.0f },
    { .center = glm::vec3( 3.0f,      1.2f,  17.0f), .radius =     4.2f, .albedo = glm::vec3(0.31f, 0.12f, 1.14f), .transparency = 0.43f, .emission = glm::vec3(0.01, 0.20, 0.04), .energy = 0.11f, .reflectivity = 0.51f, .ior = 1.19f, .pad0 = 0.0f, .pad1 = 0.0f }
};

const int SPHERE_COUNT = sizeof(spheres) / sizeof(sphere_t);

struct demo_window_t : public imgui_window_t
{
    camera_t camera;
    bool params_changed;
    int index = 0;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : imgui_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true*/ ),
          camera(4.0, 0.25, glm::lookAt(glm::vec3(7.0f, 7.0f, 4.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
    {
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.5);
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

    void update_ui() override
    {
        params_changed = false;

        ImGui::SetNextWindowSize(ImVec2(768, 768), ImGuiWindowFlags_NoResize | ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Raytrace Demo", 0);
        ImGui::Text("Application average framerate (%.3f FPS)", ImGui::GetIO().Framerate);

        ImGui::Text("Sphere index");

        for(int i = 0; i < SPHERE_COUNT; ++i)
        {
            char index_str[8];
            sprintf(index_str, "%d", i);
            ImGui::RadioButton(index_str, &index, i);
            if (i != SPHERE_COUNT - 1)
                ImGui::SameLine();
        }

        sphere_t& sphere = spheres[index];

        if (ImGui::CollapsingHeader("Center and radius"))
        {
            params_changed |= ImGui::SliderFloat("X", &sphere.center.x, -50.0f, 4.0f, "X = %.3f");
            params_changed |= ImGui::SliderFloat("Y", &sphere.center.y, -50.0f, 4.0f, "Y = %.3f");
            params_changed |= ImGui::SliderFloat("Z", &sphere.center.z, -2.0f, 20.0f, "Z = %.3f");
            params_changed |= ImGui::SliderFloat("Radius", &sphere.radius, 0.125f, 8.0f, "R = %.3f");
        }

        if (ImGui::CollapsingHeader("Albedo color and transparency"))
        {
            params_changed |= ImGui::SliderFloat("Red  ", &sphere.albedo.x, 0.0f, 2.0f, "R = %.3f");
            params_changed |= ImGui::SliderFloat("Green", &sphere.albedo.y, 0.0f, 2.0f, "G = %.3f");
            params_changed |= ImGui::SliderFloat("Blue ", &sphere.albedo.z, 0.0f, 2.0f, "B = %.3f");
            params_changed |= ImGui::SliderFloat("Alpha", &sphere.transparency, 0.0f, 1.0f, "A = %.3f");
        }

        if (ImGui::CollapsingHeader("Emission color and energy"))
        {
            params_changed |= ImGui::SliderFloat("Red   ", &sphere.emission.x, 0.0f, 2.0f, "R = %.3f");
            params_changed |= ImGui::SliderFloat("Green ", &sphere.emission.y, 0.0f, 2.0f, "G = %.3f");
            params_changed |= ImGui::SliderFloat("Blue  ", &sphere.emission.z, 0.0f, 2.0f, "B = %.3f");
            params_changed |= ImGui::SliderFloat("Energy", &sphere.energy, 0.0625f, 4.0f, "E = %.3f");
        }

        if (ImGui::CollapsingHeader("Reflectivity and index of refraction"))
        {
            params_changed |= ImGui::SliderFloat("Reflectivity", &sphere.reflectivity, 0.0f, 2.0f, "R = %.3f");
            params_changed |= ImGui::SliderFloat("IOR", &sphere.ior, 0.5f, 2.0f, "IOR = %.3f");
        }

        ImGui::End();
    }

};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char* argv[])
{
    const int res_x = 1920;
    const int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Ray Tracer", 4, 4, 3, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // shaders
    //===================================================================================================================================================================================================================
    glsl_program_t ray_tracer(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/trace.cs"));
    ray_tracer.enable();

    uniform_t uni_rt_camera_ws = ray_tracer["camera_ws"];
    uniform_t uni_rt_camera_matrix = ray_tracer["camera_matrix"];

    ray_tracer["inv_res"] = glm::vec2(1.0f / res_x, 1.0f / res_y);
    ray_tracer["focal_scale"] = window.camera.focal_scale();
    ray_tracer["depth"] = 6;
    ray_tracer["sphere_count"] = 8;

    glsl_program_t quad_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/render.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/render.fs"));
    quad_renderer.enable();
    quad_renderer["raytrace_image"] = 0;


    //===================================================================================================================================================================================================================
    // uniform buffer for spheres
    //===================================================================================================================================================================================================================
    GLuint ubo_id;
    glGenBuffers(1, &ubo_id);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(spheres), spheres, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_id);

    //===================================================================================================================================================================================================================
    // fake VAO for full-screen quad rendering
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    //===================================================================================================================================================================================================================
    // raytrace texture will occupy texture unit 0 and image unit 0
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint raytrace_image;
    glGenTextures(1, &raytrace_image);
    glBindTexture(GL_TEXTURE_2D, raytrace_image);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, res_x, res_y);
    glBindImageTexture(0, raytrace_image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    //===================================================================================================================================================================================================================
    // Global GL settings : DEPTH not needed, hence disabled
    //===================================================================================================================================================================================================================
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();

        if (window.params_changed)
        {
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(spheres), spheres, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_id);
        }

        //===============================================================================================================================================================================================================
        // ray trace the scene
        //===============================================================================================================================================================================================================
        glm::mat4 cmatrix4x4 = window.camera.camera_matrix();
        glm::mat3 camera_matrix = glm::mat3(cmatrix4x4);
        glm::vec3 camera_ws = glm::vec3(cmatrix4x4[3]);

        ray_tracer.enable();
        uni_rt_camera_ws = camera_ws;
        uni_rt_camera_matrix = camera_matrix;

        glDispatchCompute(res_x / 8, res_y / 8, 1);                                             // fill initial ray buffer
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        //===============================================================================================================================================================================================================
        // copy image to screen buffer
        //===============================================================================================================================================================================================================
        glBindVertexArray(vao_id);
        quad_renderer.enable();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        window.end_frame();

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
        glViewport(0, 0, res_x, res_y);
    }

    glDeleteTextures(1, &raytrace_image);
    glDeleteVertexArrays(1, &vao_id);

    glfw::terminate();
    return 0;
}