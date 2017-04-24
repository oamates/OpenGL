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
    uniform_t uniform_dt = particle_compute["dt"];
    uniform_t uniform_time = particle_compute["time"];

    glsl_program_t particle_render(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/particle_render.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/particle_render.fs"));
    uniform_t uniform_projection_view_matrix = particle_render["projection_view_matrix"];

    //===================================================================================================================================================================================================================
    // point data initialization 
    //===================================================================================================================================================================================================================
    const int PARTICLE_GROUP_SIZE  = 128;
    const int PARTICLE_GROUP_COUNT = 16384;
    const int PARTICLE_COUNT       = PARTICLE_GROUP_SIZE * PARTICLE_GROUP_COUNT;
    const int ATTRACTOR_COUNT      = 2;

    float attractor_masses[ATTRACTOR_COUNT];

    GLuint vao_id, position_buffer, velocity_buffer, attractor_buffer;

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glGenBuffers(1, &position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
    glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);

    glm::vec4* positions = (glm::vec4*) glMapBufferRange(GL_ARRAY_BUFFER, 0, PARTICLE_COUNT * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    for (int i = 0; i < PARTICLE_COUNT; i++)
        positions[i] = glm::vec4(glm::ballRand(100.0f), glm::gaussRand(0.0f, 1.0f));

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &velocity_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, velocity_buffer);
    glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);

    glm::vec4* velocities = (glm::vec4*) glMapBufferRange(GL_ARRAY_BUFFER, 0, PARTICLE_COUNT * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    for (int i = 0; i < PARTICLE_COUNT; i++)
        velocities[i] = glm::vec4(glm::gaussRand(0.0f, 10.0f) * glm::sphericalRand(1.0f), 0.0f);

    glUnmapBuffer(GL_ARRAY_BUFFER);

    GLuint position_tbo, velocity_tbo;
    glGenTextures(1, &position_tbo);
    glBindTexture(GL_TEXTURE_BUFFER, position_tbo);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, position_buffer);

    glGenTextures(1, &velocity_tbo);
    glBindTexture(GL_TEXTURE_BUFFER, velocity_tbo);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, velocity_buffer);

    glGenBuffers(1, &attractor_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, attractor_buffer);
    glBufferData(GL_UNIFORM_BUFFER, ATTRACTOR_COUNT * sizeof(glm::vec4), 0, GL_STATIC_DRAW);

    for (int i = 0; i < ATTRACTOR_COUNT; i++)
        attractor_masses[i] = 1.5f + glm::abs(glm::gaussRand(0.0f, 12.0f));

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, attractor_buffer);
    glBindImageTexture(0, velocity_tbo, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(1, position_tbo, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST disabled
    // * Blending enabled
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.0f, 0.05f, 0.0f);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    double last_time = glfw::time();
    
    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        float dt = glm::min(window.frame_dt * 25.0, 2.0);

        glm::vec4* attractors = (glm::vec4* ) glMapBufferRange(GL_UNIFORM_BUFFER, 0, ATTRACTOR_COUNT * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (int i = 0; i < ATTRACTOR_COUNT; i++)
        {
            attractors[i] = glm::vec4(glm::sin(time * (i + 4) * 0.05f + 1.44f) * 25.0f,
                                      glm::sin(time * (i + 7) * 0.09f + 1.44f) * 25.0f,
                                      glm::sin(time * (i + 3) * 0.03f + 1.44f) * 25.0f,
                                      attractor_masses[i]);
        };

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        particle_compute.enable();
        uniform_dt = dt;
        uniform_time = time;
        glDispatchCompute(PARTICLE_GROUP_COUNT, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        particle_render.enable();
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        uniform_projection_view_matrix = projection_view_matrix;        
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