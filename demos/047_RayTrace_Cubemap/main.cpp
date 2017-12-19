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

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "fbo.hpp"

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

    glsl_shader_t cubemap_gs(GL_GEOMETRY_SHADER, "glsl/cubemap.gs");            // CubemapGeomShader
    glsl_shader_program_t cubemap_gs_program(cubemap_gs);                       // GeometryProgram, cmap_geom_prog
    uniform_t uni_cg_projection_matrix = default_gs_program["ProjectionMatrix"];
    uniform_t uni_cg_camera_matrix = default_gs_program["CameraMatrix"];

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

    static_assert(ball_offsets.size() == ball_count);
    static_assert(ball_rotations.size() == ball_count);

    Array<Texture> reflect_textures(ball_count);                        // The array of cube-maps storing the environment maps of the balls
    Texture numbers_texture;                                            // The array texture storing the ball number decals
    Texture cloth_texture;                                              // The texture used to render the cloth

    glUseProgram(0);

    glsl_pipeline_t cloth_pp;
    cloth_pp.bind();
    cloth_pp.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    cloth_pp.add_stage(GL_GEOMETRY_SHADER_BIT, default_gs_program);
    cloth_pp.add_stage(GL_FRAGMENT_SHADER_BIT, cloth_fs_program);

    glsl_pipeline_t ball_pp;
    ball_pp.Bind();
    ball_pp.add_stage(GL_VERTEX_SHADER_BIT,   common_vs_program);
    ball_pp.add_stage(GL_GEOMETRY_SHADER_BIT, default_gs_program);
    ball_pp.add_stage(GL_FRAGMENT_SHADER_BIT, ball_fs_program);

    const glm::vec3 light_position = glm::vec3(0.0f, 20.0f, -2.0f);

    uniform_t uni_cv_light_position = light_position;                   // vert_prog.light_position.Set(light_position);
    uni_cf_color1 = glm::vec3f(0.1f, 0.3f, 0.1f);                       // cloth_prog.color_1.Set(Vec3f(0.1f, 0.3f, 0.1f));
    uni_cf_color2 = glm::vec3f(0.3f, 0.4f, 0.3f);                       // cloth_prog.color_2.Set(Vec3f(0.3f, 0.4f, 0.3f));

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

    /* TEXTURE LOADING */

        {
            const char* tex_names[OGLPLUS_EXAMPLE_034BB_BALL_COUNT] = {
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

            auto bound_tex = gl.Bound(Texture::Target::_2DArray, numbers_texture)
                .BorderColor(Vec4f(0,0,0,0))
                .MinFilter(TextureMinFilter::LinearMipmapLinear)
                .MagFilter(TextureMagFilter::Linear)
                .Wrap(TextureWrap::ClampToBorder);

            for(GLuint i = 0; i != ball_count; ++i)
            {
                auto image = images::LoadTexture(tex_names[i]);
                if(i == 0)
                {
                    bound_tex.Image3D(
                        0,
                        PixelDataInternalFormat::RGBA,
                        image.Width(),
                        image.Height(),
                        16,
                        0,
                        image.Format(),
                        image.Type(),
                        nullptr
                    );
                }
                bound_tex.SubImage3D(
                    0,
                    0, 0, i,
                    image.Width(),
                    image.Height(),
                    1,
                    image.Format(),
                    image.Type(),
                    image.RawData()
                );
            }
            bound_tex.GenerateMipmap();
        }

        glActiveTexture(GL_TEXTURE2);
        cloth_prog.cloth_tex.Set(2);
        gl.Bound(Texture::Target::_2D, cloth_texture)
            .MinFilter(TextureMinFilter::LinearMipmapLinear)
            .MagFilter(TextureMagFilter::Linear)
            .WrapS(TextureWrap::Repeat)
            .WrapT(TextureWrap::Repeat)
            .Image2D(
                images::BrushedMetalUByte(512, 512, 10240, -16, +16, 8, 32)
            ).GenerateMipmap();

        glActiveTexture(GL_TEXTURE3);
        ball_prog.reflect_tex.Set(3);

        GLuint cubemap_side = 128;

        Array<Texture> temp_cubemaps(ball_count);

    // ========================================== prerender the cubemaps ================================================

        InitializeCubemaps(reflect_textures, cubemap_side);
        InitializeCubemaps(temp_cubemaps, cubemap_side);

    void InitializeCubemaps(Array<Texture>& cubemaps, GLuint cubemap_side)
    {
        std::vector<GLubyte> black(cubemap_side*cubemap_side*3, 0x00);

        for(GLuint b = 0; b != ball_count; ++b)
        {
            gl.Bound(Texture::Target::CubeMap, cubemaps[b]).Filter(TextureFilter::Linear).Wrap(TextureWrap::ClampToEdge);
            for(int f = 0; f != 6; ++f)
            {
                Texture::ImageCM(f, 0, PixelDataInternalFormat::RGB, cubemap_side, cubemap_side, 0, PixelDataFormat::RGB, PixelDataType::UnsignedByte, black.data());
            }
        }
    }


    // ========================================== prerender the cubemaps ================================================
    PrerenderCubemaps(temp_cubemaps, reflect_textures, cubemap_side);
    void PrerenderCubemaps(Array<Texture>& src_texs, Array<Texture>& dst_texs, GLuint tex_side)
    {
        glActiveTexture(GL_TEXTURE4);
        Texture z_buffer;
        gl.Bound(Texture::Target::CubeMap, z_buffer).Filter(TextureFilter::Nearest).Wrap(TextureWrap::ClampToEdge);

        for(int i = 0; i != 6; ++i)
        {
            Texture::Image2D( Texture::CubeMapFace(i), 0, PixelDataInternalFormat::DepthComponent, tex_side, tex_side, 0, PixelDataFormat::DepthComponent, PixelDataType::Float, nullptr);
        }

        glActiveTexture(GL_TEXTURE3);

        Framebuffer fbo;
        gl.Bound(Framebuffer::Target::Draw, fbo).AttachTexture(FramebufferAttachment::Depth, z_buffer, 0);

        CubemapGeomShader cmap_geom_shader;
        GeometryProgram cmap_geom_prog(cmap_geom_shader);

        ProgramPipeline cmap_cloth_pp, cmap_ball_pp;

        glUseProgram(0);

        cmap_cloth_pp.Bind();
        cmap_cloth_pp.UseStages(vert_prog).Vertex();
        cmap_cloth_pp.UseStages(cmap_geom_prog).Geometry();
        cmap_cloth_pp.UseStages(cloth_prog).Fragment();

        cmap_ball_pp.Bind();
        cmap_ball_pp.UseStages(vert_prog).Vertex();
        cmap_ball_pp.UseStages(cmap_geom_prog).Geometry();
        cmap_ball_pp.UseStages(ball_prog).Fragment();

        cmap_geom_prog.projection_matrix.Set(CamMatrixf::PerspectiveX(Degrees(90), 1.0, 1, 80));
        gl.Viewport(tex_side, tex_side);

        for(GLuint b = 0; b != ball_count; ++b)
        {
            Framebuffer::AttachTexture(Framebuffer::Target::Draw, FramebufferAttachment::Color, dst_texs[b], 0);

            vert_prog.camera_position.Set(ball_offsets[b]);
            cmap_geom_prog.camera_matrix.Set(ModelMatrixf::Translation(-ball_offsets[b]));

            RenderScene(src_texs, cmap_cloth_pp, cmap_ball_pp, b);
        }
    }


    // ========================================== prerender the cubemaps ================================================

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    //===================================================================================================================================================================================================================
    // program main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();

        double time = window.frame_ts;

        // setup the camera
        glm::vec3 camera_target(0.0f, 2.2f, 5.0f);
        auto camera = CamMatrixf::Orbiting(
            camera_target,
            GLfloat(16.0 - SineWave(time / 15.0)*12.0),
            FullCircles(time / 24.0),
            Degrees(50 + SineWave(time / 20.0) * 35)
        );

        glm::vec3 camera_position = camera.Position();
        vert_prog.camera_position.Set(camera_position);
        geom_prog.camera_matrix.Set(camera);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cloth_pp.Bind();                                 // Render the plane

        vert_prog.model_matrix = ModelMatrixf();
        vert_prog.texture_matrix.Set(Mat3f(
            16.0,  0.0,  0.0,
             0.0, 16.0,  0.0,
             0.0,  0.0,  1.0
        ));

        plane.Draw();

        // Render the balls
        ball_pp.Bind();

        vert_prog.texture_matrix.Set(Mat3f(
            6.0f, 0.0f, 0.0f,
            0.0f, 3.0f,-1.0f,
            0.0f, 0.0f, 1.0f
        ));

        for(GLuint i = 0; i != ball_count; ++i)
        {
            Vec3f rot = ball_rotations[i];
            int ci = ((i / 4) % 2 == 0)?i : ((i/4)+2)*4-i-1;
            ci %= 8;
            Vec3f col = ball_colors[ci];
            vert_prog.model_matrix = ModelMatrixf(
                ModelMatrixf::Translation(ball_offsets[i]) *
                ModelMatrixf::RotationZ(FullCircles(rot.z())) *
                ModelMatrixf::RotationY(FullCircles(rot.y())) *
                ModelMatrixf::RotationX(FullCircles(rot.x()))
            );

            ball_prog.color_1 = (i > 7) ? Vec3f(1.0f, 0.9f, 0.8f) : col;
            ball_prog.color_2 = col;
            ball_prog.ball_idx = i;

            reflect_textures[i].Bind(Texture::Target::CubeMap);
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
