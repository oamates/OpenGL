//========================================================================================================================================================================================================================
// DEMO 083 : CMS algorithm
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>


#include <iostream>
#include <vector>
#include <math.h>

#include "isosurface.hpp"
#include "algcms.hpp"
#include "vec3.hpp"
#include "mesh.hpp"

struct ExampleClass
{
    // Sphere function
    float sphereFunction(float x, float y, float z) const
    {
        return x * x + y * y + z * z - 1.0f;
    }

    // Cube function
    float cubeFunction(float x, float y, float z) const
    {
        return std::max(fabs(x) - 1.0f, std::max((fabs(y) - 1.0f), fabs(z) - 1.0f));
    }

    // Cone function
    float coneFunction(float x, float y, float z) const
    {
        return 10.0f * (x * x + y * y) - (z - 1.0f) * (z - 1.0f);
    }

    // Anti-tank-like function
    float antiTankFunction(float x, float y, float z) const
    {
        return x * x * y * y + x * x * z * z + y * y * z * z - 0.01f;
    }

    // Heart function
    float heartFunction(float x, float y, float z) const
    {
        return pow(x * x + y * y + 2.0f * z * z - 0.5f, 3.0) - y * y * y * (x * x - 0.01f * z * z);
    }

    // Torus function
    float torusFunction(float x, float y, float z) const
    {
        float R = 0.45f;
        float r = 0.2f;
        float x0 = x - 0.25f;
        float q = x0 * x0 + y * y + z * z + R * R - r * r;

        return q * q - 4.0f * R * R * (z * z + x0 * x0);
    }

    // Double torus function
    float doubleTorusFunction(float x, float y, float z) const
    {
        float x2 = x * x;
        float x3 = x2 * x;
        float x4 = x2 * x2;
        float y2 = y * y;
        return -(0.01f - x4 + 2.0f * x3 * x3 - x4 * x4 + 2.0f * x2 * y2 - 2.0f * x4 * y2 - y2 * y2 - z * z);
    }

    // Interlinked torii function
    float linkedToriiFunction(float x, float y, float z) const
    {
        float R = 0.45f;
        float r = 0.2f;
        float x0 = x - 0.25f;
        float x1 = x + 0.25f;
        float q0 = x0 * x0 + y * y + z * z + R * R - r * r;
        float q1 = x1 * x1 + y * y + z * z + R * R - r * r;
        return (q0 * q0 - 4.0f * R * R * (x0 * x0 + z * z)) * (q1 * q1 - 4.0f * R * R * (x1 * x1 + y * y));
    }

    glm::dvec3 tri(const glm::dvec3& x) const
    {
        glm::dvec3 q = glm::abs(glm::fract(x) - glm::dvec3(0.5));
        return glm::clamp(q, 0.05, 0.45);
    }

    double sdf(float x, float y, float z) const
    {
        glm::dvec3 p = glm::dvec3(x, y, z);
        glm::dvec3 pp = 16.0 * p;
        glm::dvec3 op = tri(1.1 * pp + tri(1.1 * glm::dvec3(pp.z, pp.x, pp.y)));
        glm::dvec3 q = pp + (op - glm::dvec3(0.25)) * 0.3;
        q = glm::cos(0.444 * q + glm::sin(1.112 * glm::dvec3(pp.z, pp.x, pp.y)));
        return glm::length(q) - 1.05;
    }

    // ============================================================================================================================================================================================================================
    float operator()(float x, float y, float z) const
    {
//        return torusFunction(x, y, z);
        return antiTankFunction(x, y, z);
    }

    glm::vec3 gradient(const glm::vec3& p) const
    {
        float x = p.x; 
        float y = p.y;
        float z = p.z;
        const float gradient_delta = 0.06125;

        float f100 = (*this)(x + gradient_delta, y - gradient_delta, z - gradient_delta);
        float f001 = (*this)(x - gradient_delta, y - gradient_delta, z + gradient_delta);
        float f010 = (*this)(x - gradient_delta, y + gradient_delta, z - gradient_delta);  
        float f111 = (*this)(x + gradient_delta, y + gradient_delta, z + gradient_delta);

        return glm::normalize(glm::vec3( f100 - f001 - f010 + f111, 
                                        -f100 - f001 + f010 + f111, 
                                        -f100 + f001 - f010 + f111));
    }

};


static float BBOX_SIZE                  = 2.0f;
static int MIN_OCTREE_RES               = 2;
static int MAX_OCTREE_RES               = 8;
static float COMPLEX_SURFACE_THRESHOLD  = 0.85f;

