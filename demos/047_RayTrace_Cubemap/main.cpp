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
#include "camera.hpp"
#include "image.hpp"
#include "fbo.hpp"
#include "vao.hpp"
#include "vertex.hpp"
#include "sampler.hpp"
#include "image/stb_image.h"

const int BALL_COUNT = 16;

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
          camera(8.0f, 0.5f, glm::lookAt(glm::vec3(4.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
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

struct glsl_pipeline_t
{
    GLuint id;

    glsl_pipeline_t()
        { glGenProgramPipelines(1, &id); }

    void bind()
        { glBindProgramPipeline(id); }

    void active_shader_program(const glsl_shader_program_t& shader_program)
        { glActiveShaderProgram(id, shader_program.id); }

    void add_stage(GLbitfield stage_bitmask, const glsl_program_t& program)
        { glUseProgramStages(id, stage_bitmask, program.id); }

    void add_stage(GLbitfield stage_bitmask, const glsl_shader_program_t& shader_program)
        { glUseProgramStages(id, stage_bitmask, shader_program.id); }

    ~glsl_pipeline_t()
        { glDeleteProgramPipelines(1, &id); }
};

struct sphere_pft2_t
{
    vao_t vao_pft2;                         // full position + frame + uv-texture vertex array object
    GLuint vao_id;                          // minimalistic position only vao, which uses same array and index buffers

    sphere_pft2_t(int res_x, int res_y)
    {
        int V = (res_x + 1) * (res_y + 1);
        int I = 2 * (res_x + 1) * res_y;

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
                float v = constants::half_pi * (uv.y - 0.5f);

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

        bool even_row = true;
        int i = 0;
        for (int y = 0; y < res_y; ++y)
        {
            if (even_row)
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

        vao_pft2 = vao_t(GL_TRIANGLE_STRIP, vertices, V, indices, I);

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        glBindBuffer(GL_ARRAY_BUFFER, vao_pft2.vbo.id);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pft2_t), (const GLvoid*) offsetof(vertex_pft2_t, position));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao_pft2.ibo.id);

        free(vertices);
        free(indices);
    }

    void render_pft2()
        { vao_pft2.render(); }

    void render()
    {
        glBindVertexArray(vao_id);
        vao_pft2.ibo.render();
    }
};

struct plane_pft2_t
{
    vao_t vao_pft2;                         // full position + frame + uv-texture vertex array object
    GLuint vao_id;                          // minimalistic position only vao, which uses same array and index buffers

    plane_pft2_t (float scale)
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
            vertex_pft2_t(glm::vec3(scale * unit_square[0], 0.0f), normal, tangent_u, tangent_v, unit_square[0] + glm::vec2(0.5f, 0.5f)),
            vertex_pft2_t(glm::vec3(scale * unit_square[1], 0.0f), normal, tangent_u, tangent_v, unit_square[1] + glm::vec2(0.5f, 0.5f)),
            vertex_pft2_t(glm::vec3(scale * unit_square[2], 0.0f), normal, tangent_u, tangent_v, unit_square[2] + glm::vec2(0.5f, 0.5f)),
            vertex_pft2_t(glm::vec3(scale * unit_square[3], 0.0f), normal, tangent_u, tangent_v, unit_square[3] + glm::vec2(0.5f, 0.5f)),
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
};

int main(int argc, char *argv[])
{
    const int res_x = 1920;
    const int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("VAO Loader", 8, 4, 3, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // init shaders
    //===================================================================================================================================================================================================================
    glsl_shader_t common_vs(GL_VERTEX_SHADER, "glsl/common.vs");                // CommonVertShader
    glsl_shader_program_t common_vs_program(common_vs);                         // VertexProgram, vert_prog
    uniform_t uni_cv_model_matrix    = common_vs_program["ModelMatrix"];        // ProgramUniform<Mat4f> model_matrix
    uniform_t uni_cv_texture_matrix  = common_vs_program["TextureMatrix"];      // ProgramUniform<Mat3f> texture_matrix
    uniform_t uni_cv_camera_position = common_vs_program["CameraPosition"];     // ProgramUniform<Vec3f> camera_position
    uniform_t uni_cv_light_position  = common_vs_program["LightPosition"];      // ProgramUniform<Vec3f> light_position

    glsl_shader_t default_gs(GL_GEOMETRY_SHADER, "glsl/default.gs");            // DefaultGeomShader
    glsl_shader_program_t default_gs_program(default_gs);                       // GeometryProgram, geom_prog
    uniform_t uni_dg_projection_matrix = default_gs_program["ProjectionMatrix"];
    uniform_t uni_dg_camera_matrix = default_gs_program["CameraMatrix"];

    glsl_shader_t cubemap_gs(GL_GEOMETRY_SHADER, "glsl/cubemap.gs");            // CubemapGeomShader, cmap_geom_shader
    glsl_shader_program_t cubemap_gs_program(cubemap_gs);                       // GeometryProgram, cmap_geom_prog
    uniform_t uni_cg_projection_matrix = cubemap_gs_program["ProjectionMatrix"];
    uniform_t uni_cg_camera_matrix = cubemap_gs_program["CameraMatrix"];

    glsl_shader_t cloth_fs(GL_FRAGMENT_SHADER, "glsl/cloth.fs");                // ClothFragmentShader
    glsl_shader_program_t cloth_fs_program(cloth_fs);                           // ClothProgram, cloth_prog
    uniform_t uni_cf_color_1   = cloth_fs_program["color_1"];                    // ProgramUniform<Vec3f> color_1
    uniform_t uni_cf_color_2   = cloth_fs_program["color_2"];                    // ProgramUniform<Vec3f> color_2
    uniform_t uni_cf_cloth_tex = cloth_fs_program["ClothTex"];                  // ProgramUniformSampler cloth_tex;
    uniform_t uni_cf_light_map = cloth_fs_program["LightMap"];                  // ProgramUniformSampler light_map;

    glsl_shader_t ball_fs(GL_FRAGMENT_SHADER, "glsl/ball.fs");                  // BallFragmentShader
    glsl_shader_program_t ball_fs_program(ball_fs);                             // BallProgram, ball_prog
    uniform_t uni_bf_color_1     = ball_fs_program["color_1"];                  // ProgramUniform<Vec3f> color_1
    uniform_t uni_bf_color_2     = ball_fs_program["color_2"];                  // ProgramUniform<Vec3f> color_2
    uniform_t uni_bf_number_tex  = ball_fs_program["number_tex"];               // ProgramUniformSampler number_tex
    uniform_t uni_bf_reflect_tex = ball_fs_program["reflect_tex"];              // ProgramUniformSampler reflect_tex
    uniform_t uni_bf_ball_idx    = ball_fs_program["ball_idx"];                 // ProgramUniform<GLint> ball_idx

    glsl_program_t lightmap(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/lightmap.vs"),  // LightmapProgram prog
                            glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/lightmap.fs"));
    uniform_t uni_lm_transform_matrix = lightmap["TransformMatrix"];            // ProgramUniform<Mat4f> transform_matrix
    uniform_t uni_lm_light_position   = lightmap["LightPosition"];              // ProgramUniform<Vec3f> light_position
    uniform_t uni_lm_ball_positions   = lightmap["BallPositions"];              // ProgramUniform<Vec3f> ball_positions

    //===================================================================================================================================================================================================================
    // generate scratched billiard ball textures form albedo (numbers) and roughness textures, making spherical distortion
    // and pack them into a single GL_TEXTURE_2D_ARRAY
    // texture is twice wider because equator on sphere is twice longer than meridian
    //===================================================================================================================================================================================================================
    const int BALLS_TEXTURE_MAX_LEVEL = 10;
    const int BALLS_TEXTURE_WIDTH = 1 << BALLS_TEXTURE_MAX_LEVEL;
    const int BALLS_TEXTURE_HEIGHT = 1 << (BALLS_TEXTURE_MAX_LEVEL - 1);

    GLuint balls_tex_id;                                                                        // GLuint numbers_texture;
    glGenTextures(1, &balls_tex_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, balls_tex_id);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, BALLS_TEXTURE_MAX_LEVEL + 1, GL_RGB8, BALLS_TEXTURE_WIDTH, BALLS_TEXTURE_HEIGHT, BALL_COUNT);

    glsl_program_t sphere_texgen(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/sphere_texgen.fs"));
    sphere_texgen.enable();
    sphere_texgen["albedo_tex"] = 0;
    sphere_texgen["roughness_tex"] = 1;

    sampler_t mipmap_albedo_sampler(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);
    mipmap_albedo_sampler.bind(0);
    sampler_t mipmap_roughness_sampler(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);
    mipmap_roughness_sampler.bind(1);

    GLuint texgen_fbo_id;
    glGenFramebuffers(1, &texgen_fbo_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, texgen_fbo_id);

    GLuint albedo_tex_id, roughness_tex_id;
    glGenTextures(1, &albedo_tex_id);
    glGenTextures(1, &roughness_tex_id);

    glViewport(0, 0, BALLS_TEXTURE_WIDTH, BALLS_TEXTURE_HEIGHT);

    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    //===================================================================================================================================================================================================================
    // texture generation step
    //===================================================================================================================================================================================================================
    const char* albedo_tex_name_pattern = "../../../resources/tex2d/bbal_hires/bbal_0x%01X.jpg";
    const char* roughness_tex_name_pattern = "../../../resources/tex2d/scratches/scratches_0x%01X.jpg";

    for (int i = 0; i != BALL_COUNT; ++i)
    {
        debug_msg("Render step #%d.", i); fflush(stdout);
        glFramebufferTexture3D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_ARRAY, balls_tex_id, 0, i);
        check_status();

        int width, height, bpp;
        unsigned char* src_data;
        GLenum format;
        char tex_name[64];

        //===============================================================================================================================================================================================================
        // load albedo image
        //===============================================================================================================================================================================================================
        sprintf(tex_name, albedo_tex_name_pattern, i);
        src_data = stbi_load(tex_name, &width, &height, &bpp, 0);

        if (!src_data)
            exit_msg("Cannot load texture: %s", tex_name);

        format = (bpp == 1) ? GL_RED : (bpp == 2) ? GL_RG : (bpp == 3) ? GL_RGB : GL_RGBA;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedo_tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, src_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        free(src_data);

        //===============================================================================================================================================================================================================
        // load albedo image
        //===============================================================================================================================================================================================================
        sprintf(tex_name, roughness_tex_name_pattern, i);
        src_data = stbi_load(tex_name, &width, &height, &bpp, 0);

        if (!src_data)
            exit_msg("Cannot load texture: %s", tex_name);

        format = (bpp == 1) ? GL_RED : (bpp == 2) ? GL_RG : (bpp == 3) ? GL_RGB : GL_RGBA;

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, roughness_tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, src_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        free(src_data);

        //===============================================================================================================================================================================================================
        // combine images into one
        //===============================================================================================================================================================================================================
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    //===================================================================================================================================================================================================================
    // generate mipmaps of texture array
    //===================================================================================================================================================================================================================
    GLuint tex_ids[] = {albedo_tex_id, roughness_tex_id};
    glDeleteTextures(2, tex_ids);

    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, balls_tex_id);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    //===================================================================================================================================================================================================================
    // store images for debug purposes
    //===================================================================================================================================================================================================================
    GLsizei pixel_buffer_size = 3 * BALLS_TEXTURE_WIDTH * BALLS_TEXTURE_HEIGHT;
    unsigned char* pixel_buffer = (unsigned char*) malloc(pixel_buffer_size);
    for (int layer = 0; layer != BALL_COUNT; ++layer)
    {
        int res_x = BALLS_TEXTURE_WIDTH;
        int res_y = BALLS_TEXTURE_HEIGHT;
        for (int level = 0; level <= BALLS_TEXTURE_MAX_LEVEL; ++level)
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

    //===================================================================================================================================================================================================================
    // precompute step
    //===================================================================================================================================================================================================================
    const float scale = 16.0f;
    sphere_pft2_t sphere(48, 24);
    plane_pft2_t plane(scale);

    const GLuint ball_count = BALL_COUNT;

    glm::vec3 ball_colors[] =
    {
        glm::vec3(0.8f, 0.5f, 0.2f),
        glm::vec3(0.2f, 0.2f, 0.5f),
        glm::vec3(0.6f, 0.2f, 0.4f),
        glm::vec3(0.1f, 0.1f, 0.3f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.3f, 0.1f, 0.2f),
        glm::vec3(0.2f, 0.5f, 0.2f),
        glm::vec3(0.6f, 0.3f, 0.2f)
    };

    glm::vec3 ball_offsets[] =
    {
        glm::vec3(-1.0f, 1.0f,-6.5f),
        glm::vec3( 3.0f, 1.0f, 6.5f),
        glm::vec3( 5.0f, 1.0f, 5.0f),
        glm::vec3( 3.0f, 1.0f,-1.0f),
        glm::vec3(-0.1f, 1.0f,-1.1f),
        glm::vec3(-3.0f, 1.0f, 3.0f),
        glm::vec3(-2.8f, 1.0f, 7.0f),
        glm::vec3(-1.1f, 1.0f, 9.0f),
        glm::vec3( 3.0f, 1.0f, 2.0f),
        glm::vec3(-7.0f, 1.0f, 3.0f),
        glm::vec3(-9.5f, 1.0f, 4.5f),
        glm::vec3( 1.0f, 1.0f, 5.2f),
        glm::vec3(-8.0f, 1.0f, 8.0f),
        glm::vec3(-5.0f, 1.0f, 1.0f),
        glm::vec3( 2.0f, 1.0f, 9.0f),
        glm::vec3( 8.0f, 1.0f, 7.5f)
    };

    glm::vec3 ball_rotations[] =
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

    static_assert(sizeof(ball_offsets) == ball_count * sizeof(glm::vec3));
    static_assert(sizeof(ball_rotations) == ball_count * sizeof(glm::vec3));

    glUseProgram(0);

    glsl_pipeline_t cloth_pp;
    cloth_pp.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    cloth_pp.add_stage(GL_GEOMETRY_SHADER_BIT, default_gs_program);
    cloth_pp.add_stage(GL_FRAGMENT_SHADER_BIT, cloth_fs_program);
    cloth_pp.bind();

    const glm::vec3 light_position = glm::vec3(0.0f, 20.0f, -2.0f);

    cloth_pp.active_shader_program(common_vs_program);
    uni_cv_light_position = light_position;                             // vert_prog.light_position.Set(light_position);
    cloth_pp.active_shader_program(cloth_fs_program);
    uni_cf_color_1 = glm::vec3(0.1f, 0.3f, 0.1f);                       // cloth_prog.color_1.Set(Vec3f(0.1f, 0.3f, 0.1f));
    uni_cf_color_2 = glm::vec3(0.3f, 0.4f, 0.3f);                       // cloth_prog.color_2.Set(Vec3f(0.3f, 0.4f, 0.3f));
    uni_cf_light_map = 0;                                               // cloth_prog.light_map.Set(0);
    uni_cf_cloth_tex = 2;

    glsl_pipeline_t ball_pp;
    ball_pp.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    ball_pp.add_stage(GL_GEOMETRY_SHADER_BIT, default_gs_program);
    ball_pp.add_stage(GL_FRAGMENT_SHADER_BIT, ball_fs_program);
    ball_pp.bind();
    ball_pp.active_shader_program(ball_fs_program);
    uni_bf_number_tex = 1;                                              // ball_prog.number_tex.Set(1)
    uni_bf_reflect_tex = 3;

    lightmap.enable();
    glActiveTexture(GL_TEXTURE0);

    GLuint table_light_map;                                             // Texture table_light_map; -- table light map and the associated framebuffer
    glGenTextures(1, &table_light_map);
    const int LIGHT_MAP_RESOLUTION = 1024;
    glBindTexture(GL_TEXTURE_2D, table_light_map);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, LIGHT_MAP_RESOLUTION, LIGHT_MAP_RESOLUTION);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint table_light_fbo_id;                                          // Framebuffer table_light_fbo_id;
    glGenFramebuffers(1, &table_light_fbo_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, table_light_fbo_id);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, table_light_map, 0);

    glViewport(0, 0, LIGHT_MAP_RESOLUTION, LIGHT_MAP_RESOLUTION);
    glClearColor(1.0 ,1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    GLfloat i_u = 1.0f / (scale * scale);
    GLfloat i_v = 1.0f / (scale * scale);

    glm::mat4 transform_matrix = glm::mat4(glm::vec4(0.5f / scale, 0.0f,         0.0f, 0.0f),
                                           glm::vec4(0.0f,         0.5f / scale, 0.0f, 0.0f),
                                           glm::vec4(0.0f,         1.0f,         0.0f, 0.0f),
                                           glm::vec4(0.0f,         0.0f,         0.0f, 1.0f));

    uni_lm_transform_matrix = transform_matrix;
    uni_lm_light_position = light_position;
    uni_lm_ball_positions = ball_offsets;

    glDisable(GL_DEPTH_TEST);
    plane.render();

    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);

    //===================================================================================================================================================================================================================
    // store lightmap image for debug purposes
    //===================================================================================================================================================================================================================


    pixel_buffer_size = 3 * LIGHT_MAP_RESOLUTION * LIGHT_MAP_RESOLUTION;
    pixel_buffer = (unsigned char*) malloc(pixel_buffer_size);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_buffer);
    image::png::write("lightmap.png", LIGHT_MAP_RESOLUTION, LIGHT_MAP_RESOLUTION, pixel_buffer, PNG_COLOR_TYPE_RGB);
    free(pixel_buffer);
    debug_msg("Done saving lightmap texture."); fflush(stdout);

    glUseProgram(0);
    glClearColor(0.12f, 0.13f, 0.11f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, balls_tex_id);

    //===================================================================================================================================================================================================================
    // generate table cloth texture --> unit 2
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE2);
    GLuint cloth_texture;
    glGenTextures(1, &cloth_texture);
    glBindTexture(GL_TEXTURE_2D, cloth_texture);


    const char* cloth_tex_name = "../../../resources/tex2d/cloth.jpg";
    int width, height, bpp;
    unsigned char* src_data = stbi_load(cloth_tex_name, &width, &height, &bpp, 0);

    if (!src_data)
        exit_msg("Cannot load texture: %s", cloth_tex_name);

    GLenum format = (bpp == 1) ? GL_RED : (bpp == 2) ? GL_RG : (bpp == 3) ? GL_RGB : GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, src_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    free(src_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    debug_msg("Done."); fflush(stdout);

    //===================================================================================================================================================================================================================
    // prerender the cubemaps --> texture unit 3
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE3);
    GLuint cubemap_side = 128;
    GLuint reflect_textures[ball_count];
    GLuint temp_cubemaps[ball_count];

    glGenTextures(ball_count, reflect_textures);
    glGenTextures(ball_count, temp_cubemaps);

    sampler_t linear_reflect_sampler(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    mipmap_albedo_sampler.bind(3);
    sampler_t linear_cubemap_sampler(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    mipmap_roughness_sampler.bind(4);


    for(GLuint b = 0; b != ball_count; ++b)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, reflect_textures[b]);
        glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, cubemap_side, cubemap_side);
        glm::vec4 black_rgb = glm::vec4(0.0);
        glClearTexImage(reflect_textures[b], 0, GL_RGBA, GL_FLOAT, glm::value_ptr(black_rgb));

        glBindTexture(GL_TEXTURE_CUBE_MAP, temp_cubemaps[b]);
        glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, cubemap_side, cubemap_side);
        glClearTexImage(temp_cubemaps[b], 0, GL_RGBA, GL_FLOAT, glm::value_ptr(black_rgb));
    }

    //===================================================================================================================================================================================================================
    // prerender the cubemaps
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE4);
    GLuint z_buffer;

    glGenTextures(1, &z_buffer);
    glBindTexture(GL_TEXTURE_CUBE_MAP, z_buffer);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_DEPTH_COMPONENT32, cubemap_side, cubemap_side);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, z_buffer, 0);



    glUseProgram(0);

    glsl_pipeline_t cmap_cloth_pp;
    cmap_cloth_pp.bind();
    cmap_cloth_pp.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    cmap_cloth_pp.add_stage(GL_GEOMETRY_SHADER_BIT, cubemap_gs_program);
    cmap_cloth_pp.add_stage(GL_FRAGMENT_SHADER_BIT, cloth_fs_program);

    glsl_pipeline_t cmap_ball_pp;
    cmap_ball_pp.bind();
    cmap_ball_pp.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    cmap_ball_pp.add_stage(GL_GEOMETRY_SHADER_BIT, cubemap_gs_program);
    cmap_ball_pp.add_stage(GL_FRAGMENT_SHADER_BIT, ball_fs_program);

    glViewport(0, 0, cubemap_side, cubemap_side);

    glActiveTexture(GL_TEXTURE3);

    for(int b = 0; b != ball_count; ++b)
    {
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, reflect_textures[b], 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cmap_cloth_pp.bind();
        cmap_cloth_pp.active_shader_program(common_vs_program);
        uni_cv_camera_position = ball_offsets[b];
        uni_cv_texture_matrix = glm::mat3(glm::vec3(16.0f,  0.0f, 0.0f),
                                          glm::vec3( 0.0f, 16.0f, 0.0f),
                                          glm::vec3( 0.0f,  0.0f, 1.0f));

        cmap_cloth_pp.active_shader_program(cubemap_gs_program);
        uni_cg_camera_matrix = glm::translate(-ball_offsets[b]);
        uni_cg_projection_matrix = glm::infinitePerspective(constants::half_pi, 1.0f, 0.25f);

        plane.render_pft2();

        cmap_ball_pp.bind();
        cmap_ball_pp.active_shader_program(common_vs_program);

        uni_cv_texture_matrix = glm::mat3(glm::vec3(6.0f, 0.0f,  0.0f),
                                          glm::vec3(0.0f, 3.0f, -1.0f),
                                          glm::vec3(0.0f, 0.0f,  1.0f));

        for(int i = 0; i != ball_count; ++i)
        {
            if (i == b) continue;

            cmap_ball_pp.active_shader_program(common_vs_program);
            glm::vec3 rot = ball_rotations[i];
            uni_cv_model_matrix = glm::translate(glm::eulerAngleYXZ (rot.y, rot.x, rot.z), ball_offsets[i]);

            cmap_ball_pp.active_shader_program(ball_fs_program);
            uni_bf_color_1 = ball_colors[i];
            uni_bf_color_2 = ball_colors[i];
            uni_bf_ball_idx = i;

            glBindTexture(GL_TEXTURE_CUBE_MAP, temp_cubemaps[i]);

            sphere.render_pft2();
        }
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, res_x, res_y);

    debug_msg("Beginning main render loop.");

    //===================================================================================================================================================================================================================
    // program main loop
    //===================================================================================================================================================================================================================
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
        cloth_pp.bind();
        cloth_pp.active_shader_program(common_vs_program);
        uni_cv_camera_position = camera_position;
        uni_cv_model_matrix = glm::mat4(1.0f);
        glm::mat3 texture_matrix = glm::mat3(glm::vec3(16.0f,  0.0f, 0.0f),
                                             glm::vec3( 0.0f, 16.0f, 0.0f),
                                             glm::vec3( 0.0f,  0.0f, 1.0f));
        uni_cv_texture_matrix = texture_matrix;

        cloth_pp.active_shader_program(default_gs_program);
        uni_dg_projection_matrix = window.camera.projection_matrix;
        uni_dg_camera_matrix = camera;

        plane.render_pft2();

        //===============================================================================================================================================================================================================
        // render balls
        //===============================================================================================================================================================================================================
        ball_pp.bind();
        ball_pp.active_shader_program(common_vs_program);
        texture_matrix = glm::mat3(glm::vec3(6.0f, 0.0f,  0.0f),
                                   glm::vec3(0.0f, 3.0f, -1.0f),
                                   glm::vec3(0.0f, 0.0f,  1.0f));
        uni_cv_texture_matrix = texture_matrix;

        for(int i = 0; i != ball_count; ++i)
        {
            ball_pp.active_shader_program(common_vs_program);
            glm::vec3 rot = ball_rotations[i];
            uni_cv_model_matrix = glm::translate(glm::eulerAngleYXZ (rot.y, rot.x, rot.z), ball_offsets[i]);

            ball_pp.active_shader_program(ball_fs_program);
            uni_bf_color_1 = ball_colors[i];
            uni_bf_color_2 = ball_colors[i];
            uni_bf_ball_idx = i;

            glBindTexture(GL_TEXTURE_CUBE_MAP, reflect_textures[i]);
            sphere.render_pft2();
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
