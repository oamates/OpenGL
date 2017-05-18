//========================================================================================================================================================================================================================
// DEMO 063: Omnidirectional shadows
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
#include <glm/gtc/matrix_transform.hpp>

#include "constants.hpp"
#include "glfw_window.hpp"
#include "log.hpp"
#include "gl_info.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "plato.hpp"
#include "polyhedron.hpp"
#include "torus.hpp"
#include "vertex.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;
    bool pause = false;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
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

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
            pause = !pause;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

struct fbo_depth_cubemap_array
{
    GLuint id;
    GLuint texture_id;

    fbo_depth_cubemap_array() : id(0), texture_id(0) {};

    fbo_depth_cubemap_array(GLenum texture_unit, GLsizei texture_size, GLuint depth, GLenum internal_format = GL_DEPTH_COMPONENT32)
    {
        debug_msg("Creating depth only FBO with GL_TEXTURE_CUBE_MAP_ARRAY attachment.");
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        glGenTextures(1, &texture_id);
        glActiveTexture (texture_unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, texture_id);
        //glTexStorage2D(GL_TEXTURE_CUBE_MAP_ARRAY, 1, internal_format, texture_size, texture_size);

        glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 1, internal_format, texture_size, texture_size, 6 * depth);

        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_id, 0);



        //================================================================================================================================================================================================================
        // check that the created framebuffer object is ok
        //================================================================================================================================================================================================================
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            exit_msg("Could not initialize GL_FRAMEBUFFER object.");

        debug_msg("Done. FBO id = %d. texture_id = %d", id, texture_id);
    }

    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    void bind_texture(GLenum texture_unit)
    {
        glActiveTexture (texture_unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
    }

    ~fbo_depth_cubemap_array()
    {
        glDeleteTextures(1, &texture_id);
        glDeleteFramebuffers(1, &id);       
    }
};


//=======================================================================================================================================================================================================================
// Standard parameterization of the torus of rotation
//=======================================================================================================================================================================================================================
vertex_pnt2_t torus3d_func (const glm::vec2& uv)
{
    vertex_pnt2_t vertex;

    const double R = 1.5;
    const double r = 0.6;
    double cos_2piu = glm::cos(constants::two_pi_d * uv.x);
    double sin_2piu = glm::sin(constants::two_pi_d * uv.x);
    double cos_2piv = glm::cos(constants::two_pi_d * uv.y);
    double sin_2piv = glm::sin(constants::two_pi_d * uv.y);

    vertex.position = glm::vec3((R + r * cos_2piv) * cos_2piu, (R + r * cos_2piv) * sin_2piu, r * sin_2piv);
    vertex.normal = glm::vec3(cos_2piv * cos_2piu, cos_2piv * sin_2piu, sin_2piv);
    vertex.uv = uv;
    return vertex;
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
        data[index].rotor = glm::vec4(glm::sphericalRand(1.0f), 0.75f * glm::gaussRand(0.0f, 1.0f));
        index++;
    };
};

