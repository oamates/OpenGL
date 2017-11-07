//========================================================================================================================================================================================================================
// DEMO 008 : Geometry Shader
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
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
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Geometry Shader", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // shader compulation and uniform id 
    //===================================================================================================================================================================================================================
    glsl_program_t cubes_program(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/cubes.vs"),
                                 glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/cubes.gs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/cubes.fs"));

    cubes_program.enable();
    uniform_t uniform_projection_matrix = cubes_program["projection_matrix"];
    uniform_t uniform_view_matrix       = cubes_program["view_matrix"];
    uniform_t uniform_light_direction   = cubes_program["light_direction"];

    //===================================================================================================================================================================================================================
    // point data initialization 
    //===================================================================================================================================================================================================================
    const unsigned int POINT_COUNT = 0x8000;
    std::vector<glm::mat4> point_data;
    point_data.reserve(POINT_COUNT);
    for (unsigned int i = 0; i < POINT_COUNT; ++i)
    {
        glm::vec3 axis_z = glm::sphericalRand(1.0f);
        glm::vec3 axis_x = glm::normalize(glm::cross(axis_z, glm::sphericalRand(1.0f)));
        glm::vec3 axis_y = glm::cross(axis_z, axis_x);
        point_data.push_back(glm::mat4(glm::vec4(axis_x, 0.0f),
                                       glm::vec4(axis_y, 0.0f),
                                       glm::vec4(axis_z, 0.0f),
                                       glm::vec4(40.0f * glm::sphericalRand(1.0f), 1.0f)));
    };
 
    GLuint vao_id, vbo_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, POINT_COUNT * sizeof(glm::mat4), glm::value_ptr(point_data[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 64, (void*)(0));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 64, (void*)(16));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 64, (void*)(32));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 64, (void*)(48));

    // ==================================================================================================================================================================================================================
    // Camera, view_matrix and projection_matrix initialization
    // ==================================================================================================================================================================================================================
    glClearColor(0.01f, 0.0f, 0.08f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    // ==================================================================================================================================================================================================================
    // The main loop
    // ==================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint cube_texture_id = image::png::texture2d("../../../resources/plato_tex2d/cube.png");
    glBindTexture(GL_TEXTURE_2D, cube_texture_id);
    
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = 0.05 * window.frame_ts;
        glm::vec4 light_direction = glm::vec4(glm::cos(time), glm::sin(time), 0.0f, 0.0f);

        uniform_projection_matrix = window.camera.projection_matrix;
        uniform_view_matrix = window.camera.view_matrix;
        uniform_light_direction = light_direction;
        glDrawArrays(GL_POINTS, 0, POINT_COUNT);        

        window.end_frame();
        glfw::poll_events();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}