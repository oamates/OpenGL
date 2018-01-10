//========================================================================================================================================================================================================================
// DEMO 059 : Physics-Based Rendering
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "image/stb_image.h"

#include "log.hpp"
#include "gl_aux.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "vao.hpp"
#include "vertex.hpp"
#include "image.hpp"
#include "sampler.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
          camera(128.0f, 0.5f, glm::mat4(1.0f))
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
    }

    int env_map = 0;
    int level = 0;
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
            env_map = (env_map + 1) % 3;

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE))
            level = (level + 1) % 8;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

struct sphere_t
{
    vao_t vao;

    sphere_t(int res_x, int res_y)
    {
        int V = (res_x + 1) * (res_y + 1);
        int I = 2 * (res_x + 1) * res_y;

        vertex_pnt2_t* vertices = (vertex_pnt2_t*) malloc(V * sizeof(vertex_pnt2_t));
        GLuint* indices = (GLuint*) malloc (I * sizeof(GLuint));

        int v = 0;
        for (int y = 0; y <= res_y; ++y)
        {
            for (int x = 0; x <= res_x; ++x)
            {
                glm::vec2 uv = glm::vec2(x, y) / glm::vec2(res_x, res_y);
                glm::vec3 position;

                float sn_y = glm::sin(uv.y * constants::pi);
                position.x = glm::cos(uv.x * constants::two_pi) * sn_y;
                position.y = glm::cos(uv.y * constants::pi);
                position.z = glm::sin(uv.x * constants::two_pi) * sn_y;

                vertices[v++] = vertex_pnt2_t(position, position, uv);
            }
        }

        bool even_row = true;
        int i = 0;
        for (int y = 0; y < res_y; ++y)
        {
            if (even_row) // even rows: y == 0, y == 2; and so on
            {
                for (int x = 0; x <= res_x; ++x)
                {
                    indices[i++] = y * (res_x + 1) + x;
                    indices[i++] = (y + 1) * (res_x + 1) + x;
                }
            }
            else
            {
                for (int x = res_x; x >= 0; --x)
                {
                    indices[i++] = (y + 1) * (res_x + 1) + x;
                    indices[i++] = y * (res_x + 1) + x;
                }
            }
            even_row = !even_row;
        }

        vao = vao_t(GL_TRIANGLE_STRIP, vertices, V, indices, I);

        free(vertices);
        free(indices);
    }

    void render()
        { vao.render(); }
};


//=======================================================================================================================================================================================================================
// lights
//=======================================================================================================================================================================================================================
const int light_count = 4;

glm::vec3 light_positions[light_count];

const float luma = 128.0f;
glm::vec3 light_colors[light_count] =
{
    luma * glm::vec3(1.0f, 1.0f, 0.0f),
    luma * glm::vec3(0.0f, 0.0f, 1.0f),
    luma * glm::vec3(0.0f, 1.0f, 0.0f),
    luma * glm::vec3(1.0f, 0.0f, 0.0f)
};

glm::vec2 uv0[light_count] =
{
    glm::vec2( 3.24182f,  1.23746f),
    glm::vec2( 8.01346f, -9.18265f),
    glm::vec2(-1.98346f, -7.20393f),
    glm::vec2(-1.87934f,  9.87124f)
};

glm::vec2 uv1[light_count] =
{
    glm::vec2(-0.12387f,  0.84136f),
    glm::vec2( 0.44121f, -0.13718f),
    glm::vec2( 0.19834f, -0.35836f),
    glm::vec2(-0.21387f,  0.34725f)
};


struct spherical_texture_generator_t
{
    int res_x, res_y;
    int unit;
    glsl_program_t generator;
    sampler_t sampler;

    GLuint fbo_id;                                                                  /*  Framebuffer for offscreen rendering  */
    GLuint vao_id;                                                                  /* attribute-less VAO for quad rendering */

    spherical_texture_generator_t(int unit, int res_x, int res_y)
        : res_x(res_x), res_y(res_y),
          unit(unit),
          generator(glsl_shader_t(GL_VERTEX_SHADER, "glsl/quad.vs"), glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/sphere_texgen.fs")),
          sampler(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_CLAMP_TO_EDGE)
    {
        glGenFramebuffers(1, &fbo_id);
        sampler.set_max_af_level(16.0);
        glGenVertexArrays(1, &vao_id);
    }

    void enable()
    {
        sampler.bind(unit);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);
        glViewport(0, 0, res_x, res_y);
        generator.enable();
        generator["texture_in"] = unit;
        glBindVertexArray(vao_id);
    }

