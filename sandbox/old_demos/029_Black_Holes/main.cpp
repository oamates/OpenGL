//========================================================================================================================================================================================================================
// DEMO 028 : Compute Shader Particle System
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include "constants.hpp"
#include "glfw_window.hpp"
#include "log.hpp"
#include "camera3d.hpp"
#include "shader.hpp"
#include "sphere.hpp"

//========================================================================================================================================================================================================================
// 3d moving camera : standard initial orientation in space
//========================================================================================================================================================================================================================
const float linear_velocity = 2.7f;
const float angular_rate = 0.0001f;
static camera3d camera;

//========================================================================================================================================================================================================================
// keyboard and mouse handlers
//========================================================================================================================================================================================================================
void keyboard_handler(int key, int scancode, int action, int mods)
{
    if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(linear_velocity);
    else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(linear_velocity);
    else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(linear_velocity);
    else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(linear_velocity);
};

void mouse_handler(double dx, double dy, double duration)
{
    duration = glm::max(duration, 0.01);    
    double norm = sqrt(dx * dx + dy * dy);
    if (norm > 0.01f)
    {
        dx /= norm; dy /= norm;
        double angle = angular_rate * sqrt(norm) / (duration + 0.01);
        camera.rotateXY(dx, dy, angle);
    };
};

vertex_pnt3 unit_sphere_func(const glm::vec3& direction)
{
    glm::vec3 unit_direction = glm::normalize(direction);
    return vertex_pnt3(unit_direction, unit_direction, unit_direction);
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main()
{
    //===================================================================================================================================================================================================================
    // GLFW window creation + GLEW library initialization
    // 8AA samples, OpenGL 4.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    glfw_window window("Compute Shader Particle System", 8, 4, 3, 1920, 1080);
    window.log_info();
    window.mouse_handler = mouse_handler;
    window.keyboard_handler = keyboard_handler;
    camera.infinite_perspective(constants::two_pi / 6.0f, window.aspect_ratio(), 0.1f);

	//===================================================================================================================================================================================================================
	// create programs : one for particle compute, the other for render
	//===================================================================================================================================================================================================================
    glsl_program particle_compute(glsl_shader(GL_COMPUTE_SHADER, "glsl/particle.cs"));
    //particle_compute.enable();
    GLint uniform_dt              = particle_compute.uniform_id("dt");
    GLint uniform_time            = particle_compute.uniform_id("time");
    GLint uniform_attractor_count = particle_compute.uniform_id("attractor_count");

    glsl_program particle_render(glsl_shader(GL_VERTEX_SHADER,   "glsl/particle_render.vs"),
                                 glsl_shader(GL_FRAGMENT_SHADER, "glsl/particle_render.fs"));

    GLint uniform_projection_view_matrix = particle_render.uniform_id("projection_view_matrix");

    glsl_program black_hole_renderer(glsl_shader(GL_VERTEX_SHADER,   "glsl/black_hole.vs"),
                                     glsl_shader(GL_FRAGMENT_SHADER, "glsl/black_hole.fs"));

    GLint black_hole_projection_view_matrix = black_hole_renderer.uniform_id("projection_view_matrix");
    GLint black_hole_time                   = black_hole_renderer.uniform_id("time");

	//===================================================================================================================================================================================================================
	// Point data initialization 
	//===================================================================================================================================================================================================================
    const int PARTICLE_GROUP_SIZE  = 128;
    const int PARTICLE_GROUP_COUNT = 16384;
    const int PARTICLE_COUNT       = PARTICLE_GROUP_SIZE * PARTICLE_GROUP_COUNT;
    const int MAX_ATTRACTOR_COUNT  = 64;
    const int ATTRACTOR_COUNT      = 5;

    float attractor_radii[ATTRACTOR_COUNT];
    for (int i = 0; i < ATTRACTOR_COUNT; i++)
        attractor_radii[i] = glm::linearRand(4.0f, 8.0f);

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
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, attractor_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_ATTRACTOR_COUNT * sizeof(glm::vec4), 0, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, attractor_buffer);

    glBindImageTexture(0, velocity_tbo, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(1, position_tbo, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST enabled
    // * Blending enabled
    //===================================================================================================================================================================================================================
	glClearColor(0.01f, 0.0f, 0.05f, 0.0f);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_DEPTH_TEST);

    double last_time = window.time();

    sphere black_hole;
    black_hole.generate_pnt_vao(unit_sphere_func, 32);

    
    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
	while(!window.should_close())
	{

        double current_time = window.time();
        float dt = current_time - last_time;

        glm::vec4* attractors = (glm::vec4* ) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ATTRACTOR_COUNT * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (int i = 0; i < ATTRACTOR_COUNT; i++)
        {
            attractors[i] = glm::vec4(glm::sin(current_time * (10 * i + 4) * 0.05f + 1.44f) * 25.0f,
                                      glm::sin(current_time * (10 * i + 7) * 0.09f + 1.44f) * 25.0f,
                                      glm::sin(current_time * (10 * i + 3) * 0.03f + 1.44f) * 25.0f,
                                      attractor_radii[i]);
        };

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        particle_compute.enable();

        glUniform1i(uniform_attractor_count, ATTRACTOR_COUNT);
        glUniform1f(uniform_dt, dt);
        glUniform1f(uniform_time, current_time);
        glDispatchCompute(PARTICLE_GROUP_COUNT, 1, 1);

        glm::mat4 projection_view_matrix = camera.projection_view_matrix();
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);

        particle_render.enable();
        glUniformMatrix4fv(uniform_projection_view_matrix, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));        
        glBindVertexArray(vao_id);

        //===============================================================================================================================================================================================================
        // dispatch compute has to finish at this point 
        //===============================================================================================================================================================================================================
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);

        glDisable(GL_BLEND);

        black_hole_renderer.enable();
        glUniform1f(black_hole_time, current_time);
        glUniformMatrix4fv(black_hole_projection_view_matrix, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));        
        black_hole.instanced_render(ATTRACTOR_COUNT);

        last_time = current_time;

        window.swap_buffers();
        window.poll_events();
    };
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
	return 0;
};