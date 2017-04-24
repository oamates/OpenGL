//========================================================================================================================================================================================================================
// DEMO 019: Plato Solids
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
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "plato.hpp"
#include "polyhedron.hpp"


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

struct motion3d_t
{
    glm::vec4 shift;
    glm::vec4 rotor;
};

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
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Plato Solids", 4, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // phong lighting model shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t simple_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_light.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_light.fs"));

    simple_light.enable();

    uniform_t uniform_projection_matrix = simple_light["projection_matrix"];
    uniform_t uniform_view_matrix       = simple_light["view_matrix"];      
    uniform_t uniform_time              = simple_light["time"];             
    uniform_t uniform_light_ws          = simple_light["light_ws"];         
    uniform_t uniform_camera_ws         = simple_light["camera_ws"];         
    uniform_t uniform_base              = simple_light["buffer_base"];
    uniform_t uniform_solid_scale       = simple_light["solid_scale"];
    uniform_t uniform_diffuse_texture   = simple_light["diffuse_texture"];
    uniform_t uniform_normal_texture    = simple_light["normal_texture"];


    //===================================================================================================================================================================================================================
    // initialize buffers : vertex + tangent frame + texture coordinates 
    // for five regular plato solids and load the diffuse + bump textures for each
    //===================================================================================================================================================================================================================
    polyhedron tetrahedron, cube, octahedron, dodecahedron, icosahedron;

    tetrahedron.regular_pft2_vao(4, 4, plato::tetrahedron::vertices, plato::tetrahedron::normals, plato::tetrahedron::faces);
    cube.regular_pft2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces);
    octahedron.regular_pft2_vao(6, 8, plato::octahedron::vertices, plato::octahedron::normals, plato::octahedron::faces);
    dodecahedron.regular_pft2_vao(20, 12, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);
    icosahedron.regular_pft2_vao(12, 20, plato::icosahedron::vertices, plato::icosahedron::normals, plato::icosahedron::faces);

    glActiveTexture(GL_TEXTURE0);
    GLuint tetrahedron_diffuse_texture_id  = image::png::texture2d("../../../resources/plato_tex2d/tetrahedron.png");
    glActiveTexture(GL_TEXTURE1);
    GLuint tetrahedron_normal_texture_id   = image::png::texture2d("../../../resources/plato_tex2d/tetrahedron_bump.png");
    glActiveTexture(GL_TEXTURE2);
    GLuint cube_diffuse_texture_id         = image::png::texture2d("../../../resources/plato_tex2d/cube.png");
    glActiveTexture(GL_TEXTURE3);
    GLuint cube_normal_texture_id          = image::png::texture2d("../../../resources/plato_tex2d/cube_bump.png");
    glActiveTexture(GL_TEXTURE4);
    GLuint octahedron_diffuse_texture_id   = image::png::texture2d("../../../resources/plato_tex2d/octahedron.png");
    glActiveTexture(GL_TEXTURE5);          
    GLuint octahedron_normal_texture_id    = image::png::texture2d("../../../resources/plato_tex2d/octahedron_bump.png");
    glActiveTexture(GL_TEXTURE6);
    GLuint dodecahedron_diffuse_texture_id = image::png::texture2d("../../../resources/plato_tex2d/pentagon.png");
    glActiveTexture(GL_TEXTURE7);
    GLuint dodecahedron_normal_texture_id  = image::png::texture2d("../../../resources/plato_tex2d/pentagon_bump.png");
    glActiveTexture(GL_TEXTURE8);
    GLuint icosahedron_diffuse_texture_id  = image::png::texture2d("../../../resources/plato_tex2d/icosahedron.png");
    glActiveTexture(GL_TEXTURE9);
    GLuint icosahedron_normal_texture_id   = image::png::texture2d("../../../resources/plato_tex2d/icosahedron_bump.png");


    const int N = 8;
    const int group_size = N * N * N;
    const float cell_size = 6.0f;
    const float origin_distance = 1.25f * cell_size * N;

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
    uniform_projection_matrix = window.camera.projection_matrix;
    uniform_solid_scale = 2.5f;

    //===================================================================================================================================================================================================================
    // the main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;

        uniform_time = time;
        uniform_view_matrix = window.camera.view_matrix;
    
        glm::vec3 light_ws = glm::vec3(cell_size * cos(0.25f * time), cell_size * sin(0.25f * time), cell_size * N);
        uniform_light_ws = light_ws;

        glm::vec3 camera_ws = window.camera.position();
        uniform_camera_ws = camera_ws;

        uniform_base = (int) 0 * group_size;
        uniform_diffuse_texture = 0;
        uniform_normal_texture = 1;
        tetrahedron.instanced_render(group_size);

        uniform_base = (int) 1 * group_size;
        uniform_diffuse_texture = 2;
        uniform_normal_texture = 3;
        cube.instanced_render(group_size);

        uniform_base = (int) 2 * group_size;
        uniform_diffuse_texture = 4;
        uniform_normal_texture = 5;
        octahedron.instanced_render(group_size);

        uniform_base = (int) 3 * group_size;
        uniform_diffuse_texture = 6;
        uniform_normal_texture = 7;
        dodecahedron.instanced_render(group_size);

        uniform_base = (int) 4 * group_size;
        uniform_diffuse_texture = 8;
        uniform_normal_texture = 9;
        icosahedron.instanced_render(group_size);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}