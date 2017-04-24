//========================================================================================================================================================================================================================
// DEMO 022: Reflections
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

vertex_pft2_t torus_func(const glm::vec2& uv)
{
    vertex_pft2_t vertex;
    vertex.uv = uv;

    float cos_2piu = glm::cos(constants::two_pi * uv.y);
    float sin_2piu = glm::sin(constants::two_pi * uv.y);
    float cos_2piv = glm::cos(constants::two_pi * uv.x);
    float sin_2piv = glm::sin(constants::two_pi * uv.x);

    float R = 0.7f;
    float r = 0.3f;

    vertex.position = glm::vec3(
                        (R + r * cos_2piu) * cos_2piv,
                        (R + r * cos_2piu) * sin_2piv,
                             r * sin_2piu);

    vertex.tangent_x = glm::vec3(-sin_2piu * cos_2piv, -sin_2piu * sin_2piv, cos_2piu);
    vertex.tangent_y = glm::vec3(-sin_2piv, cos_2piv, 0.0f);

    vertex.normal = glm::vec3(cos_2piu * cos_2piv, cos_2piu * sin_2piv, sin_2piu);

    return vertex;
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
        data[index].rotor = glm::vec4(glm::sphericalRand(1.0f), 2.0f * glm::gaussRand(0.0f, 1.0f));
        index++;
    }
}

//=======================================================================================================================================================================================================================
// Computes reflection matrix
// n is the normal vector to the plane, d is the distance from the plane to the origin, 
// so the plane consists of the vectors v satisfying {v : <n, v> + d = 0}
//=======================================================================================================================================================================================================================
glm::mat4 reflection_matrix(const glm::vec3& n, float d)
{
    float m_2xy = -2.0f * n.x * n.y;
    float m_2xz = -2.0f * n.x * n.z;
    float m_2yz = -2.0f * n.y * n.z;

    return glm::mat4(glm::vec4(1.0f - 2.0f * n.x * n.x,                   m_2xy,                   m_2xz, 0.0f),
                     glm::vec4(                  m_2xy, 1.0f - 2.0f * n.y * n.y,                   m_2yz, 0.0f),
                     glm::vec4(                  m_2xz,                   m_2yz, 1.0f - 2.0f * n.z * n.z, 0.0f),
                     glm::vec4(-2.0f * d * n, 1.0f));
}

struct room
{
    GLuint vao_id;    
    vbo_t vbo;

    room(float size)
    {
        vertex_pnt2_t vertices[36];

        glm::vec2 unit_square[4] = 
        {
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f)
        };

        int index = 0;
        int vindex = 0;

