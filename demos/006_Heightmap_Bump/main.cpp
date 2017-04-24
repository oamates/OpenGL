//========================================================================================================================================================================================================================
// DEMO 006 : Heightmap Bump
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
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

struct motion3d_t
{
    glm::vec3 shift;
    float hue;
    glm::vec3 axis;
    float angular_velocity;
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
        data[index].shift = group_shift + cell_size * glm::vec3(float(i) - middle, float(j) - middle, float(k) - middle);
        data[index].hue = glm::gaussRand(0.0f, 1.0f);
        data[index].axis = glm::sphericalRand(1.0f);
        data[index].angular_velocity = glm::gaussRand(0.0f, 1.0f);
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

    demo_window_t window("Ray Marching", 4, 4, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // blinn-phong lighting model with framed bump texturing
    //===================================================================================================================================================================================================================
    glsl_program_t framed_lighting(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/framed_lighting.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/framed_lighting.fs"));

    framed_lighting.enable();
    
    uniform_t uni_fl_pv_matrix   = framed_lighting["projection_view_matrix"];      
    uniform_t uni_fl_light_ws    = framed_lighting["light_ws"];         
    uniform_t uni_fl_camera_ws   = framed_lighting["camera_ws"];        
    uniform_t uni_fl_solid_scale = framed_lighting["solid_scale"];
    uniform_t uni_fl_time        = framed_lighting["time"];
    uniform_t uni_fl_base        = framed_lighting["buffer_base"];

    framed_lighting["diffuse_texture"] = 0;
    framed_lighting["bump_texture"] = 1;

    //===================================================================================================================================================================================================================
    // blinn-phong lighting model with bump texturing using normals and partial derivatives
    //===================================================================================================================================================================================================================
    glsl_program_t normal_lighting(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/normal_lighting.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/normal_lighting.fs"));

    normal_lighting.enable();

    uniform_t uni_nl_pv_matrix   = normal_lighting["projection_view_matrix"];      
    uniform_t uni_nl_light_ws    = normal_lighting["light_ws"];         
    uniform_t uni_nl_camera_ws   = normal_lighting["camera_ws"];        
    uniform_t uni_nl_solid_scale = normal_lighting["solid_scale"];
    uniform_t uni_nl_time        = normal_lighting["time"];
    uniform_t uni_nl_base        = normal_lighting["buffer_base"];

    normal_lighting["diffuse_texture"] = 0;
    normal_lighting["bump_texture"] = 1;

    //===================================================================================================================================================================================================================
    // load textures
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint diffuse_texture_id = image::png::texture2d("../../../resources/tex2d/rock.png");
    glActiveTexture(GL_TEXTURE1);
    GLuint bump_texture_id    = image::png::texture2d("../../../resources/tex2d/rock_heightmap.png");

    //===================================================================================================================================================================================================================
    // create two buffers
    //===================================================================================================================================================================================================================
    polyhedron framed_cube, normal_cube;
    framed_cube.regular_pft2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces);
    normal_cube.regular_pnt2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces);

    //===================================================================================================================================================================================================================
    // light variables
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    const int N = 5;
    const int group_size = N * N * N;
    const float cell_size = 0.75f;
    const float origin_distance = 0.75f * cell_size * N;

    motion3d_t data[2 * group_size];

    fill_shift_rotor_data(&data[0 * group_size], glm::vec3(0.0f, 0.0f, -origin_distance), cell_size, N);
    fill_shift_rotor_data(&data[1 * group_size], glm::vec3(0.0f, 0.0f,  origin_distance), cell_size, N);

    GLuint ssbo_id;
    glGenBuffers(1, &ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ssbo_id, 0, sizeof(data));


    //===================================================================================================================================================================================================================
    // main program loop : just clear the buffer in a loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 camera_ws = window.camera.position();
        glm::vec3 light_ws = glm::vec3(200.0f * glm::cos(time), 200.0f * glm::sin(time), 0.0f);

        framed_lighting.enable();
        uni_fl_pv_matrix = projection_view_matrix;
        uni_fl_light_ws = light_ws;
        uni_fl_camera_ws = camera_ws;
        uni_fl_solid_scale = 2.0f * cell_size / N;
        uni_fl_time = time;
        uni_fl_base = 0;
        framed_cube.instanced_render(group_size);


        normal_lighting.enable();
        uni_nl_pv_matrix = projection_view_matrix;
        uni_nl_light_ws = light_ws;
        uni_nl_camera_ws = camera_ws;
        uni_nl_solid_scale = 2.0f * cell_size / N;
        uni_nl_time = time;
        uni_nl_base = group_size;
        normal_cube.instanced_render(group_size);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}