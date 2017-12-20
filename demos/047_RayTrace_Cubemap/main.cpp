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
#include "fbo.hpp"
#include "brushed_metal.hpp"

#include "image/stb_image.h"

const int BALL_COUNT = 15;

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

    void add_stage(GLbitfield stage_bitmask, const glsl_program_t& program)
        { glUseProgramStages(id, stage_bitmask, program.id); }

    void add_stage(GLbitfield stage_bitmask, const glsl_shader_program_t& shader_program)
        { glUseProgramStages(id, stage_bitmask, shader_program.id); }

    ~glsl_pipeline_t()
        { glDeleteProgramPipelines(1, &id); }
};

/*
template <typename ShapeBuilder> struct Shape
{
    ShapeBuilder make_shape;                                    // helper object building shape vertex attributes
    shapes::DrawingInstructions shape_instr;                    // helper object encapsulating shape drawing instructions
    typename ShapeBuilder::IndexArray shape_indices;            // indices pointing to shape primitive elements

    VertexArray vao;                                            // A vertex array object for the rendered shape
    const GLuint nva;                                           // number of vertex attributes

    // VBOs for the shape's vertex attributes
    Array<Buffer> vbos;

    Shape(const Program& prog, const ShapeBuilder& builder)
     : make_shape(builder)
     , shape_instr(make_shape.Instructions())
     , shape_indices(make_shape.Indices())
     , nva(4)
     , vbos(nva)
    {
        vao.Bind();                                             // bind the VAO for the shape
        typename ShapeBuilder::VertexAttribs vert_attr_info;
        const GLchar* vert_attr_name[] = {"Position", "Normal", "Tangent", "TexCoord"};
        for(GLuint va = 0; va != nva; ++va)
        {
            const GLchar* name = vert_attr_name[va];
            std::vector<GLfloat> data;
            auto getter = vert_attr_info.VertexAttribGetter(data, name);
            if(getter != nullptr)
            {
                // bind the VBO for the vertex attribute
                vbos[va].Bind(Buffer::Target::Array);
                GLuint npv = getter(make_shape, data);
                // upload the data
                Buffer::Data(Buffer::Target::Array, data);
                // setup the vertex attribs array
                VertexArrayAttrib attr(prog, name);
                attr.Setup<GLfloat>(npv);
                attr.Enable();
            }
        }
    }

    void Draw(void)
    {
        vao.Bind();
        glFrontFace(make_shape.FaceWinding());
        shape_instr.Draw(shape_indices);
    }
};

    void Reshape(GLuint width, GLuint height)
    {
        glViewport(0, 0, width, height);
        geom_prog.projection_matrix.Set(CamMatrixf::PerspectiveX(Degrees(60), width, height, 1, 80));
    }
*/

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("VAO Loader", 8, 3, 3, 1920, 1080, true);

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
    uniform_t uni_cf_color_1   = cloth_fs_program["Color1"];                    // ProgramUniform<Vec3f> color_1
    uniform_t uni_cf_color_2   = cloth_fs_program["Color2"];                    // ProgramUniform<Vec3f> color_2
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
    // precompute step
    //===================================================================================================================================================================================================================
    glm::vec3 plane_u = glm::vec3(16.0f, 0.0f,  0.0f);
    glm::vec3 plane_v = glm::vec3( 0.0f, 0.0f,-16.0f);

    Shape<shapes::Plane> plane = Shape<shapes::Plane>(vert_prog, shapes::Plane(plane_u, plane_v));
    Shape<shapes::Sphere> sphere = Shape<shapes::Sphere>(vert_prog, shapes::Sphere(1.0, 36, 24));

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
    cloth_pp.bind();
    cloth_pp.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    cloth_pp.add_stage(GL_GEOMETRY_SHADER_BIT, default_gs_program);
    cloth_pp.add_stage(GL_FRAGMENT_SHADER_BIT, cloth_fs_program);

    glsl_pipeline_t ball_pp;
    ball_pp.bind();
    ball_pp.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    ball_pp.add_stage(GL_GEOMETRY_SHADER_BIT, default_gs_program);
    ball_pp.add_stage(GL_FRAGMENT_SHADER_BIT, ball_fs_program);

    const glm::vec3 light_position = glm::vec3(0.0f, 20.0f, -2.0f);

    uni_cv_light_position = light_position;                             // vert_prog.light_position.Set(light_position);
    uni_cf_color_1 = glm::vec3(0.1f, 0.3f, 0.1f);                       // cloth_prog.color_1.Set(Vec3f(0.1f, 0.3f, 0.1f));
    uni_cf_color_2 = glm::vec3(0.3f, 0.4f, 0.3f);                       // cloth_prog.color_2.Set(Vec3f(0.3f, 0.4f, 0.3f));

    glActiveTexture(GL_TEXTURE0);
    uni_cf_light_map = 0;                                               // cloth_prog.light_map.Set(0);

    GLuint table_light_map;                                             // Texture table_light_map; -- table light map and the associated framebuffer
    glGenTextures(1, &table_light_map);
    GLuint light_map_side = 512;
    glBindTexture(GL_TEXTURE_2D, table_light_map);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, light_map_side, light_map_side);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //Framebuffer table_light_fbo_id;
    GLuint table_light_fbo_id;
    glGenFramebuffers(1, &table_light_fbo_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, table_light_fbo_id);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, table_light_map, 0);

    glViewport(0, 0, light_map_side, light_map_side);
    glClearColor(1.0 ,1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    GLfloat i_u = 1.0f / glm::dot(plane_u, plane_u);
    GLfloat i_v = 1.0f / glm::dot(plane_v, plane_v);

    glm::mat4 transform_matrix = glm::mat4(glm::vec4( plane_u * i_u,   0.0f),
                                           glm::vec4( plane_v * i_v,   0.0f),
                                           glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                                           glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    uni_lm_transform_matrix = transform_matrix;
    uni_lm_light_position = light_position;
    uni_lm_ball_positions = ball_offsets;

    glDisable(GL_DEPTH_TEST);

    /* !!!!!!!!!!!!!!!! */
    //Shape<shapes::Plane> tmp_plane(prog, shapes::Plane(plane_u, plane_v));
    //tmp_plane.Draw();

    glEnable(GL_DEPTH_TEST);
    glUseProgram(0);

    glClearColor(0.12f, 0.13f, 0.11f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glActiveTexture(GL_TEXTURE1);
    uni_bf_number_tex = 1;                                              // ball_prog.number_tex.Set(1)

    const char* tex_names[BALL_COUNT] =
    {
        "pool_ball_1",
        "pool_ball_2",
        "pool_ball_3",
        "pool_ball_4",
        "pool_ball_5",
        "pool_ball_6",
        "pool_ball_7",
        "pool_ball_8",
        "pool_ball_9",
        "pool_ball10",
        "pool_ball11",
        "pool_ball12",
        "pool_ball13",
        "pool_ball14",
        "pool_ball15",
    };

    // Texture numbers_texture;                                            // The array texture storing the ball number decals

    GLuint numbers_texture;
    glGenTextures(1, &numbers_texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, numbers_texture);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glm::vec4 black = glm::vec4(0.0f);
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(black));

    for(GLuint i = 0; i != BALL_COUNT; ++i)
    {
        int width, height, bpp;
        unsigned char* src_data = stbi_load(tex_names[i], &width, &height, &bpp, 0);

        if (src_data == 0)
            exit_msg("Cannot load texture: %s", tex_names[i]);

        if (i == 0)
        {
            int levels = 0;
            int s = (width > height) ? width : height;
            while (s != 0) { s >>= 1; ++levels; }
            glTexStorage3D(GL_TEXTURE_2D_ARRAY, levels, GL_RGBA8, width, height, 16);
        }

        GLenum format = (bpp == 1) ? GL_RED :
                        (bpp == 2) ? GL_RG :
                        (bpp == 3) ? GL_RGB : GL_RGBA;

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, 1, format, GL_UNSIGNED_BYTE, src_data);
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glActiveTexture(GL_TEXTURE2);
    uni_cf_cloth_tex = 2;

    GLuint cloth_texture;                                           // Texture cloth_texture --- used to render the cloth
    glGenTextures(1, &cloth_texture);
    glBindTexture(GL_TEXTURE_2D, cloth_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLubyte* rgb_data = brushed_metal_texture(512, 512, 10240, -16, +16, 8, 32);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_data);
    free(rgb_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE3);
    uni_bf_reflect_tex = 3;

    // ========================================== prerender the cubemaps ================================================
    GLuint cubemap_side = 128;
    GLuint reflect_textures[ball_count];
    GLuint temp_cubemaps[ball_count];

    glGenTextures(ball_count, reflect_textures);
    glGenTextures(ball_count, temp_cubemaps);

    for(GLuint b = 0; b != ball_count; ++b)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, reflect_textures[b]);
        glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB, cubemap_side, cubemap_side);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glm::vec3 black_rgb = glm::vec3(0.0);
        glClearTexImage(reflect_textures[b], 0, GL_RGB, GL_FLOAT, glm::value_ptr(black_rgb));

        glBindTexture(GL_TEXTURE_CUBE_MAP, temp_cubemaps[b]);
        glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB, cubemap_side, cubemap_side);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glClearTexImage(temp_cubemaps[b], 0, GL_RGB, GL_FLOAT, glm::value_ptr(black_rgb));
    }

    // ========================================== prerender the cubemaps ================================================
    //PrerenderCubemaps(temp_cubemaps, reflect_textures, cubemap_side);
    //PrerenderCubemaps(Array<Texture>& src_texs, Array<Texture>& dst_texs, GLuint tex_side)

    glActiveTexture(GL_TEXTURE4);
    GLuint z_buffer;                // Texture z_buffer;

    glGenTextures(1, &z_buffer);
    glBindTexture(GL_TEXTURE_CUBE_MAP, z_buffer);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_DEPTH_COMPONENT, cubemap_side, cubemap_side);

    /* whatta hell is it ?? */
    glActiveTexture(GL_TEXTURE3);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, z_buffer, 0);

    glsl_pipeline_t cmap_cloth_pp, cmap_ball_pp;

    glUseProgram(0);

    cmap_cloth_pp.bind();
    cmap_cloth_pp.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    cmap_cloth_pp.add_stage(GL_GEOMETRY_SHADER_BIT, cubemap_gs_program);
    cmap_cloth_pp.add_stage(GL_FRAGMENT_SHADER_BIT, cloth_fs_program);

    cmap_ball_pp.bind();
    cmap_ball_pp.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    cmap_ball_pp.add_stage(GL_GEOMETRY_SHADER_BIT, cubemap_gs_program);
    cmap_ball_pp.add_stage(GL_FRAGMENT_SHADER_BIT, ball_fs_program);

    uni_cg_projection_matrix = glm::infinitePerspective(constants::half_pi, 1.0f, 0.25f);

    glViewport(0, 0, cubemap_side, cubemap_side);

    for(GLuint b = 0; b != ball_count; ++b)
    {
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, reflect_textures[b], 0);
        uni_cv_camera_position = ball_offsets[b];
        uni_cg_camera_matrix = glm::translate(-ball_offsets[b]);
        RenderScene(src_texs, cmap_cloth_pp, cmap_ball_pp, b);
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    //===================================================================================================================================================================================================================
    // program main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();

        double time = window.frame_ts;

        glm::mat4 camera = window.camera.view_matrix;
        glm::vec3 camera_position = window.camera.position();

        uni_cv_camera_position = camera_position;
        uni_dg_camera_matrix = camera;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cloth_pp.bind();                                 // Render the plane

        uni_cv_model_matrix = glm::mat4(1.0f);

        glm::mat3 texture_matrix = glm::mat3(glm::vec3(16.0f,  0.0f, 0.0f),
                                             glm::vec3( 0.0f, 16.0f, 0.0f),
                                             glm::vec3( 0.0f,  0.0f, 1.0f));
        uni_cv_texture_matrix = texture_matrix;

        plane.Draw();

        ball_pp.bind();

        texture_matrix = glm::mat3(glm::vec3(6.0f, 0.0f,  0.0f),
                                   glm::vec3(0.0f, 3.0f, -1.0f),
                                   glm::vec3(0.0f, 0.0f,  1.0f));
        uni_cv_texture_matrix = texture_matrix;

        for(int i = 0; i != ball_count; ++i)
        {
            glm::vec3 rot = ball_rotations[i];
            int ci = ((i / 4) % 2 == 0) ? i : ((i / 4) + 2) * 4 - i - 1;
            ci %= 8;
            glm::vec3 col = ball_colors[ci];

            uni_cv_model_matrix = glm::translate(glm::eulerAngleYXZ (rot.y, rot.x, rot.z), ball_offsets[i]);
            uni_bf_color_1 = (i > 7) ? glm::vec3(1.0f, 0.9f, 0.8f) : col;
            uni_bf_color_2 = col;
            uni_bf_ball_idx = i;

            glBindTexture(GL_TEXTURE_CUBE_MAP, reflect_textures[i]);
            sphere.Draw();
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