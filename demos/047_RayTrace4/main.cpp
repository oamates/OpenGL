//========================================================================================================================================================================================================================
// DEMO 047 : Raytrace demo
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "gl_aux.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
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
// material, light and geometric structure
//=======================================================================================================================================================================================================================
struct material_t
{
    float kA, kD, kS, kR;       // ambient, diffuse, specular, reflectance
    float shininess;            // shininess for specular reflex
    glm::vec3 color;
};

struct sphere_t
{
    glm::vec3 position;
    float radius;
    material_t material;
};

void transform(sphere_t& sphere, const glm::mat4& m)
{
    sphere.position = glm::vec3(m * glm::vec4(sphere.position, 1.0f));
}

struct plane_t
{
    glm::vec3 normal;
    float d;
    material_t material;
};

void transform(plane_t& plane, const glm::mat4& m)
{
    glm::vec4 n = glm::vec4(plane.normal, plane.d);
    n = glm::inverse(glm::transpose(m)) * n;
    plane.normal = glm::vec3(n);
    plane.d = n.w;
}

struct cylinder_t
{
    glm::vec3 position;
    float a;
    material_t material;
};

void transform(cylinder_t& cylinder, const glm::mat4& m)
{
    cylinder.position = glm::vec3(m * glm::vec4(cylinder.position, 1.0f));
}

struct pointlight_t
{
    glm::vec3 position;
    float k;                        // intensity
    float falloff;
};

void transform(pointlight_t& pointlight, const glm::mat4& m)
{
    pointlight.position = glm::vec3(m * glm::vec4(pointlight.position, 1.0f));
}


//=======================================================================================================================================================================================================================
// geometric objects: spheres, planes and cylinders
//=======================================================================================================================================================================================================================
const int NUM_SPHERES = 2;          
sphere_t sphere[NUM_SPHERES] =
{
    {
        .position = glm::vec3(0.0f, -2.0f, -12.0f),
        .radius = 3.0f,
        .material = 
        {
            .kA = 0.1f,
            .kD = 0.8f,
            .kS = 0.02f,
            .kR = 0.6f,
            .shininess = 1.5f,
            .color = glm::vec3(0.7f, 0.3f, 0.3f)
        }
    },
    {
        .position = glm::vec3(0.0f, 0.0f, 0.0f),
        .radius = 5.0f,
        .material = 
        {
            .kA = 0.1f,
            .kD = 0.0f,
            .kS = 0.01f,
            .kR = 0.2f,
            .shininess = 1.0f,
            .color = glm::vec3(0.0f, 1.0f, 0.3f)
        }
    }
};

const int NUM_PLANES = 2;
plane_t plane[NUM_PLANES] =
{
    {
        .normal = glm::vec3(0.0f, 1.0f, 0.0f),
        .d = 5.0f,
        .material = 
        {
            .kA = 0.3f,
            .kD = 0.5f,
            .kS = 0.0f,
            .kR = 0.0f,
            .shininess = 1.0f,
            .color = glm::vec3(0.7f, 0.7f, 0.7f)
        }            
    },
    {
        .normal = glm::vec3(0.0f, 0.0f, 1.0f),
        .d = 17.0f,
        .material = 
        {
            .kA = 0.3f,
            .kD = 0.6f,
            .kS = 0.0f,
            .kR = 0.0f,
            .shininess = 1.0f,
            .color = glm::vec3(0.4f, 0.3f, 0.7f)
        }            
    }
};

const int NUM_CYLINDERS = 1;
cylinder_t cylinder[NUM_CYLINDERS] = 
{
    {
        .position = glm::vec3(-9.0f, 0.0f, 15.0f),
        .a = 3.0f,
        .material = 
        {
            .kA = 0.4f,
            .kD = 0.6f,
            .kS = 0.0f,
            .kR = 0.3f,
            .shininess = 1.0f,
            .color = glm::vec3(0.4f, 0.6f, 0.5f)
        }            
    }
};


//=======================================================================================================================================================================================================================
// light objects
//=======================================================================================================================================================================================================================
const int NUM_LIGHTS = 2;    
pointlight_t pointlight[NUM_LIGHTS] = 
{
    {
        .position = glm::vec3(-4.0f, 10.0f, 10.0f),
        .k = 0.5f,
        .falloff = 0.005f
    },
    {
        .position = glm::vec3(8.0f, 10.0f, -10.0f),
        .k = 0.5f,
        .falloff = 0.01f
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

    demo_window_t window("Raytrace demo", 4, 3, 3, 1920, 1080);

    //===================================================================================================================================================================================================================
    // Raytrace shader and uniform variables initialization
    //===================================================================================================================================================================================================================
    glsl_program_t raytracer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/raytracer.vs"),
                             glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/raytracer.fs"));
    raytracer.enable();

    uniform_t uni_camera_matrix = raytracer["camera_matrix"];
    uniform_t uni_camera_ws     = raytracer["camera_ws"];
    uniform_t uni_focal_scale   = raytracer["focal_scale"];

    uniform_t uni_sphere        = raytracer["sphere[0].position"];
    uniform_t uni_plane         = raytracer["plane[0].normal"];
    uniform_t uni_cylinder      = raytracer["cylinder[0].position"];
    uniform_t uni_pointlight    = raytracer["pointlight[0].position"];

    glm::vec2 focal_scale = glm::vec2(1.0f / window.camera.projection_matrix[0][0], 1.0f / window.camera.projection_matrix[1][1]);
    uni_focal_scale = focal_scale;

    //===================================================================================================================================================================================================================
    // Fake VAO for full screen quad initialization
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view_matrix = window.camera.view_matrix;
        glm::mat3 camera_matrix = glm::inverse(glm::mat3(view_matrix));
        glm::vec3 camera_ws = -camera_matrix * glm::vec3(view_matrix[3]);

        uni_camera_matrix = camera_matrix;
        uni_camera_ws = camera_ws;

        for(int i = 0; i < NUM_SPHERES;   i++) transform(sphere[i],     view_matrix);
        for(int i = 0; i < NUM_CYLINDERS; i++) transform(cylinder[i],   view_matrix);
        for(int i = 0; i < NUM_PLANES;    i++) transform(plane[i],      view_matrix);
        for(int i = 0; i < NUM_LIGHTS;    i++) transform(pointlight[i], view_matrix);

        glUniform1fv(uni_sphere.location,     sizeof(sphere) / sizeof(float),     (const GLfloat*) sphere);
        glUniform1fv(uni_plane.location,      sizeof(plane) / sizeof(float),      (const GLfloat*) plane);
        glUniform1fv(uni_cylinder.location,   sizeof(cylinder) / sizeof(float),   (const GLfloat*) cylinder);
        glUniform1fv(uni_pointlight.location, sizeof(pointlight) / sizeof(float), (const GLfloat*) pointlight);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // done, terminate the program
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}