const GLenum DEPTH_CUBEMAP_TEXTURE_UNIT = GL_TEXTURE10;
const GLuint DEPTH_CUBEMAP_TEXTURE_SIZE = 2048;
const int LIGHT_COUNT = 4;

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

    demo_window_t window("Physics-Based Rendering", 4, 4, 2, 1920, 1080);

    //===================================================================================================================================================================================================================
    // shadow map shader, renders to cube map distance-to-light texture
    //===================================================================================================================================================================================================================
    glsl_program_t shadow_map(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/shadow_map.vs"),
                              glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/shadow_map.gs"),
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/shadow_map.fs"));

    shadow_map.enable();

    uniform_t uni_sm_light_ws    = shadow_map["light_ws"];
    uniform_t uni_sm_buffer_base = shadow_map["buffer_base"];
    uniform_t uni_sm_time        = shadow_map["time"];

    //===================================================================================================================================================================================================================
    // Phong lighting model shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t simple_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/simple_light.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/simple_light.fs"));

    simple_light.enable();

    uniform_t uni_sl_pv_matrix   = simple_light["projection_view_matrix"];
    uniform_t uni_sl_camera_ws   = simple_light["camera_ws"];
    uniform_t uni_sl_light_ws    = simple_light["light_ws"];
    uniform_t uni_sl_diffuse_tex = simple_light["diffuse_tex"];
    uniform_t uni_sl_normal_tex  = simple_light["normal_tex"];
    uniform_t uni_sl_base        = simple_light["buffer_base"];
    uniform_t uni_sl_time        = simple_light["time"];

    //===================================================================================================================================================================================================================
    // polyhedra + torus models initialization
    //===================================================================================================================================================================================================================
    polyhedron tetrahedron, cube, octahedron, dodecahedron, icosahedron;

    tetrahedron.regular_pft2_vao(4, 4, plato::tetrahedron::vertices, plato::tetrahedron::normals, plato::tetrahedron::faces);
    cube.regular_pft2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces);
    octahedron.regular_pft2_vao(6, 8, plato::octahedron::vertices, plato::octahedron::normals, plato::octahedron::faces);
    dodecahedron.regular_pft2_vao(20, 12, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);
    icosahedron.regular_pft2_vao(12, 20, plato::icosahedron::vertices, plato::icosahedron::normals, plato::icosahedron::faces);

    glActiveTexture(GL_TEXTURE0); GLuint tetrahedron_diffuse_texture_id  = image::png::texture2d("../../../resources/plato_tex2d/tetrahedron.png");
    glActiveTexture(GL_TEXTURE1); GLuint tetrahedron_normal_texture_id   = image::png::texture2d("../../../resources/plato_tex2d/tetrahedron_bump.png");
    glActiveTexture(GL_TEXTURE2); GLuint cube_diffuse_texture_id         = image::png::texture2d("../../../resources/plato_tex2d/cube.png");
    glActiveTexture(GL_TEXTURE3); GLuint cube_normal_texture_id          = image::png::texture2d("../../../resources/plato_tex2d/cube_bump.png");
    glActiveTexture(GL_TEXTURE4); GLuint octahedron_diffuse_texture_id   = image::png::texture2d("../../../resources/plato_tex2d/octahedron.png");
    glActiveTexture(GL_TEXTURE5); GLuint octahedron_normal_texture_id    = image::png::texture2d("../../../resources/plato_tex2d/octahedron_bump.png");
    glActiveTexture(GL_TEXTURE6); GLuint dodecahedron_diffuse_texture_id = image::png::texture2d("../../../resources/plato_tex2d/pentagon.png");
    glActiveTexture(GL_TEXTURE7); GLuint dodecahedron_normal_texture_id  = image::png::texture2d("../../../resources/plato_tex2d/pentagon_bump.png");
    glActiveTexture(GL_TEXTURE8); GLuint icosahedron_diffuse_texture_id  = image::png::texture2d("../../../resources/plato_tex2d/icosahedron.png");
    glActiveTexture(GL_TEXTURE9); GLuint icosahedron_normal_texture_id   = image::png::texture2d("../../../resources/plato_tex2d/icosahedron_bump.png");

    const int N = 3;
    const int group_size = N * N * N;
    const float cell_size = 2.25f;
    const float origin_distance = 1.25f * cell_size * N;
    const GLsizeiptr chunk_size = group_size * sizeof(motion3d_t);  

    motion3d_t data[5 * group_size];

    fill_shift_rotor_data(&data[0 * group_size], glm::vec3(0.0f,             0.0f,  origin_distance), cell_size, N);
    fill_shift_rotor_data(&data[1 * group_size], glm::vec3(0.0f,             0.0f, -origin_distance), cell_size, N);
    fill_shift_rotor_data(&data[2 * group_size], glm::vec3(0.0f,  origin_distance,             0.0f), cell_size, N);
    fill_shift_rotor_data(&data[3 * group_size], glm::vec3(0.0f, -origin_distance,             0.0f), cell_size, N);
    fill_shift_rotor_data(&data[4 * group_size], glm::vec3(0.0f,             0.0f,             0.0f), cell_size, N);

    GLuint ssbo_id;
    glGenBuffers(1, &ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ssbo_id, 0, sizeof(data));

    fbo_depth_cubemap_array shadow_cubemap(DEPTH_CUBEMAP_TEXTURE_UNIT, DEPTH_CUBEMAP_TEXTURE_SIZE, LIGHT_COUNT);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0f);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    
    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while (!window.should_close())
    {
        window.new_frame();

        float time = 0.25f * window.frame_ts;
        float radius = 5.0f;

        glm::vec3 light_ws[4] = 
        {
            radius * glm::vec3(1.0f, glm::cos(0.15f * time), glm::sin(0.16f * time)),
            radius * glm::vec3(glm::sin(0.13f * time), 1.0f, glm::cos(0.13f * time)),
            radius * glm::vec3(glm::cos(0.17f * time), glm::sin(0.11f * time), 1.0f),
            radius * glm::vec3(0.4f, glm::sin(0.14f * time), glm::sin(0.18f * time)),
        };

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 camera_ws = window.camera.position();

        //===============================================================================================================================================================================================================
        // render pass to get cubemap z-buffer filled w.r.t current light position
        //===============================================================================================================================================================================================================
        glViewport(0, 0, DEPTH_CUBEMAP_TEXTURE_SIZE, DEPTH_CUBEMAP_TEXTURE_SIZE);

        shadow_cubemap.bind();
        glClear(GL_DEPTH_BUFFER_BIT);

        shadow_map.enable();
        glCullFace(GL_FRONT);

        if (!window.pause)
        {
            uni_sm_light_ws = light_ws;
            uni_sm_time = time;
        }

        uni_sm_buffer_base = 0 * group_size; tetrahedron.instanced_render(group_size);
        uni_sm_buffer_base = 1 * group_size; cube.instanced_render(group_size);
        uni_sm_buffer_base = 2 * group_size; octahedron.instanced_render(group_size);
        uni_sm_buffer_base = 3 * group_size; dodecahedron.instanced_render(group_size);
        uni_sm_buffer_base = 4 * group_size; icosahedron.instanced_render(group_size);

        //===============================================================================================================================================================================================================
        // on-screen render pass using created depth texture
        //===============================================================================================================================================================================================================

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window.res_x, window.res_y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_BACK);
        simple_light.enable();

        uni_sl_pv_matrix = projection_view_matrix;
        uni_sl_camera_ws = camera_ws;

        if (!window.pause)
        {
            uni_sl_time = time;
            uni_sl_light_ws = light_ws;
        }
    
        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

        uni_sl_base = 0 * group_size;
        uni_sl_diffuse_tex = 0;
        uni_sl_normal_tex = 1;
        tetrahedron.instanced_render(group_size);

        uni_sl_base = 1 * group_size;
        uni_sl_diffuse_tex = 2;
        uni_sl_normal_tex = 3;
        cube.instanced_render(group_size);

        uni_sl_base = 2 * group_size;
        uni_sl_diffuse_tex = 4;
        uni_sl_normal_tex = 5;
        octahedron.instanced_render(group_size);

        uni_sl_base = 3 * group_size;
        uni_sl_diffuse_tex = 6;
        uni_sl_normal_tex = 7;
        dodecahedron.instanced_render(group_size);

        uni_sl_base = 4 * group_size;
        uni_sl_diffuse_tex = 8;
        uni_sl_normal_tex = 9;
        icosahedron.instanced_render(group_size);
     
        window.end_frame();
    }

    glfw::terminate();
    return 0;
}                               