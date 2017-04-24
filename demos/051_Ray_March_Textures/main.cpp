//========================================================================================================================================================================================================================
// DEMO 051 : RayMarch Textures
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

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "plato.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "polyhedron.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;
    glm::vec3 mouse_pos;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, time */),
          mouse_pos(0.0f, 0.0f, -1.0f)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
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
    }

    void on_mouse_move() override
    {
        mouse_pos.x = mouse.x;
        mouse_pos.y = mouse.y;
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }

    void on_mouse_button(int button, int action, int mods) override
    {
        mouse_pos.z = (action == GLFW_PRESS) ? 1.0f : -1.0f;
    }
};

struct motion3d_t
{
    glm::vec4 shift;
    glm::vec4 rotor;
};

glm::vec3 spheric_inversion(glm::vec4 sphere, glm::vec3 point)
{
    float radius = sphere.w;
    glm::vec3 center = glm::vec3(sphere);
    glm::vec3 ray = point - center;
    return center + (radius * radius * ray / glm::dot(ray, ray)); 
}

glm::vec3 map2(glm::vec3 p)
{    
    p /= sqrt(1.0f + glm::dot(p, p));
    glm::vec4 sphere[24] = 
    {
        glm::vec4( 2.0f,  2.0f,  3.0f, 4.0f),
        glm::vec4(-2.0f,  2.0f,  3.0f, 4.0f),
        glm::vec4( 2.0f, -2.0f,  3.0f, 4.0f),
        glm::vec4(-2.0f, -2.0f,  3.0f, 4.0f),
        glm::vec4( 2.0f,  2.0f, -3.0f, 4.0f),
        glm::vec4(-2.0f,  2.0f, -3.0f, 4.0f),
        glm::vec4( 2.0f, -2.0f, -3.0f, 4.0f),
        glm::vec4(-2.0f, -2.0f, -3.0f, 4.0f),

        glm::vec4( 2.0f,  3.0f,  2.0f, 4.0f),
        glm::vec4(-2.0f,  3.0f,  2.0f, 4.0f),
        glm::vec4( 2.0f,  3.0f, -2.0f, 4.0f),
        glm::vec4(-2.0f,  3.0f, -2.0f, 4.0f),
        glm::vec4( 2.0f, -3.0f,  2.0f, 4.0f),
        glm::vec4(-2.0f, -3.0f,  2.0f, 4.0f),
        glm::vec4( 2.0f, -3.0f, -2.0f, 4.0f),
        glm::vec4(-2.0f, -3.0f, -2.0f, 4.0f),

        glm::vec4( 3.0f,  2.0f,  2.0f, 4.0f),
        glm::vec4( 3.0f, -2.0f,  2.0f, 4.0f),
        glm::vec4( 3.0f,  2.0f, -2.0f, 4.0f),
        glm::vec4( 3.0f, -2.0f, -2.0f, 4.0f),
        glm::vec4(-3.0f,  2.0f,  2.0f, 4.0f),
        glm::vec4(-3.0f, -2.0f,  2.0f, 4.0f),
        glm::vec4(-3.0f,  2.0f, -2.0f, 4.0f),
        glm::vec4(-3.0f, -2.0f, -2.0f, 4.0f)
    };

    for (int i = 0; i < 24; ++i)
    {
        p = spheric_inversion(sphere[i], p);
        p = -glm::vec3(p.z, p.x, p.y);
    }
    return p;
}


//=======================================================================================================================================================================================================================
// function that initializes initial model matrices and object rotation axes
//=======================================================================================================================================================================================================================
void fill_shift_rotor_data(motion3d_t* data, const glm::vec3& group_shift, float cell_size, int N)
{
    float middle = 0.5f * float(N) - 0.5f;
    int index = 0;
    for (int i = 0; i < N; ++i) for (int j = 0; j < N; ++j) for (int k = 0; k < N; ++k)
    {
        data[index].shift = glm::vec4(group_shift + cell_size * glm::vec3(float(i) - middle, float(j) - middle, float(k) - middle), 0.0f);
        data[index].rotor = glm::vec4(glm::sphericalRand(1.0f), 4.0f * glm::gaussRand(0.0f, 1.0f));
        index++;
    }
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("RayMarch Textures", 4, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // phong lighting model shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t simple_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_light.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_light.fs"));

    simple_light.enable();

    uniform_t uniform_projection_view_matrix = simple_light["projection_view_matrix"];
    uniform_t uniform_time                   = simple_light["time"];
    uniform_t uniform_camera_ws              = simple_light["camera_ws"];         
    uniform_t uniform_light_ws               = simple_light["light_ws"];         
    uniform_t uniform_buffer_base            = simple_light["buffer_base"];

    simple_light["solid_scale"] = 3.5f;

    //===================================================================================================================================================================================================================
    // Create plato polyhedra buffers
    //===================================================================================================================================================================================================================
    polyhedron tetrahedron, cube, octahedron, dodecahedron, icosahedron;

    tetrahedron.regular_pft2_vao(4, 4, plato::tetrahedron::vertices, plato::tetrahedron::normals, plato::tetrahedron::faces);
    cube.regular_pft2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces);
    octahedron.regular_pft2_vao(6, 8, plato::octahedron::vertices, plato::octahedron::normals, plato::octahedron::faces);
    dodecahedron.regular_pft2_vao(20, 12, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);
    icosahedron.regular_pft2_vao(12, 20, plato::icosahedron::vertices, plato::icosahedron::normals, plato::icosahedron::faces);

    const int N = 2;
    const int group_size = N * N * N;
    const float cell_size = 8.0f;
    const float origin_distance = 1.25f * cell_size * N;
    const GLsizeiptr chunk_size = group_size * sizeof(motion3d_t);  

    motion3d_t data[5 * group_size];

    fill_shift_rotor_data(&data[0 * group_size], glm::vec3(            0.0f,             0.0f, 0.0f), cell_size, N);
    fill_shift_rotor_data(&data[1 * group_size], glm::vec3(            0.0f,  origin_distance, 0.0f), cell_size, N);
    fill_shift_rotor_data(&data[2 * group_size], glm::vec3(            0.0f, -origin_distance, 0.0f), cell_size, N);
    fill_shift_rotor_data(&data[3 * group_size], glm::vec3( origin_distance,             0.0f, 0.0f), cell_size, N);
    fill_shift_rotor_data(&data[4 * group_size], glm::vec3(-origin_distance,             0.0f, 0.0f), cell_size, N);

    GLuint ssbo_id;
    glGenBuffers(1, &ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ssbo_id, 0, sizeof(data));

    //===================================================================================================================================================================================================================
    // light variables
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    //===================================================================================================================================================================================================================
    // main program loop : just clear the buffer in a loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        glm::vec3 camera_ws = window.camera.position();
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        float time = glfw::time();
        glm::vec3 light_ws = glm::vec3(cell_size * N * glm::cos(time), 0.0f, cell_size * N * glm::sin(time));

        uniform_time = 0.05 * time;
        uniform_light_ws = light_ws;
        uniform_camera_ws = camera_ws;
        uniform_projection_view_matrix = projection_view_matrix;


        uniform_buffer_base = (int) 0 * group_size;
        dodecahedron.instanced_render(group_size);

        uniform_buffer_base = (int) 1 * group_size;
        cube.instanced_render(group_size);

        uniform_buffer_base = (int) 2 * group_size;
        octahedron.instanced_render(group_size);

        uniform_buffer_base = (int) 3 * group_size;
        tetrahedron.instanced_render(group_size);

        uniform_buffer_base = (int) 4 * group_size;
        icosahedron.instanced_render(group_size);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;

}