int ADDRESS_SIZE = MAX_OCTREE_RES; // To be used by some of the classes


#include "log.hpp"
#include "gl_info.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "isosurface.hpp"
#include "image.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;
    bool wireframe_mode = false;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
    }

    //===================================================================================================================================================================================================================
    // mouse handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
            wireframe_mode = !wireframe_mode;

        glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);
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
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("CPU Marching cubes algorithm", 4, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Load trilinear blend shader which produces nice 3-dimensional material texture from arbitrary 2 dimensional input
    //===================================================================================================================================================================================================================
    glsl_program_t trilinear_blend(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/tb.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/tb.fs"));

    trilinear_blend.enable();
    uniform_t uniform_model_matrix           = trilinear_blend["model_matrix"];
    uniform_t uniform_projection_view_matrix = trilinear_blend["projection_view_matrix"];
    uniform_t uniform_camera_ws              = trilinear_blend["camera_ws"];
    uniform_t uniform_light_ws               = trilinear_blend["light_ws"];
    uniform_t uniform_Ka                     = trilinear_blend["Ka"];
    uniform_t uniform_Kd                     = trilinear_blend["Kd"];
    uniform_t uniform_Ks                     = trilinear_blend["Ks"];
    uniform_t uniform_Ns                     = trilinear_blend["Ns"];
    uniform_t uniform_bf                     = trilinear_blend["bf"];
    trilinear_blend["tb_tex2d"] = 0;

    glActiveTexture(GL_TEXTURE0);
    GLuint stone_tex = image::png::texture2d("../../../resources/tex2d/moss.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);


    //===================================================================================================================================================================================================================
    // light variables
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);


    //===================================================================================================================================================================================================================
    // run CMS algorithm and generate iso-surface
    //===================================================================================================================================================================================================================
    ExampleClass t;

    cms::Isosurface_t<ExampleClass> iso(&t);

    float halfSize = 0.5f * BBOX_SIZE;

    cms::Range container[3] = 
    {
        cms::Range(-halfSize, halfSize),
        cms::Range(-halfSize, halfSize),
        cms::Range(-halfSize, halfSize)
    };

    cms::AlgCMS cmsAlg(&iso, container, MIN_OCTREE_RES, MAX_OCTREE_RES);
    cmsAlg.setComplexSurfThresh(COMPLEX_SURFACE_THRESHOLD);                 // Set the complex surface threshold

    cms::mesh_t mesh;
    cmsAlg.extractSurface(mesh);                                            // Proceed to extract the surface <runs the algorithm>

    GLuint vao_id, vbo_id, nbo_id, ibo_id;

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(glm::vec3), mesh.vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    for (int i = 0; i < mesh.vertices.size(); ++i)
    {
        glm::vec3 g = t.gradient(mesh.vertices[i]);
        mesh.normals.push_back(g);
    }

    glGenBuffers(1, &nbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, nbo_id);
    glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(glm::vec3), mesh.normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(GLuint), mesh.indices.data(), GL_STATIC_DRAW);

    debug_msg("mesh.m_vertices.size() = %u", (unsigned int) mesh.vertices.size());
    debug_msg("mesh.m_normals.size() = %u", (unsigned int) mesh.normals.size());
    debug_msg("mesh.m_indices.size() = %u", (unsigned int) mesh.indices.size());



    //===================================================================================================================================================================================================================
    // main program loop : just clear the buffer in a loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        //===============================================================================================================================================================================================================
        // clear back buffer, process events and update timer
        //===============================================================================================================================================================================================================
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        float time = glfw::time();
        const float light_radius = 12.5f;
        glm::vec3 light_ws = glm::vec3(light_radius * cos(0.5f * time), 25.0f, light_radius * sin(0.5f * time));
        glm::vec3 camera_ws = window.camera.position();

        //===============================================================================================================================================================================================================
        // Render the output of marching cubes algorithm
        //===============================================================================================================================================================================================================
        uniform_projection_view_matrix = projection_view_matrix;
        uniform_light_ws = light_ws;
        uniform_camera_ws = camera_ws;

        uniform_model_matrix = glm::mat4(1.0f);
        uniform_Ka = glm::vec3(0.15f);
        uniform_Kd = glm::vec3(0.77f);
        uniform_Ks = glm::vec3(0.33f);
        uniform_Ns = 20.0f;
        uniform_bf = 0.2875f;

        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}