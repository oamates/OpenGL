//========================================================================================================================================================================================================================
// DEMO 015: Rendering order
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>

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

    bool position_changed = false;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
    }

    //===================================================================================================================================================================================================================
    // mouse handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) { camera.move_forward(frame_dt);   position_changed = true; }
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) { camera.move_backward(frame_dt);  position_changed = true; }
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) { camera.straight_right(frame_dt); position_changed = true; }
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) { camera.straight_left(frame_dt);  position_changed = true; }
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

const int CUBE_SIZE = 0x08;
const int HOLE_SIZE = 0x04;

const int POINT_COUNT = (2 * CUBE_SIZE + 1) * (2 * CUBE_SIZE + 1) * (2 * CUBE_SIZE + 1) - 
                        (2 * HOLE_SIZE + 1) * (2 * HOLE_SIZE + 1) * (2 * HOLE_SIZE + 1);


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

    demo_window_t window("Alpha Blending", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // creating shaders and uniforms
    //===================================================================================================================================================================================================================
    glsl_program_t cubes_program(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/cubes.vs"),
                                 glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/cubes.gs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/cubes.fs"));

    cubes_program.enable();

    uniform_t uniform_projection_matrix = cubes_program["projection_matrix"];
    uniform_t uniform_view_matrix       = cubes_program["view_matrix"];      
    uniform_t uniform_texture_sampler   = cubes_program["texture_sampler"];  
    uniform_t uniform_global_time       = cubes_program["global_time"];      

    //===================================================================================================================================================================================================================
    // point data initialization 
    //===================================================================================================================================================================================================================

    GLuint vao_id, vbo_id, ibo_id;
    
    std::vector<glm::mat4> points;
    std::vector<GLushort> indices;
    points.reserve(POINT_COUNT);
    indices.reserve(POINT_COUNT);

    GLushort index = 0;
    for (int i = -CUBE_SIZE; i <= CUBE_SIZE; ++i)
    for (int j = -CUBE_SIZE; j <= CUBE_SIZE; ++j)
    for (int k = -CUBE_SIZE; k <= CUBE_SIZE; ++k)
    {
        if ((abs(i) > HOLE_SIZE) || (abs(j) > HOLE_SIZE) || (abs(k) > HOLE_SIZE))
        {
            glm::vec3 axis_z = glm::sphericalRand(1.0f);
            glm::vec3 axis_x = glm::normalize(glm::cross(axis_z, glm::sphericalRand(1.0f)));
            glm::vec3 axis_y = glm::cross(axis_z, axis_x);
            points.push_back(glm::mat4(glm::vec4(axis_x, 0.0f),
                                       glm::vec4(axis_y, 0.0f),
                                       glm::vec4(axis_z, 0.0f),
                                       glm::vec4(6.0f * glm::vec3(i, j, k), 1.0f)));
            indices.push_back(index++);
        }
    }
 

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, POINT_COUNT * sizeof(glm::mat4), glm::value_ptr(points[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 64, (void*)(0));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 64, (void*)(16));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 64, (void*)(32));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 64, (void*)(48));


    glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, POINT_COUNT * sizeof(GLushort), &indices[0], GL_DYNAMIC_DRAW);


    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST enabled, GL_LESS - accept fragment if it closer to the camera than the former one
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.00f, 0.05f, 1.0f);                                                                                // dark blue background
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);


    unsigned int frame = 0;
    double startup_time = glfwGetTime();                                                                                    // set the time uniform
    uniform_global_time = (float) startup_time;

    glActiveTexture(GL_TEXTURE0);
    GLuint cube_texture_id = image::png::texture2d("../../../resources/plato_tex2d/cube.png");
    glBindTexture(GL_TEXTURE_2D, cube_texture_id);
    uniform_texture_sampler = 0;                                                                                            // set our "texture_sampler" to use texture unit 0

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        float angle = 0.01 * time;
        glm::vec4 light_direction = glm::vec4(cos(angle), sin(angle), 0.0f, 0.0f);

        uniform_projection_matrix = window.camera.projection_matrix;
        uniform_view_matrix = window.camera.view_matrix;
        glDrawElements(GL_POINTS, POINT_COUNT, GL_UNSIGNED_SHORT, 0);        
        window.swap_buffers();

        uniform_global_time = time;

        if (window.position_changed)
        {
            glm::vec4 position = glm::vec4(window.camera.position(), 1.0f);
            bool index_order_changed = false;
            bool done = false;
            unsigned int iteration = 0;
            while (!done)
            {
                done = true;
                float norm1 = glm::length2(window.camera.view_matrix * points[indices[0]][3]);

                for(unsigned int i = 1; i < POINT_COUNT - iteration; ++i)
                {
                    float norm2 = glm::length2(window.camera.view_matrix * points[indices[i]][3]);
                    if ((norm2 - norm1) > 0.01)
                    {
                        GLushort q = indices[i - 1];
                        indices[i - 1] = indices[i];
                        indices[i] = q;
                        done = false;
                    }
                    else
                        norm1 = norm2;
                    
                }
                index_order_changed |= (!done);
                ++iteration;
            }
            debug_msg("Sorted after %u iterations", iteration);
            if (index_order_changed)
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, POINT_COUNT * sizeof(GLushort), &indices[0], GL_DYNAMIC_DRAW);
            window.position_changed = false;
        }
        window.end_frame();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}