//========================================================================================================================================================================================================================
// DEMO 002 : GL_POINTS primitive
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "gl_info.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "vao.hpp"
#include "vertex.hpp"

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

    demo_window_t window("GL_POINTS primitive", 4, 3, 3, 1920, 1080);

    //===================================================================================================================================================================================================================
    // Shader and uniform variables initialization
    //===================================================================================================================================================================================================================
    glsl_program_t point_sphere(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/sphere.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/sphere.fs"));

    point_sphere.enable();
    uniform_t uni_sphere_pv_matrix = point_sphere["projection_view_matrix"];                            
    uniform_t uni_sphere_spheres   = point_sphere["spheres"];                                                          
    uniform_t uni_sphere_time      = point_sphere["time"];                                                                

    glsl_program_t point_cube(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/cube.vs"),
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/cube.fs"));
    point_cube.enable();
    uniform_t uni_cube_pv_matrix = point_cube["projection_view_matrix"];                                                
    uniform_t uni_cube_time      = point_cube["time"];

    //===================================================================================================================================================================================================================
    // Point data initialization 
    //===================================================================================================================================================================================================================
    const unsigned int SPHERE_COUNT = 0x80;
    const unsigned int POINT_COUNT = 0x2000;
    const float box_size = 12.0f;
    const float inv_size = 1.0f / box_size;

    struct 
    {
        glm::vec3 center;
        float radius;
    }
    spheres[SPHERE_COUNT];

    struct
    {
        glm::vec3 direction;
        float speed;
    }
    velocities[SPHERE_COUNT];

    for(unsigned int i = 0; i < SPHERE_COUNT; ++i)
    {
        spheres[i].center = box_size * glm::vec3(glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f));
        spheres[i].radius = inv_size * glm::min(glm::abs(glm::gaussRand(0.0f, 2.0f * box_size)), 4.0f);
        velocities[i].direction = glm::sphericalRand(1.0f); 
        velocities[i].speed = glm::abs(glm::gaussRand(4.0f, 2.0f));
    }

    vertex_pn_t* points = (vertex_pn_t*) malloc(POINT_COUNT * sizeof(vertex_pn_t));
    for (unsigned int i = 0; i < POINT_COUNT; ++i)
    {
        points[i].position = glm::sphericalRand(1.0f);
        points[i].normal = glm::normalize(glm::cross(glm::sphericalRand(1.0f), points[i].position));
    }
    free(points);
 
    //===================================================================================================================================================================================================================
    // OpenGL buffers creation
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    vbo_t vbo;
    vbo.init(points, POINT_COUNT);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * BLENDING enabled
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.0f, 0.08f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    point_cube["box_size"] = box_size;            

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        //===============================================================================================================================================================================================================
        // clear back buffer, process events and update timer
        //===============================================================================================================================================================================================================
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();        
        float time = window.frame_ts;
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        //===============================================================================================================================================================================================================
        // render point spheres
        //===============================================================================================================================================================================================================
        glDepthFunc(GL_ALWAYS);
        point_sphere.enable();
        uni_sphere_pv_matrix = projection_view_matrix;
        uni_sphere_time = time;
        glUniform4fv(uni_sphere_spheres, SPHERE_COUNT, (float*) &spheres);
        vbo.instanced_render(GL_POINTS, SPHERE_COUNT);

        //===============================================================================================================================================================================================================
        // render point cube
        //===============================================================================================================================================================================================================
        glDepthFunc(GL_LESS);
        point_cube.enable();
        uni_cube_pv_matrix = projection_view_matrix;
        uni_cube_time = time;
        vbo.instanced_render(GL_POINTS, 32);

        //===============================================================================================================================================================================================================
        // update velocities and positions
        //===============================================================================================================================================================================================================
        float dt = window.frame_dt;

        for(unsigned int i = 1; i < SPHERE_COUNT; ++i)
        {
            for(unsigned int j = 0; j < i; ++j)
            {
                glm::vec3 delta = spheres[j].center - spheres[i].center;
                float l = glm::length(delta); 
                if (l < spheres[i].radius + spheres[j].radius)
                {
                    glm::vec3 n = delta / l;
                    velocities[i].direction = glm::reflect(-velocities[i].direction, n);    
                    velocities[j].direction = glm::reflect(-velocities[j].direction, n);
                }
            }
        }

        for(unsigned int i = 0; i < SPHERE_COUNT; ++i)
        {
            velocities[i].direction = glm::normalize(velocities[i].direction);
            float bound = box_size - spheres[i].radius;

            if (spheres[i].center.x >  bound) velocities[i].direction.x = -glm::abs(velocities[i].direction.x);
            if (spheres[i].center.x < -bound) velocities[i].direction.x =  glm::abs(velocities[i].direction.x);
            if (spheres[i].center.y >  bound) velocities[i].direction.y = -glm::abs(velocities[i].direction.y);
            if (spheres[i].center.y < -bound) velocities[i].direction.y =  glm::abs(velocities[i].direction.y);
            if (spheres[i].center.z >  bound) velocities[i].direction.z = -glm::abs(velocities[i].direction.z);
            if (spheres[i].center.z < -bound) velocities[i].direction.z =  glm::abs(velocities[i].direction.z);
        
            spheres[i].center += velocities[i].direction * velocities[i].speed * dt;
        }

        //===============================================================================================================================================================================================================
        // done : increment frame counter and show back buffer
        //===============================================================================================================================================================================================================
        window.end_frame();
    } 

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}