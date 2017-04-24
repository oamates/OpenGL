//========================================================================================================================================================================================================================
// DEMO 023: Cubemap Reflections
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/transform.hpp> 
#include <glm/gtx/rotate_vector.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "plato.hpp"
#include "polyhedron.hpp"
#include "surface.hpp"
#include "vao.hpp"
#include "solid.hpp"

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

const int CUBE_SIZE = 0x05;
const int HOLE_SIZE = 0x03;

const int POINT_COUNT = (2 * CUBE_SIZE + 1) * (2 * CUBE_SIZE + 1) * (2 * CUBE_SIZE + 1) - 
                        (2 * HOLE_SIZE + 1) * (2 * HOLE_SIZE + 1) * (2 * HOLE_SIZE + 1);

const int TEXTURE_RESOLUTION = 1536;

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

    demo_window_t window("Cubemap Reflections", 8, 3, 3, 1920, 1080, true);

    glClearColor(0.00f, 0.00f, 0.00f, 1.00f);                                                                   // dark blue background

    // ==================================================================================================================================================================================================================
    // Layered rendering shader
    // ==================================================================================================================================================================================================================

    glsl_program_t layered_cubes(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/layered.vs"),
                                 glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/layered.gs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/layered.fs"));

    layered_cubes.enable();
    layered_cubes["texture_sampler"] = 0;                                                                       // set our "texture_sampler" to use texture unit 0

    // ==================================================================================================================================================================================================================
    // Load texture for cube faces
    // ==================================================================================================================================================================================================================

    uniform_t layered_global_time = layered_cubes["global_time"];                                                  // time uniform

    // ==================================================================================================================================================================================================================
    // Creating shaders and uniforms
    // ==================================================================================================================================================================================================================

    glsl_program_t cubes_program(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/cubes.vs"),
                                 glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/cubes.gs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/cubes.fs"));

    cubes_program.enable();

    uniform_t uniform_view_matrix = cubes_program["view_matrix"];
    uniform_t uniform_global_time = cubes_program["global_time"];

    cubes_program["projection_matrix"] = window.camera.projection_matrix;                                       // set up projection matrix, it is not going to change
    cubes_program["texture_sampler"] = 0;                                                                       // set our "texture_sampler" to use texture unit 0

    GLuint cube_texture_id = image::png::texture2d("../../../resources/tex2d/cube.png");

    // ==================================================================================================================================================================================================================
    // Point data initialization 
    // ==================================================================================================================================================================================================================

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
        };
    };
 

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

    // ==================================================================================================================================================================================================================
    // Simple cube VAO generated by hands, no geometry shader
    // ==================================================================================================================================================================================================================

    const unsigned int mesh_size = 36;
    GLuint cube_vao_id;
    GLuint cube_vbo_id, cube_nbo_id, cube_tbo_id;

    glGenVertexArrays(1, &cube_vao_id);
    glBindVertexArray(cube_vao_id);
    const float cube_size = 10.0f;
    const glm::vec3 vertex[] = 
    {
        glm::vec3(-cube_size, -cube_size, -cube_size),
        glm::vec3( cube_size, -cube_size, -cube_size),
        glm::vec3(-cube_size,  cube_size, -cube_size),
        glm::vec3( cube_size,  cube_size, -cube_size),
        glm::vec3(-cube_size, -cube_size,  cube_size),
        glm::vec3( cube_size, -cube_size,  cube_size),
        glm::vec3(-cube_size,  cube_size,  cube_size),
        glm::vec3( cube_size,  cube_size,  cube_size)
    };

    const glm::vec3 triangulation[] = 
    {
        vertex[0], vertex[2], vertex[3], vertex[0], vertex[3], vertex[1],                                                   // faces parallel to xy plane : the face [0231] and ...
        vertex[4], vertex[5], vertex[7], vertex[4], vertex[7], vertex[6],                                                   // ... the face [4576]
        vertex[0], vertex[4], vertex[6], vertex[0], vertex[6], vertex[2],                                                   // faces parallel to yz plane : the face [0462] and ...
        vertex[1], vertex[3], vertex[7], vertex[1], vertex[7], vertex[5],                                                   // ... the face [1375]
        vertex[0], vertex[1], vertex[5], vertex[0], vertex[5], vertex[4],                                                   // faces parallel to zx plane : the face [0154] and ...
        vertex[2], vertex[6], vertex[7], vertex[2], vertex[7], vertex[3]                                                    // ... the face [2673]
    };   

    glEnableVertexAttribArray(0);
    glGenBuffers(1, &cube_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangulation), glm::value_ptr(triangulation[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    const glm::vec3 normal[] = 
    { 
        glm::vec3 ( 1.0f,  0.0f,  0.0f),
        glm::vec3 (-1.0f,  0.0f,  0.0f),
        glm::vec3 ( 0.0f,  1.0f,  0.0f),
        glm::vec3 ( 0.0f, -1.0f,  0.0f),
        glm::vec3 ( 0.0f,  0.0f,  1.0f),
        glm::vec3 ( 0.0f,  0.0f, -1.0f)
    };

    const glm::vec3 normal_data[] = 
    {                                                                                                                       
        normal[5], normal[5], normal[5], normal[5], normal[5], normal[5],                                                   // faces parallel to xy plane : the face [0231] and ...            
        normal[4], normal[4], normal[4], normal[4], normal[4], normal[4],                                                   // ... the face [4576]                                             
        normal[1], normal[1], normal[1], normal[1], normal[1], normal[1],                                                   // faces parallel to yz plane : the face [0462] and ...            
        normal[0], normal[0], normal[0], normal[0], normal[0], normal[0],                                                   // ... the face [1375]                                             
        normal[3], normal[3], normal[3], normal[3], normal[3], normal[3],                                                   // faces parallel to zx plane : the face [0154] and ...            
        normal[2], normal[2], normal[2], normal[2], normal[2], normal[2]                                                    // ... the face [2673]                                             
    };   

    glEnableVertexAttribArray(1);
    glGenBuffers(1, &cube_nbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, cube_nbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normal_data), glm::value_ptr(normal_data[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    const glm::vec2 tc[] = 
    { 
        glm::vec2 (0.0f, 0.0f),
        glm::vec2 (1.0f, 0.0f),
        glm::vec2 (1.0f, 1.0f),
        glm::vec2 (0.0f, 1.0f) 
    };

    const glm::vec2 texture_coords[] = 
    {
        tc[0], tc[1], tc[2], tc[0], tc[2], tc[3],
        tc[0], tc[1], tc[2], tc[0], tc[2], tc[3],
        tc[0], tc[1], tc[2], tc[0], tc[2], tc[3],
        tc[0], tc[1], tc[2], tc[0], tc[2], tc[3],
        tc[0], tc[1], tc[2], tc[0], tc[2], tc[3],
        tc[0], tc[1], tc[2], tc[0], tc[2], tc[3]
    };   

    glEnableVertexAttribArray(2);
    glGenBuffers(1, &cube_tbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, cube_tbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coords), glm::value_ptr(texture_coords[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glsl_program_t simple_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/simple_light.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/simple_light.fs"));

    simple_light.enable();

    uniform_t simple_light_view_matrix = simple_light["view_matrix"];

    simple_light["projection_matrix"] = window.camera.projection_matrix;
    simple_light["texture_sampler"] = 0;


    // ==================================================================================================================================================================================================================
    // Creating additional framebuffer for rendering to texture
    // ==================================================================================================================================================================================================================

    GLuint texture_fbo_id;                                                                                                  // framebuffer object for rendering to texture
    glGenFramebuffers(1, &texture_fbo_id);                                                                                  // id = 0 corresponds to rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, texture_fbo_id);                                                                      // make it current
    
    GLuint render_texture_id;                                                                                               // The texture we're going to render to
    glGenTextures(1, &render_texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, render_texture_id);                                                                  // bind the newly created texture : all future texture functions will modify this texture
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB, TEXTURE_RESOLUTION, TEXTURE_RESOLUTION);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, render_texture_id, 0);
    GLenum draw_buffers_list[] = {GL_COLOR_ATTACHMENT0};                                                                    // list of the draw buffers
    glDrawBuffers(1, draw_buffers_list);                                                                                    // 1 is the size of DrawBuffers

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)                                                // check that the created framebuffer object is ok
        exit_msg("Could not initialize GL_FRAMEBUFFER object.");

    // ==================================================================================================================================================================================================================
    // The main loop
    // ==================================================================================================================================================================================================================

    while(!window.should_close())
    {
        window.new_frame();
        float global_time = glfwGetTime();

        // ==============================================================================================================================================================================================================
        // Rendering the main scene to the off-screen frame buffer 
        // ==============================================================================================================================================================================================================

        glBindFramebuffer(GL_FRAMEBUFFER, texture_fbo_id);
        glViewport(0, 0, TEXTURE_RESOLUTION, TEXTURE_RESOLUTION);                                                           // Render on the whole framebuffer, complete from the lower left corner to the upper right
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
 //       glDisable(GL_DEPTH_TEST);
//        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

        layered_cubes.enable();
        layered_global_time = global_time;
        glBindVertexArray(vao_id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cube_texture_id);

        glDrawElements(GL_POINTS, POINT_COUNT, GL_UNSIGNED_SHORT, 0);        

        // ==============================================================================================================================================================================================================
        // Rendering central cube using created texture
        // ==============================================================================================================================================================================================================
  
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window.res_x, window.res_y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        simple_light.enable();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, render_texture_id);

        glBindVertexArray(cube_vao_id);
        simple_light_view_matrix = window.camera.view_matrix;
        glDrawArrays(GL_TRIANGLES, 0, mesh_size);

        // ==============================================================================================================================================================================================================
        // Rendering the main scene to the screen
        // ==============================================================================================================================================================================================================

        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

        cubes_program.enable();
        glBindVertexArray(vao_id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cube_texture_id);
        uniform_global_time = global_time;
        uniform_view_matrix = window.camera.view_matrix;                                   

        glDrawElements(GL_POINTS, POINT_COUNT, GL_UNSIGNED_SHORT, 0);        

/*
        if (position_changed)
        {
            glm::vec4 position = view_matrix[3];
            bool index_order_changed = false;
            bool done = false;
            unsigned int iteration = 0;
            while (!done)
            {
                done = true;
                float norm1 = glm::length2(view_matrix * points[indices[0]][3]);

                for(unsigned int i = 1; i < POINT_COUNT - iteration; ++i)
                {
                    float norm2 = glm::length2(view_matrix * points[indices[i]][3]);
                    if ((norm2 - norm1) > 0.01)
                    {
                        GLushort q = indices[i - 1];
                        indices[i - 1] = indices[i];
                        indices[i] = q;
                        done = false;
                    }
                    else
                        norm1 = norm2;
                    
                };
                index_order_changed |= (!done);
                ++iteration;
            };
            debug_msg("Sorted after %u iterations", iteration);
            if (index_order_changed)
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, POINT_COUNT * sizeof(GLushort), &indices[0], GL_DYNAMIC_DRAW);
            position_changed = false;
        };
*/
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}