//========================================================================================================================================================================================================================
// DEMO 016: CUBEMAP Textures and Environment map
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

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "surface.hpp"
#include "sphere.hpp"
#include "plato.hpp"

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

vertex_pnt2_t torus_func(const glm::vec2& uv)
{
    vertex_pnt2_t vertex;
    vertex.uv = uv;

    float cos_2piu = glm::cos(constants::two_pi * uv.y);
    float sin_2piu = glm::sin(constants::two_pi * uv.y);
    float cos_2piv = glm::cos(constants::two_pi * uv.x);
    float sin_2piv = glm::sin(constants::two_pi * uv.x);

    float R = 8.0f;
    float r = 3.2f;

    vertex.position = glm::vec3((R + r * cos_2piu) * cos_2piv,
                                (R + r * cos_2piu) * sin_2piv,
                                r * sin_2piu);

    vertex.normal = glm::vec3(cos_2piu * cos_2piv, cos_2piu * sin_2piv, sin_2piu);

    return vertex;
}

vertex_pnt3_t ellipsoid_func(const glm::vec3& uvw)
{
    const glm::vec3 axes = glm::vec3(12.0f, 8.0f, 7.0f);
    vertex_pnt3_t vertex;    
    vertex.uvw = uvw;
    vertex.position = glm::vec3(0.0f, 0.0f, -30.0f) + axes * glm::normalize(uvw);
    vertex.normal = glm::normalize(uvw / axes);
    return vertex;
}

vertex_pnt3_t minkowski_L4_support_func(const glm::vec3& uvw)
{
    vertex_pnt3_t vertex;    
    vertex.uvw = uvw;

    glm::vec3 uvw2 = uvw * uvw;
    glm::vec3 uvw3 = uvw2 * uvw;

    float inv_norm = 1.0f / sqrt(glm::length(uvw2));
    float inv_der_norm = 1.0f / glm::length(uvw3);

    vertex.position = glm::vec3(0.0f, 0.0f, 30.0f) + 8.0f * uvw * inv_norm;
    vertex.normal = glm::normalize(uvw3);

    return vertex;
}

vertex_pnt3_t minkowski_L6_support_func(const glm::vec3& uvw)
{
    vertex_pnt3_t vertex;    
    vertex.uvw = uvw;

    glm::vec3 uvw2 = uvw * uvw;
    glm::vec3 uvw3 = uvw2 * uvw;
    glm::vec3 uvw5 = uvw3 * uvw2;

    float inv_norm = 1.0f / cbrt(glm::length(uvw3));
    float inv_der_norm = 1.0f / glm::length(uvw5);

    vertex.position = glm::vec3(0.0f, -30.0f, 0.0f) + 8.0f * uvw * inv_norm;
    vertex.normal = glm::normalize(uvw5);

    return vertex;
}

vertex_pnt3_t minkowski_L8_support_func(const glm::vec3& uvw)
{
    vertex_pnt3_t vertex;    
    vertex.uvw = uvw;

    glm::vec3 uvw2 = uvw * uvw;
    glm::vec3 uvw4 = uvw2 * uvw2;
    glm::vec3 uvw7 = uvw4 * uvw2 * uvw;

    float inv_norm = 1.0f / sqrt(sqrt((glm::length(uvw4))));

    vertex.position = glm::vec3(0.0f, 30.0f, 0.0f) + 8.0f * uvw * inv_norm;
    vertex.normal = glm::normalize(uvw7);

    return vertex;
}


vertex_pnt3_t smooth_dodecahedron_func(const glm::vec3& uvw)
{
    vertex_pnt3_t vertex;    
    vertex.uvw = uvw;
    vertex.normal = glm::normalize(uvw);

    float d = 0.0;
    for (int i = 0; i < plato::dodecahedron::V; ++i)
    {
        float q = glm::length2(plato::dodecahedron::vertices[i] - vertex.normal);
        d += 0.60f / (0.15f + q);
    }

    vertex.position = glm::vec3(-30.0f, 0.0f, 0.0f) + (vertex.normal) * d;

    return vertex;
}