        for(int i = 0; i < 6; ++i)
        {
            int A = plato::cube::faces[vindex++];
            int B = plato::cube::faces[vindex++];
            int C = plato::cube::faces[vindex++];
            int D = plato::cube::faces[vindex++];
            glm::vec3 normal = -plato::cube::normals[i];
            vertices[index++] = vertex_pnt2_t(size * plato::cube::vertices[A], normal, unit_square[0]);
            vertices[index++] = vertex_pnt2_t(size * plato::cube::vertices[B], normal, unit_square[1]);
            vertices[index++] = vertex_pnt2_t(size * plato::cube::vertices[C], normal, unit_square[2]);
            vertices[index++] = vertex_pnt2_t(size * plato::cube::vertices[A], normal, unit_square[0]);
            vertices[index++] = vertex_pnt2_t(size * plato::cube::vertices[C], normal, unit_square[2]);
            vertices[index++] = vertex_pnt2_t(size * plato::cube::vertices[D], normal, unit_square[3]);
        }

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        vbo.init(vertices, 36);
    }

    void render()
    {
        glBindVertexArray(vao_id);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    ~room()
        { glDeleteVertexArrays(1, &vao_id); };
};

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

    demo_window_t window("Reflections", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // phong lighting model shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t simple_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_light.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_light.fs"));

    simple_light.enable();
    uniform_t uniform_projection_view_matrix = simple_light["projection_view_matrix"];
    uniform_t uniform_time                   = simple_light["time"];                  
    uniform_t uniform_light_ws               = simple_light["light_ws"];              
    uniform_t uniform_camera_ws              = simple_light["camera_ws"];             
    uniform_t uniform_base                   = simple_light["buffer_base"];
    uniform_t uniform_diffuse_texture        = simple_light["diffuse_texture"];
    uniform_t uniform_normal_texture         = simple_light["normal_texture"];

    simple_light["solid_scale"] = 15.0f;

    //===================================================================================================================================================================================================================
    // Initialize buffers : position + tangent frame + texture coordinates 
    //===================================================================================================================================================================================================================
    polyhedron tetrahedron, cube, octahedron, dodecahedron, icosahedron;
    tetrahedron.regular_pft2_vao(4, 4, plato::tetrahedron::vertices, plato::tetrahedron::normals, plato::tetrahedron::faces);
    cube.regular_pft2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces);
    octahedron.regular_pft2_vao(6, 8, plato::octahedron::vertices, plato::octahedron::normals, plato::octahedron::faces);
    dodecahedron.regular_pft2_vao(20, 12, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);
    icosahedron.regular_pft2_vao(12, 20, plato::icosahedron::vertices, plato::icosahedron::normals, plato::icosahedron::faces);

    //===================================================================================================================================================================================================================
    // Creating toral mesh
    //===================================================================================================================================================================================================================
    surface_t torus1;
    torus1.generate_vao<vertex_pft2_t>(torus_func, 24, 64);

    //===================================================================================================================================================================================================================
    // Load textures : diffuse + bump for each polyhedron
    //===================================================================================================================================================================================================================
    const int TEXTURE_COUNT = 12;
    GLuint texture_id[TEXTURE_COUNT];

    const char* texture_filenames [TEXTURE_COUNT] = 
    {
        "../../../resources/plato_tex2d/tetrahedron.png",  "../../../resources/plato_tex2d/tetrahedron_bump.png",
        "../../../resources/plato_tex2d/cube.png",         "../../../resources/plato_tex2d/cube_bump.png",
        "../../../resources/plato_tex2d/octahedron.png",   "../../../resources/plato_tex2d/octahedron_bump.png",
        "../../../resources/plato_tex2d/pentagon.png",     "../../../resources/plato_tex2d/pentagon_bump.png",
        "../../../resources/plato_tex2d/icosahedron.png",  "../../../resources/plato_tex2d/icosahedron_bump.png",
        "../../../resources/tex2d/torus.png",              "../../../resources/tex2d/torus_bump.png"
    };

    for (int i = 0; i < TEXTURE_COUNT; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        texture_id[i] = image::png::texture2d(texture_filenames[i]);
    }

    glActiveTexture(GL_TEXTURE0 + TEXTURE_COUNT);
    GLuint glass_texture_id = image::png::texture2d("../../../resources/tex2d/marble.png");

    //===================================================================================================================================================================================================================
    // Mirror shader to render the reflected image : mirror surface texture goes to texture unit #TEXTURE_COUNT
    //===================================================================================================================================================================================================================
    glsl_program_t mirror_shader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/mirror.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/mirror.fs"));

    mirror_shader.enable();
    uniform_t mirror_projection_view_matrix = mirror_shader["projection_view_matrix"];
    uniform_t mirror_glass_texture          = mirror_shader["glass_texture"];
    uniform_t mirror_light_ws               = mirror_shader["light_ws"];
    uniform_t mirror_camera_ws              = mirror_shader["camera_ws"];    

    mirror_glass_texture = (int) TEXTURE_COUNT;


    //===================================================================================================================================================================================================================
    // Initialize objects displacement vectors and rotation axes, and write the data to GL_SHADER_STORAGE_BUFFER
    // The buffer will be read according to gl_InstanceID variable and buffer_base uniform
    //===================================================================================================================================================================================================================
    const int N = 4;
    const int group_size = N * N * N;
    const float cell_size = 30.0f;
    const float origin_distance = 1.25f * cell_size * N;
    const float mirror_size = origin_distance + 0.6f * cell_size * N;
    const float height = 0.5f * cell_size * N;
    const GLsizeiptr chunk_size = group_size * sizeof(motion3d_t);  

    motion3d_t data[6 * group_size];

    fill_shift_rotor_data(&data[0 * group_size], glm::vec3(            0.0f,             0.0f,  origin_distance), cell_size, N);
    fill_shift_rotor_data(&data[1 * group_size], glm::vec3(            0.0f,             0.0f, -origin_distance), cell_size, N);
    fill_shift_rotor_data(&data[2 * group_size], glm::vec3(            0.0f,  origin_distance,             0.0f), cell_size, N);
    fill_shift_rotor_data(&data[3 * group_size], glm::vec3(            0.0f, -origin_distance,             0.0f), cell_size, N);
    fill_shift_rotor_data(&data[4 * group_size], glm::vec3( origin_distance,             0.0f,             0.0f), cell_size, N);
    fill_shift_rotor_data(&data[5 * group_size], glm::vec3(-origin_distance,             0.0f,             0.0f), cell_size, N);

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
    const float light_radius = cell_size * N; 

    const int REFLECTIONS_COUNT = plato::cube::F;
    glm::mat4 reflections[REFLECTIONS_COUNT];

    const float cube_size = 500.0;

    for (int i = 0; i < REFLECTIONS_COUNT; ++i)
        reflections[i] = reflection_matrix(plato::cube::normals[i], cube_size);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(torus1.vao.ibo.pri);

    room granite_room(cube_size);    

    //===================================================================================================================================================================================================================
    // the main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        glm::vec4 light_ws = glm::vec4(light_radius * cos(5.5f * time), light_radius * sin(5.5f * time), -0.66f * light_radius, 1.0f);
        glm::vec4 camera_ws = glm::vec4(window.camera.position(), 1.0f);
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        simple_light.enable();
        uniform_time = time;
        uniform_light_ws = light_ws;
        uniform_camera_ws = camera_ws;

        //===============================================================================================================================================================================================================
        // Render reflected images inverting orientation
        //===============================================================================================================================================================================================================

        glCullFace(GL_FRONT);

        for (int i = 0; i < REFLECTIONS_COUNT; ++i)
        {
            glm::mat4 projection_view_reflection_matrix = projection_view_matrix * reflections[i];

            uniform_projection_view_matrix = projection_view_reflection_matrix;
            uniform_base = 0 * group_size;
            uniform_diffuse_texture = 0;
            uniform_normal_texture = 1;
            tetrahedron.instanced_render(group_size);
    
            uniform_base = 1 * group_size;
            uniform_diffuse_texture = 2;
            uniform_normal_texture = 3;
            cube.instanced_render(group_size);
    
            uniform_base = 2 * group_size;
            uniform_diffuse_texture = 4;
            uniform_normal_texture = 5;
            octahedron.instanced_render(group_size);
    
            uniform_base = 3 * group_size;
            uniform_diffuse_texture = 6;
            uniform_normal_texture = 7;
            dodecahedron.instanced_render(group_size);
    
            uniform_base = 4 * group_size;
            uniform_diffuse_texture = 8;
            uniform_normal_texture = 9;
            icosahedron.instanced_render(group_size);

            uniform_base = 5 * group_size;
            uniform_diffuse_texture = 10;
            uniform_normal_texture = 11;
            torus1.instanced_render(group_size);
        };

        //===============================================================================================================================================================================================================
        // Render marble room 
        //===============================================================================================================================================================================================================

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        mirror_shader.enable();

        mirror_light_ws = light_ws;
        mirror_camera_ws = camera_ws;
        mirror_projection_view_matrix = projection_view_matrix;
        granite_room.render();

        //===============================================================================================================================================================================================================
        // Render original objects with normal face orientation
        //===============================================================================================================================================================================================================

        simple_light.enable();
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glCullFace(GL_BACK);

        uniform_projection_view_matrix = projection_view_matrix;

        uniform_base = 0 * group_size;
        uniform_diffuse_texture = 0;
        uniform_normal_texture = 1;
        tetrahedron.instanced_render(group_size);

        uniform_base = 1 * group_size;
        uniform_diffuse_texture = 2;
        uniform_normal_texture = 3;
        cube.instanced_render(group_size);

        uniform_base = 2 * group_size;
        uniform_diffuse_texture = 4;
        uniform_normal_texture = 5;
        octahedron.instanced_render(group_size);

        uniform_base = 3 * group_size;
        uniform_diffuse_texture = 6;
        uniform_normal_texture = 7;
        dodecahedron.instanced_render(group_size);

        uniform_base = 4 * group_size;
        uniform_diffuse_texture = 8;
        uniform_normal_texture = 9;
        icosahedron.instanced_render(group_size);

        uniform_base = 5 * group_size;
        uniform_diffuse_texture = 10;
        uniform_normal_texture = 11;
        torus1.instanced_render(group_size);

        window.end_frame();
    }

    glDeleteTextures(TEXTURE_COUNT, texture_id);
    glDeleteTextures(1, &glass_texture_id);

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}