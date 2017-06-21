//========================================================================================================================================================================================================================
// DEMO 028 : CS + GL_TEXTURE_BUFFER
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

    demo_window_t window("CS + GL_TEXTURE_BUFFER", 4, 4, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // create programs : one for particle compute, the other for render
    //===================================================================================================================================================================================================================
    glsl_program_t particle_compute(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/particle.cs"));
    particle_compute.enable();
    uniform_t uniform_dt = particle_compute["dt"];
    uniform_t uniform_time = particle_compute["time"];

    glsl_program_t particle_render(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/particle_render.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/particle_render.fs"));
    particle_render.enable();
    uniform_t uni_pr_pv_matrix = particle_render["projection_view_matrix"];
    uniform_t uni_pr_camera_ws = particle_render["camera_ws"];
    uniform_t uni_pr_light_ws = particle_render["light_ws"];
    particle_render["tb_tex2d"] = 0;

    //===================================================================================================================================================================================================================
    // point data initialization 
    //===================================================================================================================================================================================================================
    const int PARTICLE_GROUP_SIZE  = 128;
    const int PARTICLE_GROUP_COUNT = 16384;
    const int PARTICLE_COUNT = PARTICLE_GROUP_SIZE * PARTICLE_GROUP_COUNT;

    GLuint vao_id, position_buffer;

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glGenBuffers(1, &position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
    glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);

    glm::vec4* positions = (glm::vec4*) glMapBufferRange(GL_ARRAY_BUFFER, 0, PARTICLE_COUNT * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    for (int i = 0; i < PARTICLE_COUNT; i++)
        positions[i] = glm::vec4(glm::gaussRand(0.0f, 1.0f), glm::gaussRand(0.0f, 1.0f), glm::gaussRand(0.0f, 1.0f), glm::gaussRand(0.0f, 1.0f));

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    GLuint position_tbo;
    glGenTextures(1, &position_tbo);
    glBindTexture(GL_TEXTURE_BUFFER, position_tbo);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, position_buffer);
    glBindImageTexture(0, position_tbo, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE0);
    GLuint tb_tex_id = image::png::texture2d("../../../resources/tex2d/marble.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);


    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST enabled
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.0f, 0.05f, 0.0f);
    glEnable(GL_DEPTH_TEST);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();


        float time = window.frame_ts;
        float dt = window.frame_dt;

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 camera_ws = window.camera.position();
        glm::vec3 light_ws = 8.75f * glm::vec3(glm::cos(time), 0.0f, glm::sin(time));


        particle_compute.enable();
        uniform_dt = dt;
        uniform_time = time;
        glDispatchCompute(PARTICLE_GROUP_COUNT, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        particle_render.enable();
        uni_pr_pv_matrix = projection_view_matrix;
        uni_pr_camera_ws = camera_ws;
        uni_pr_light_ws = light_ws;

        glBindVertexArray(vao_id);
        glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);

        window.end_frame();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}