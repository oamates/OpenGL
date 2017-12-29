//========================================================================================================================================================================================================================
// DEMO 047: Raytrace using cubemaps
//========================================================================================================================================================================================================================
#include <random>
#include <cstdlib>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_NO_CTOR_INIT
#define USE_DSA_UNIFORMS

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"
#include "pipeline.hpp"
#include "camera.hpp"
#include "image.hpp"
#include "fbo.hpp"
#include "vao.hpp"
#include "vertex.hpp"
#include "sampler.hpp"
#include "image/stb_image.h"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
          camera(64.0f, 0.5f, glm::lookAt(glm::vec3(8.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
    {
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
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

struct sphere_pft2_t
{
    vao_t vao;                              // full position + frame + uv-texture vertex array object
    GLuint vao_id;                          // minimalistic position only vao, which uses same array and index buffers

    sphere_pft2_t(int res_x, int res_y)
    {
        int V = (res_x + 1) * (res_y + 1);
        int I = (2 * res_x + 3) * res_y;

        vertex_pft2_t* vertices = (vertex_pft2_t*) malloc(V * sizeof(vertex_pft2_t));
        GLuint* indices = (GLuint*) malloc (I * sizeof(GLuint));

        float inv_res_x = 1.0f / res_x;
        float inv_res_y = 1.0f / res_y;

        int vindex = 0;
        for (int y = 0; y <= res_y; ++y)
        {
            for (int x = 0; x <= res_x; ++x)
            {
                glm::vec2 uv = glm::vec2(inv_res_x * x, inv_res_y * y);

                float u = constants::two_pi * uv.x;
                float v = constants::pi * (uv.y - 0.5f);

                float cs_u = glm::cos(u);
                float sn_u = glm::sin(u);
                float r = glm::cos(v);
                float z = glm::sin(v);

                glm::vec3 position = glm::vec3(r * cs_u, r * sn_u, z);
                glm::vec3 normal = position;
                glm::vec3 tangent_u = glm::vec3(-sn_u, cs_u, 0.0f);
                glm::vec3 tangent_v = glm::cross(normal, tangent_u);

                vertices[vindex++] = vertex_pft2_t(position, normal, tangent_u, tangent_v, uv);
            }
        }

        int i = 0;
        for (int y = 0; y < res_y; ++y)
        {
            for (int x = 0; x <= res_x; ++x)
            {
                indices[i++] = (y + 1) * (res_x + 1) + x;
                indices[i++] = y * (res_x + 1) + x;
            }
            indices[i++] = -1;
        }

        vao = vao_t(GL_TRIANGLE_STRIP, vertices, V, indices, I);
        free(vertices);
        free(indices);
    }

    void render()
        { vao.render(); }
};

struct plane_pft2_t
{
    vao_t vao_pft2;                         // full position + frame + uv-texture vertex array object
    GLuint vao_id;                          // minimalistic position only vao, which uses same array and index buffers
    glm::vec2 scale;
    glm::vec2 tex_scale;

    plane_pft2_t (const glm::vec2& scale, const glm::vec2& tex_scale)
        : scale(scale), tex_scale(tex_scale)
    {

        glm::vec2 unit_square[] =
        {
            glm::vec2(-0.5f,  0.5f),
            glm::vec2(-0.5f, -0.5f),
            glm::vec2( 0.5f,  0.5f),
            glm::vec2( 0.5f, -0.5f)
        };

        glm::vec3 normal    = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 tangent_u = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 tangent_v = glm::vec3(0.0f, 1.0f, 0.0f);

        vertex_pft2_t vertices[] =
        {
            vertex_pft2_t(glm::vec3(scale * unit_square[0], 0.0f), normal, tangent_u, tangent_v, tex_scale * unit_square[0]),
            vertex_pft2_t(glm::vec3(scale * unit_square[1], 0.0f), normal, tangent_u, tangent_v, tex_scale * unit_square[1]),
            vertex_pft2_t(glm::vec3(scale * unit_square[2], 0.0f), normal, tangent_u, tangent_v, tex_scale * unit_square[2]),
            vertex_pft2_t(glm::vec3(scale * unit_square[3], 0.0f), normal, tangent_u, tangent_v, tex_scale * unit_square[3]),
        };

        GLuint indices[] = {0, 1, 2, 3};

        vao_pft2 = vao_t(GL_TRIANGLE_STRIP, vertices, 4, indices, 4);

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        glBindBuffer(GL_ARRAY_BUFFER, vao_pft2.vbo.id);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pft2_t), (const GLvoid*) offsetof(vertex_pft2_t, position));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao_pft2.ibo.id);
    }

    void render_pft2()
        { vao_pft2.render(); }

    void render()
    {
        glBindVertexArray(vao_id);
        vao_pft2.ibo.render();
    }

    glm::vec2 inv_scale()
    {
        return 2.0f / scale;
    }
};

const int SPHERE_COUNT = 16;

glm::vec3 sphere_positions[SPHERE_COUNT] =
{
    glm::vec3(-1.0f,-6.5f, 1.0f),
    glm::vec3( 3.0f, 6.5f, 1.0f),
    glm::vec3( 5.0f, 5.0f, 1.0f),
    glm::vec3( 3.0f,-1.0f, 1.0f),
    glm::vec3(-0.1f,-1.1f, 1.0f),
    glm::vec3(-3.0f, 3.0f, 1.0f),
    glm::vec3(-2.8f, 7.0f, 1.0f),
    glm::vec3(-1.1f, 9.0f, 1.0f),
    glm::vec3( 3.0f, 2.0f, 1.0f),
    glm::vec3(-7.0f, 3.0f, 1.0f),
    glm::vec3(-9.5f, 4.5f, 1.0f),
    glm::vec3( 1.0f, 5.2f, 1.0f),
    glm::vec3(-8.0f, 8.0f, 1.0f),
    glm::vec3(-5.0f, 1.0f, 1.0f),
    glm::vec3( 2.0f, 9.0f, 1.0f),
    glm::vec3( 8.0f, 7.5f, 1.0f)
};

glm::vec3 sphere_rotations[SPHERE_COUNT] =
{
    glm::vec3(-0.4f, 0.1f,-0.7f),
    glm::vec3( 0.3f,-0.2f,-0.1f),
    glm::vec3( 0.2f, 0.3f, 0.4f),
    glm::vec3(-0.4f,-0.4f, 0.2f),
    glm::vec3( 0.2f, 0.3f,-0.4f),
    glm::vec3(-0.7f,-0.2f, 0.6f),
    glm::vec3( 0.3f, 0.3f, 0.2f),
    glm::vec3( 0.5f, 0.2f, 0.3f),
    glm::vec3(-0.4f, 0.4f,-0.4f),
    glm::vec3( 0.3f,-0.3f, 0.1f),
    glm::vec3( 0.1f,-0.2f,-0.2f),
    glm::vec3(-0.2f,-0.3f,-0.0f),
    glm::vec3(-0.3f, 0.5f, 0.3f),
    glm::vec3(-0.4f, 0.1f, 0.1f),
    glm::vec3( 0.3f, 0.3f,-0.2f),
    glm::vec3(-0.2f,-0.2f, 0.4f)
};

GLuint generate_sphere_texture_array(GLenum texture_unit, int max_level)
{
    int tex_res_x = 1 << max_level;
    int tex_res_y = 1 << (max_level - 1);

    GLuint balls_tex_id;                                                            /* texture 2D array to store ball albedo textures */
    glActiveTexture(texture_unit);
    glGenTextures(1, &balls_tex_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, balls_tex_id);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, max_level + 1, GL_RGB8, tex_res_x, tex_res_y, SPHERE_COUNT);

    glsl_program_t sphere_texgen(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/sphere_texgen.fs"));
    sphere_texgen.enable();
    sphere_texgen["albedo_tex"] = 0;
    sphere_texgen["roughness_tex"] = 1;

    sampler_t mipmap_albedo_sampler(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);
    mipmap_albedo_sampler.bind(0);
    sampler_t mipmap_roughness_sampler(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);
    mipmap_roughness_sampler.bind(1);

    GLuint fbo_id;
    glGenFramebuffers(1, &fbo_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);

    GLuint albedo_tex_id, roughness_tex_id;
    glGenTextures(1, &albedo_tex_id);
    glGenTextures(1, &roughness_tex_id);

    glViewport(0, 0, tex_res_x, tex_res_y);

    GLuint vao_id;                                                                  /* attribute-less VAO for quad rendering */
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    const char* albedo_tex_name_pattern = "../../../resources/tex2d/bbal_hires/bbal_0x%01X.jpg";
    const char* roughness_tex_name_pattern = "../../../resources/tex2d/scratches/scratches_0x%01X.jpg";

    for (int i = 0; i != SPHERE_COUNT; ++i)
    {
        debug_msg("Generating sphere albedo texture #%d ... ", i);
        glFramebufferTexture3D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_ARRAY, balls_tex_id, 0, i);

        int width, height, bpp;
        unsigned char* src_data;
        GLenum format;
        char tex_name[64];

        sprintf(tex_name, albedo_tex_name_pattern, i);                              /* load albedo image */
        src_data = stbi_load(tex_name, &width, &height, &bpp, 0);

        if (!src_data)
            exit_msg("Cannot load texture: %s", tex_name);

        format = (bpp == 1) ? GL_RED : (bpp == 2) ? GL_RG : (bpp == 3) ? GL_RGB : GL_RGBA;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedo_tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, src_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        free(src_data);

        sprintf(tex_name, roughness_tex_name_pattern, i);                           /* load roughness image */
        src_data = stbi_load(tex_name, &width, &height, &bpp, 0);

        if (!src_data)
            exit_msg("Cannot load texture: %s", tex_name);

        format = (bpp == 1) ? GL_RED : (bpp == 2) ? GL_RG : (bpp == 3) ? GL_RGB : GL_RGBA;
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, roughness_tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, src_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        free(src_data);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      /* combine images into one */
        debug_msg(" ... done.");
    }

    GLuint tex_ids[] = {albedo_tex_id, roughness_tex_id};
    glDeleteTextures(2, tex_ids);
    glDeleteFramebuffers(1, &fbo_id);
    glDeleteVertexArrays(1, &vao_id);

    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
    glActiveTexture(texture_unit);                                                  /* generate mipmaps of texture array */
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    GLsizei pixel_buffer_size = 3 * tex_res_x * tex_res_y;                          /* store images for debug purposes */
    unsigned char* pixel_buffer = (unsigned char*) malloc(pixel_buffer_size);
    for (int layer = 0; layer != SPHERE_COUNT; ++layer)
    {
        int res_x = tex_res_x;
        int res_y = tex_res_y;
        for (int level = 0; level <= max_level; ++level)
        {
            char tex_name[64];
            sprintf(tex_name, "bbal_0x%01X_L%01X.png", layer, level);
            GLsizei buffer_size = 3 * res_x * res_y;
            glGetTextureSubImage(balls_tex_id, level, 0, 0, layer, res_x, res_y, 1, GL_RGB, GL_UNSIGNED_BYTE, buffer_size, pixel_buffer);
            image::png::write(tex_name, res_x, res_y, pixel_buffer, PNG_COLOR_TYPE_RGB);
            res_x = glm::max(res_x >> 1, 1);
            res_y = glm::max(res_y >> 1, 1);
        }
    }
    free(pixel_buffer);
}