    GLuint process(GLuint tex_in, GLuint tex_out)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, tex_in);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_out, 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      /* combine images into one */
    }
};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    const int res_x = 1920;
    const int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Physics-Based Rendering", 4, 3, 3, 1920, 1080);
    glEnable(GL_DEPTH_TEST);

    //===================================================================================================================================================================================================================
    // main PBR shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t pbr_shader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/pbr.vs"),
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/pbr.fs"));
    pbr_shader.enable();
    pbr_shader["albedo_map"]     = 0;
    pbr_shader["normal_map"]     = 1;
    pbr_shader["metallic_map"]   = 2;
    pbr_shader["roughness_map"]  = 3;
    pbr_shader["ao_map"]         = 4;

    uniform_t uni_pbr_pv_matrix        = pbr_shader["projection_view_matrix"];
    uniform_t uni_pbr_model_matrix     = pbr_shader["model_matrix"];
    uniform_t uni_pbr_camera_ws        = pbr_shader["camera_ws"];
    uniform_t uni_pbr_light_positions  = pbr_shader["light_positions"];
    uniform_t uni_pbr_light_colors     = pbr_shader["light_colors"];
    uniform_t uni_pbr_metallic_factor  = pbr_shader["metallic_factor"];
    uniform_t uni_pbr_roughness_factor = pbr_shader["roughness_factor"];

    uni_pbr_light_colors               = light_colors;

    //===================================================================================================================================================================================================================
    // rusted iron PBR material
    //
    // texture unit 0 :: albedo_map;
    // texture unit 1 :: normal_map;
    // texture unit 2 :: metallic_map;
    // texture unit 3 :: roughness_map;
    // texture unit 4 :: ao_map;
    //
    // shader that converts square x-periodic texture into spherical texture
    //===================================================================================================================================================================================================================
    const char* tex_names[5] = 
    {
        "../../../resources/tex2d/pbr/rusted_iron/albedo.png",
        "../../../resources/tex2d/pbr/rusted_iron/normal.png",
        "../../../resources/tex2d/pbr/rusted_iron/metallic.png",
        "../../../resources/tex2d/pbr/rusted_iron/roughness.png",
        "../../../resources/tex2d/pbr/rusted_iron/ao.png"
    };

    const int TEXTURE_MAX_LEVEL = 11;
    const int TEXTURE_RES_X = 1 << TEXTURE_MAX_LEVEL;
    const int TEXTURE_RES_Y = 1 << (TEXTURE_MAX_LEVEL - 1);

    spherical_texture_generator_t texgen(5, TEXTURE_RES_X, TEXTURE_RES_Y);
    texgen.enable();

    GLsizei BUFFER_SIZE = 3 * TEXTURE_RES_X * TEXTURE_RES_Y;                          /* store images for debug purposes */
    unsigned char* pixel_buffer = (unsigned char*) malloc(BUFFER_SIZE);

    for (int i = 0; i < 5; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        GLuint tex_in = image::png::texture2d(tex_names[i]);
        GLuint tex_out;
        glGenTextures(1, &tex_out);
        glBindTexture(GL_TEXTURE_2D, tex_out);
        glTexStorage2D(GL_TEXTURE_2D, TEXTURE_MAX_LEVEL + 1, GL_RGB8, TEXTURE_RES_X, TEXTURE_RES_Y);

        texgen.process(tex_in, tex_out);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, tex_out);
        glGenerateMipmap(GL_TEXTURE_2D);

        char tex_name[32];
        sprintf(tex_name, "tex%u.png", i);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_buffer);        
        image::png::write(tex_name, TEXTURE_RES_X, TEXTURE_RES_Y, pixel_buffer, PNG_COLOR_TYPE_RGB);
    }

    free(pixel_buffer);
    debug_msg("Done generating images");

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, res_x, res_y);

    //===================================================================================================================================================================================================================
    // light source rendering program
    //===================================================================================================================================================================================================================
    glsl_program_t light_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/light_renderer.vs"),
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/light_renderer.fs"));

    uniform_t uni_lr_color        = light_renderer["color"];
    uniform_t uni_lr_camera_ws    = light_renderer["camera_ws"];
    uniform_t uni_lr_pv_matrix    = light_renderer["projection_view_matrix"];
    uniform_t uni_lr_model_matrix = light_renderer["model_matrix"];

    sphere_t sphere(64, 64);

    glClearColor(0.04f, 0.01f, 0.09f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glCullFace(GL_FRONT);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while (!window.should_close())
    {
        window.new_frame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::mat4 view_matrix = window.camera.view_matrix;
        glm::vec3 camera_ws = window.camera.position();

        //===============================================================================================================================================================================================================
        // render light sources as simple spheres
        //===============================================================================================================================================================================================================
        light_renderer.enable();
        uni_lr_camera_ws = camera_ws;
        uni_lr_pv_matrix = projection_view_matrix;

        const float R = 12.0f;
        const float r = 7.0f;

        for (int i = 0; i < light_count; ++i)
        {
            float t = 1.75f * window.frame_ts;
            glm::vec2 uv = uv0[i] + t * uv1[i];

            float cos_2piu = glm::cos(uv.x);
            float sin_2piu = glm::sin(uv.x);
            float cos_2piv = glm::cos(uv.y);
            float sin_2piv = glm::sin(uv.y);

            light_positions[i] = glm::vec3((R + r * cos_2piu) * cos_2piv,
                                           (R + r * cos_2piu) * sin_2piv, r * sin_2piu);
            uni_lr_color = light_colors[i] / luma;
            uni_lr_model_matrix = glm::scale(glm::translate(light_positions[i]), glm::vec3(0.225f));
            sphere.render();
        }

        //===============================================================================================================================================================================================================
        // render main balls
        //===============================================================================================================================================================================================================
        pbr_shader.enable();
        uni_pbr_pv_matrix = projection_view_matrix;
        uni_pbr_camera_ws = camera_ws;
        uni_pbr_light_positions = light_positions;

        int nrRows = 16;
        int nrColumns = 16;
        float spacing = 2.5f;

        for (int row = 0; row < nrRows; ++row)
        {
            for (int col = 0; col < nrColumns; ++col)
            {
                uni_pbr_metallic_factor  = 0.0625f * float(row);
                uni_pbr_roughness_factor = 0.0625f * float(col);

                glm::vec3 shift = spacing * glm::vec3(col - 0.5f * (nrColumns - 1.0f), row - 0.5f * (nrRows - 1.0f), 0.0f);
                glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), shift);
                uni_pbr_model_matrix = model_matrix;
                sphere.render();
            }
        }

        window.end_frame();
    }

    glfw::terminate();
    return 0;
}