vertex_pnt3_t smooth_icosahedron_func(const glm::vec3& uvw)
{
    vertex_pnt3_t vertex;    
    vertex.uvw = uvw;
    vertex.normal = glm::normalize(uvw);

    float d = 0.0;
    for (int i = 0; i < plato::icosahedron::V; ++i)
    {
        float q = glm::length2(plato::icosahedron::vertices[i] - vertex.normal);
        d += 0.80f / (0.14f + q);
    }

    vertex.position = glm::vec3(30.0f, 0.0f, 0.0f) + (vertex.normal) * d;

    return vertex;
}

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("OBJ Loader", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // skybox and environment map shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t skybox_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/skybox.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/skybox.fs"));

    skybox_renderer.enable();
    uniform_t uni_sbox_pv_matrix = skybox_renderer["projection_view_matrix"];
    skybox_renderer["environment_tex"] = 0;

    glsl_program_t envmap_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/envmap.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/envmap.fs"));

    envmap_renderer.enable();
    uniform_t uni_env_pv_matrix = envmap_renderer["projection_view_matrix"];
    uniform_t uni_env_camera_ws = envmap_renderer["camera_ws"];
    envmap_renderer["environment_tex"] = 0;

    //===================================================================================================================================================================================================================
    // Initialize cube buffer : vertices + indices
    //===================================================================================================================================================================================================================
    GLuint cube_vao_id, cube_vbo_id, cube_ibo_id;

    glGenVertexArrays(1, &cube_vao_id);
    glBindVertexArray(cube_vao_id);

    static const GLfloat cube_vertices[] =
    {
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f
    };

    static const GLushort cube_indices[] =
    {
        0, 1, 2, 3, 6, 7, 4, 5,
        2, 6, 0, 4, 1, 5, 3, 7
    };

    glGenBuffers(1, &cube_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &cube_ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    //===================================================================================================================================================================================================================
    // Creating toral mesh 
    //===================================================================================================================================================================================================================
    surface_t torus1;
    torus1.generate_vao<vertex_pnt2_t>(torus_func, 128, 64);

    sphere_t minkowski_L4_ball;
    minkowski_L4_ball.generate_vao_mt<vertex_pnt3_t>(minkowski_L4_support_func, 64);

    sphere_t minkowski_L6_ball;
    minkowski_L6_ball.generate_vao_mt<vertex_pnt3_t>(minkowski_L6_support_func, 64);

    sphere_t minkowski_L8_ball;
    minkowski_L8_ball.generate_vao_mt<vertex_pnt3_t>(minkowski_L8_support_func, 64);

    sphere_t ellipsoid;
    ellipsoid.generate_vao_mt<vertex_pnt3_t>(ellipsoid_func, 64);

    sphere_t smooth_dodecahedron;
    smooth_dodecahedron.generate_vao_mt<vertex_pnt3_t>(smooth_dodecahedron_func, 64);

    sphere_t smooth_icosahedron;
    smooth_icosahedron.generate_vao_mt<vertex_pnt3_t>(smooth_icosahedron_func, 64);

    //===================================================================================================================================================================================================================
    // Loading DDS cubemap texture
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    image::dds::image_t image;
    GLuint tex = image::dds::vglLoadTexture("res/cube.dds", 0, &image);
    image::dds::vglUnloadImage(&image);

    //===================================================================================================================================================================================================================
    // Global GL settings
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0x0000FFFF);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);


    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 camera_ws = window.camera.position();

        skybox_renderer.enable();
        glCullFace(GL_FRONT);
        uni_sbox_pv_matrix = projection_view_matrix;
        glBindVertexArray(cube_vao_id);
        glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_SHORT, 0);
        glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_SHORT, (void *)(8 * sizeof(GLushort)));

        envmap_renderer.enable();
        glCullFace(GL_BACK);
        uni_env_pv_matrix = projection_view_matrix;
        uni_env_camera_ws = camera_ws;

        torus1.render();
        minkowski_L4_ball.render();
        minkowski_L6_ball.render();
        minkowski_L8_ball.render();
        ellipsoid.render();
        smooth_dodecahedron.render();
        smooth_icosahedron.render();

        window.end_frame();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}