const glm::vec3 light_position = glm::vec3(0.0f, 20.0f, -2.0f);

//=======================================================================================================================================================================================================================
// Texture unit usage:
//   unit 0, unit 1 -- working auxiliary units
//   unit 2         -- lightmap texture
//   unit 3         -- plane albedo texture
//   unit 4         -- sphere albedo texture (GL_TEXTURE_2D_ARRAY)
//   unit 5         -- reflection cubemap texture
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    const int res_x = 1920;
    const int res_y = 1080;

    //===================================================================================================================================================================================================================
    // Step 0. initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 4.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Cubemap raytrace", 8, 4, 3, res_x, res_y, true);

    const float scale = 16.0f;                                                      /* generate VAO for plane and sphere */
    sphere_pft2_t sphere(48, 24);
    plane_pft2_t plane(glm::vec2(48.0f, 32.0f), glm::vec2(24.0f, 16.0f));

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);                                          /* necessary for glTexImage2D/glGetTexImage2D */
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(-1);

    //===================================================================================================================================================================================================================
    // Step 1: init shaders and separable shader programs
    //===================================================================================================================================================================================================================
    glsl_shader_t common_vs(GL_VERTEX_SHADER, "glsl/common.vs");
    glsl_shader_program_t common_vs_program(common_vs);
    dsa_uniform_t uni_cv_model_matrix    = common_vs_program["model_matrix"];
    dsa_uniform_t uni_cv_camera_position = common_vs_program["camera_ws"];
    dsa_uniform_t uni_cv_light_position  = common_vs_program["light_ws"];
    uni_cv_light_position = light_position;

    glsl_shader_t default_gs(GL_GEOMETRY_SHADER, "glsl/default.gs");
    glsl_shader_program_t default_gs_program(default_gs);
    dsa_uniform_t uni_dg_projection_matrix = default_gs_program["projection_matrix"];
    dsa_uniform_t uni_dg_camera_matrix = default_gs_program["view_matrix"];

    glsl_shader_t cubemap_gs(GL_GEOMETRY_SHADER, "glsl/cubemap.gs");
    glsl_shader_program_t cubemap_gs_program(cubemap_gs);
    dsa_uniform_t uni_cg_projection_matrix = cubemap_gs_program["projection_matrix"];
    dsa_uniform_t uni_cg_camera_matrix = cubemap_gs_program["view_matrix"];

    glsl_shader_t cloth_fs(GL_FRAGMENT_SHADER, "glsl/cloth.fs");
    glsl_shader_program_t cloth_fs_program(cloth_fs);
    dsa_uniform_t uni_cf_light_map = cloth_fs_program["light_map"];
    dsa_uniform_t uni_cf_cloth_tex = cloth_fs_program["cloth_tex"];
    uni_cf_light_map = 2;
    uni_cf_cloth_tex = 3;

    glsl_shader_t ball_simple_fs(GL_FRAGMENT_SHADER, "glsl/ball_simple.fs");
    glsl_shader_program_t ball_simple_fs_program(ball_simple_fs);
    dsa_uniform_t uni_bsf_albedo_tex  = ball_simple_fs_program["albedo_tex"];
    dsa_uniform_t uni_bsf_ball_idx    = ball_simple_fs_program["ball_idx"];
    uni_bsf_albedo_tex = 4;

    glsl_shader_t ball_reflect_fs(GL_FRAGMENT_SHADER, "glsl/ball_reflect.fs");
    glsl_shader_program_t ball_reflect_fs_program(ball_reflect_fs);
    dsa_uniform_t uni_brf_albedo_tex  = ball_reflect_fs_program["albedo_tex"];
    dsa_uniform_t uni_brf_reflect_tex = ball_reflect_fs_program["reflect_tex"];
    dsa_uniform_t uni_brf_ball_idx    = ball_reflect_fs_program["ball_idx"];
    uni_brf_albedo_tex = 4;
    uni_brf_reflect_tex = 5;

    //===================================================================================================================================================================================================================
    // Step 2: pre-generate lightmap texture
    //===================================================================================================================================================================================================================
    debug_msg("Precomputing lightmap texture.");

    const int LIGHT_MAP_RESOLUTION = 1024;

    GLuint light_map_tex_id;
    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &light_map_tex_id);
    glBindTexture(GL_TEXTURE_2D, light_map_tex_id);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, LIGHT_MAP_RESOLUTION, LIGHT_MAP_RESOLUTION);

    GLuint lightmap_fbo_id;                                                         /* framebuffer for lightmap rendering */
    glGenFramebuffers(1, &lightmap_fbo_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lightmap_fbo_id);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_map_tex_id, 0);

    glViewport(0, 0, LIGHT_MAP_RESOLUTION, LIGHT_MAP_RESOLUTION);
    glClearColor(1.0 ,1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glsl_program_t lightmap(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/lightmap.vs"),
                            glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/lightmap.fs"));
    lightmap.enable();
    lightmap["light_ws"]         = light_position;
    lightmap["sphere_positions"] = sphere_positions;
    lightmap["inv_scale"]        = plane.inv_scale();

    glDisable(GL_DEPTH_TEST);
    plane.render();

    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);

    GLsizei pixel_buffer_size = 3 * LIGHT_MAP_RESOLUTION * LIGHT_MAP_RESOLUTION;            /* store lightmap for debug purpose */
    unsigned char* pixel_buffer = (unsigned char*) malloc(pixel_buffer_size);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_buffer);
    image::png::write("lightmap.png", LIGHT_MAP_RESOLUTION, LIGHT_MAP_RESOLUTION, pixel_buffer, PNG_COLOR_TYPE_RGB);
    free(pixel_buffer);
    debug_msg("Lightmap texture saved.");

    //===================================================================================================================================================================================================================
    // Step 3: load plain texture and create a mipmap sampler for it
    //===================================================================================================================================================================================================================
    GLuint plane_albedo_tex_id;
    glActiveTexture(GL_TEXTURE3);
    glGenTextures(1, &plane_albedo_tex_id);
    glBindTexture(GL_TEXTURE_2D, plane_albedo_tex_id);

    const char* cloth_tex_name = "../../../resources/tex2d/cloth_seamless.png";
    int width, height, bpp;
    unsigned char* src_data = stbi_load(cloth_tex_name, &width, &height, &bpp, 0);

    if (!src_data)
        exit_msg("Cannot load texture: %s", cloth_tex_name);

    GLenum format = (bpp == 1) ? GL_RED : (bpp == 2) ? GL_RG : (bpp == 3) ? GL_RGB : GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, src_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    free(src_data);

    sampler_t plane_albedo_sampler(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT);
    plane_albedo_sampler.bind(3);

    debug_msg("Plane albedo texture loaded.");

    //===================================================================================================================================================================================================================
    // Step 4: generate scratched billiard ball textures from albedo (numbers) and roughness textures, making spherical distortion
    // and pack them into a single GL_TEXTURE_2D_ARRAY
    // texture is twice wider because equator on sphere is twice longer than meridian
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE4);
    GLuint sphere_albedo_tex_id = generate_sphere_texture_array(GL_TEXTURE4, 10);
    sampler_t sphere_albedo_sampler(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_CLAMP_TO_EDGE);
    sphere_albedo_sampler.bind(4);

    //===================================================================================================================================================================================================================
    // Step 5: create and prerender reflection cubemaps texture (unit 5)
    //===================================================================================================================================================================================================================
    GLuint cubemap_side = 256;
    GLuint reflect_tex_id[SPHERE_COUNT];
    glActiveTexture(GL_TEXTURE5);
    glGenTextures(SPHERE_COUNT, reflect_tex_id);

    sampler_t reflection_tex_sampler(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    reflection_tex_sampler.bind(5);

    for(GLuint b = 0; b != SPHERE_COUNT; ++b)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, reflect_tex_id[b]);
        glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, cubemap_side, cubemap_side);
    }

    GLuint depth_tex_id;                                                                    /* auxiliary depth texture */
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &depth_tex_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_tex_id);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_DEPTH_COMPONENT32, cubemap_side, cubemap_side);

    GLuint cubemap_fbo_id;
    glGenFramebuffers(1, &cubemap_fbo_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cubemap_fbo_id);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex_id, 0);

    glViewport(0, 0, cubemap_side, cubemap_side);

    glUseProgram(0);

    glsl_pipeline_t cubemap_plane_pipeline;
    cubemap_plane_pipeline.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    cubemap_plane_pipeline.add_stage(GL_GEOMETRY_SHADER_BIT, cubemap_gs_program);
    cubemap_plane_pipeline.add_stage(GL_FRAGMENT_SHADER_BIT, cloth_fs_program);

    glsl_pipeline_t cubemap_sphere_pipeline;
    cubemap_sphere_pipeline.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    cubemap_sphere_pipeline.add_stage(GL_GEOMETRY_SHADER_BIT, cubemap_gs_program);
    cubemap_sphere_pipeline.add_stage(GL_FRAGMENT_SHADER_BIT, ball_simple_fs_program);

    glClearColor(0.04f, 0.01f, 0.09f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    for(int b = 0; b != SPHERE_COUNT; ++b)
    {
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, reflect_tex_id[b], 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cubemap_plane_pipeline.bind();
        uni_cv_model_matrix = glm::mat4(1.0f);
        uni_cv_camera_position = sphere_positions[b];
        uni_cg_camera_matrix = glm::translate(-sphere_positions[b]);
        uni_cg_projection_matrix = glm::infinitePerspective(constants::half_pi, 1.0f, 0.25f);

        plane.render_pft2();

        cubemap_sphere_pipeline.bind();

        for(int i = 0; i != SPHERE_COUNT; ++i)
        {
            if (i == b) continue;

            glm::vec3 rot = sphere_rotations[i];
            uni_cv_model_matrix = glm::translate(glm::eulerAngleYXZ (rot.y, rot.x, rot.z), sphere_positions[i]);
            uni_bsf_ball_idx = float(i);
            sphere.render();
        }
    }

    //===================================================================================================================================================================================================================
    // Step 6: create pipeline objects for the main render loop
    //===================================================================================================================================================================================================================
    glsl_pipeline_t plane_pipeline;
    plane_pipeline.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    plane_pipeline.add_stage(GL_GEOMETRY_SHADER_BIT, default_gs_program);
    plane_pipeline.add_stage(GL_FRAGMENT_SHADER_BIT, cloth_fs_program);

    glsl_pipeline_t ball_pipeline;
    ball_pipeline.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    ball_pipeline.add_stage(GL_GEOMETRY_SHADER_BIT, default_gs_program);
    ball_pipeline.add_stage(GL_FRAGMENT_SHADER_BIT, ball_reflect_fs_program);

    //===================================================================================================================================================================================================================
    // Step 7: main loop
    //===================================================================================================================================================================================================================
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, res_x, res_y);

    while(!window.should_close())
    {
        window.new_frame();

        double time = window.frame_ts;
        glm::mat4 camera = window.camera.view_matrix;
        glm::vec3 camera_position = window.camera.position();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //===============================================================================================================================================================================================================
        // render billiard table
        //===============================================================================================================================================================================================================
        plane_pipeline.bind();
        uni_cv_camera_position = camera_position;
        uni_cv_model_matrix = glm::mat4(1.0f);
        uni_dg_projection_matrix = window.camera.projection_matrix;
        uni_dg_camera_matrix = camera;

        plane.render_pft2();

        //===============================================================================================================================================================================================================
        // render balls
        //===============================================================================================================================================================================================================
        ball_pipeline.bind();

        for(int i = 0; i != SPHERE_COUNT; ++i)
        {
            glm::vec3 rot = sphere_rotations[i];
            uni_cv_model_matrix = glm::translate(sphere_positions[i]) * glm::eulerAngleYXZ (rot.y, rot.x, rot.z);
            uni_brf_ball_idx = float(i);
            glBindTexture(GL_TEXTURE_CUBE_MAP, reflect_tex_id[i]);
            sphere.render();
        }

        //===============================================================================================================================================================================================================
        // show back buffer
        //===============================================================================================================================================================================================================
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}
