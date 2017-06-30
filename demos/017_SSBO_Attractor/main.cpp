//========================================================================================================================================================================================================================
// DEMO 017: SSBO Attractor
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

const unsigned int POINT_COUNT = 0x400000;

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("SSBO Attractor", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // skybox rendering shader program compilation and uniform id setup
    //===================================================================================================================================================================================================================
    glsl_program_t skybox(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/skybox.vs"),
                          glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/skybox.fs"));
    skybox.enable();
    uniform_t skybox_projection_view_matrix = skybox["projection_view_matrix"];
    uniform_t skybox_global_time            = skybox["global_time"];

    //===================================================================================================================================================================================================================
    // point rendering shader program compilation and uniform id setup
    //===================================================================================================================================================================================================================
    glsl_program_t point_lighting(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/point.vs"),
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/point.fs"));
    point_lighting.enable();
    uniform_t uniform_projection_view_matrix = point_lighting["projection_view_matrix"];
    uniform_t uniform_global_time            = point_lighting["global_time"];           

    //===================================================================================================================================================================================================================
    // point data initialization and filling GL_SHADER_STORAGE_BUFFER
    //===================================================================================================================================================================================================================
    glm::vec3* data = (glm::vec3*) malloc(POINT_COUNT * sizeof(glm::vec3));
    for (int i = 0; i < POINT_COUNT; ++i) data[i] = glm::sphericalRand(2.0f);
    GLuint ssbo_id;
    glGenBuffers(1, &ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ssbo_id, 0, POINT_COUNT * sizeof(glm::vec3));
    glBufferData(GL_SHADER_STORAGE_BUFFER, POINT_COUNT * sizeof(glm::vec3), data, GL_DYNAMIC_COPY);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST enabled, GL_LESS - accept fragment if it closer to the camera than the former one
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.0f, 0.08f, 1.0f);                                                                                 // dark blue background
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);                                                                                                   // accept fragment if it closer to the camera than the former one
    glCullFace(GL_FRONT);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);    
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    //===================================================================================================================================================================================================================
    // simple cube mesh to render universe skybox : create and fill buffers
    //===================================================================================================================================================================================================================
    const glm::vec3 cube_vertices[] = 
    {
        glm::vec3(-1.0f, -1.0f, -1.0f),
        glm::vec3( 1.0f, -1.0f, -1.0f),
        glm::vec3(-1.0f,  1.0f, -1.0f),
        glm::vec3( 1.0f,  1.0f, -1.0f),
        glm::vec3(-1.0f, -1.0f,  1.0f),
        glm::vec3( 1.0f, -1.0f,  1.0f),
        glm::vec3(-1.0f,  1.0f,  1.0f),
        glm::vec3( 1.0f,  1.0f,  1.0f)
    };

    const GLubyte cube_indices[] = 
    {
        0, 2, 3, 0, 3, 1,
        4, 5, 7, 4, 7, 6,
        0, 4, 6, 0, 6, 2,
        1, 3, 7, 1, 7, 5,
        0, 1, 5, 0, 5, 4,
        2, 6, 7, 2, 7, 3
    };   

    GLuint cube_vao_id, cube_vbo_id, cube_ibo_id;
    glGenVertexArrays(1, &cube_vao_id);
    glBindVertexArray(cube_vao_id);

    glGenBuffers(1, &cube_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), glm::value_ptr(cube_vertices[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &cube_ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    const char * galaxy_files[6] = {"../../../resources/cubemap/galaxy/positive_x.png",
                                    "../../../resources/cubemap/galaxy/negative_x.png",
                                    "../../../resources/cubemap/galaxy/positive_y.png",
                                    "../../../resources/cubemap/galaxy/negative_y.png",
                                    "../../../resources/cubemap/galaxy/positive_z.png",
                                    "../../../resources/cubemap/galaxy/negative_z.png"};
    glActiveTexture(GL_TEXTURE0);
    GLuint galaxy_cubemap = image::png::cubemap(galaxy_files);

    //===================================================================================================================================================================================================================
    // the main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        skybox.enable();
        skybox_global_time = time;
        skybox_projection_view_matrix = projection_view_matrix;     
        glBindVertexArray(cube_vao_id);
        glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLubyte), GL_UNSIGNED_BYTE, 0);

        point_lighting.enable();
        uniform_global_time = time;
        uniform_projection_view_matrix = projection_view_matrix;
        glDrawArrays(GL_POINTS, 0, POINT_COUNT);        

        window.end_frame();
    }
    
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}