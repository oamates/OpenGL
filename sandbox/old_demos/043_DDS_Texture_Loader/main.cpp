//========================================================================================================================================================================================================================
// DEMO 043: DDS Texture loader
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/matrix_transform.hpp> 

#include "constants.hpp"
#include "glfw_window.hpp"
#include "log.hpp"
#include "camera3d.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "dds.hpp"


//========================================================================================================================================================================================================================
// 3d moving camera : standard initial orientation in space
//========================================================================================================================================================================================================================
const float linear_velocity = 0.7f;
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


int main()
{
    //===================================================================================================================================================================================================================
    // GLFW window creation + GLEW library initialization
    // 8AA samples, OpenGL 4.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    glfw_window window("DDS Texture loader", 8, 4, 3, 1920, 1080);
    window.log_info();
    window.mouse_handler = mouse_handler;
    window.keyboard_handler = keyboard_handler;
    camera.infinite_perspective(constants::two_pi / 6.0f, window.aspect_ratio(), 0.1f);

    //===================================================================================================================================================================================================================
    // skybox and environment map shader initialization
    //===================================================================================================================================================================================================================
    glsl_program volume_renderer(glsl_shader(GL_VERTEX_SHADER,   "glsl/volume.vs"),
                                 glsl_shader(GL_FRAGMENT_SHADER, "glsl/volume.fs"));

    GLint uniform_projection_view_matrix = volume_renderer.uniform_id("projection_view_matrix");
    GLint uniform_instance_count = volume_renderer.uniform_id("instance_count");

    const int INSTANCE_COUNT = 256;
    volume_renderer.enable();
    glUniform1i(uniform_instance_count, INSTANCE_COUNT);

    //===================================================================================================================================================================================================================
    // Initialize cube buffer : vertices + indices
    //===================================================================================================================================================================================================================
    GLuint quad_vao_id, quad_vbo_id;

    glGenVertexArrays(1, &quad_vao_id);
    glBindVertexArray(quad_vao_id);

    glGenBuffers(1, &quad_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_id);

    static const glm::vec2 quad_data[] =
    {
        glm::vec2( 1.0f, -1.0f),
        glm::vec2(-1.0f, -1.0f),
        glm::vec2(-1.0f,  1.0f),
        glm::vec2( 1.0f,  1.0f),

        glm::vec2( 0.0f,  0.0f),
        glm::vec2( 1.0f,  0.0f),
        glm::vec2( 1.0f,  1.0f),
        glm::vec2( 0.0f,  1.0f)
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *) (4 * sizeof(glm::vec2)));

    //===================================================================================================================================================================================================================
    // Loading DDS 3D texture
    //===================================================================================================================================================================================================================
    vglImageData image;
    GLuint tex = vglLoadTexture("res/cloud.dds", 0, &image);
    glTexParameteri(image.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    vglUnloadImage(&image);

    //===================================================================================================================================================================================================================
    // Global GL settings
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection_view_matrix = camera.projection_view_matrix();

        glUniformMatrix4fv(uniform_projection_view_matrix, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, INSTANCE_COUNT);

        window.swap_buffers();
        window.poll_events();
    };
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    return 0;
};