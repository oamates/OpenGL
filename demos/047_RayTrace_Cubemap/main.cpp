// oglplus_example_uses_texture{pool_ball_1 ... pool_ball15 }

#include <cmath>

const int OGLPLUS_EXAMPLE_034BB_BALL_COUNT = 15;

namespace oglplus {

// the common vertex shader used in most of the rendering pipelines
struct CommonVertShader : public VertexShader
{
    glsl_shader_t(GL_VERTEX_SHADER, "glsl/common.vs");
};

class DefaultGeomShader : public GeometryShader
{
    glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/default.gs");
};

class CubemapGeomShader : public GeometryShader
{
    glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/cubemap.gs");
};

class VertexProgram : public QuickProgram
{
    const Program& self(void) const { return *this; }
    ProgramUniform<Mat4f> model_matrix;
    ProgramUniform<Mat3f> texture_matrix;
    ProgramUniform<Vec3f> camera_position, light_position;

    VertexProgram(void)
     : QuickProgram(ObjectDesc("Vertex"), true, CommonVertShader())
     , model_matrix(self(), "ModelMatrix")
     , texture_matrix(self(), "TextureMatrix")
     , camera_position(self(), "CameraPosition")
     , light_position(self(), "LightPosition")
    { }
};

class GeometryProgram : public QuickProgram
{
    const Program& self(void) const { return *this; }
    ProgramUniform<Mat4f> projection_matrix, camera_matrix;

    GeometryProgram(const GeometryShader& shader)
     : QuickProgram(ObjectDesc("Geometry"), true, shader)
     , projection_matrix(self(), "ProjectionMatrix")
     , camera_matrix(self(), "CameraMatrix")
    { }
};

class ClothFragmentShader : public FragmentShader
{
    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/cloth.fs");
};

class ClothProgram : public QuickProgram
{
    const Program& self(void) const { return *this; }
    ProgramUniform<Vec3f> color_1, color_2;
    ProgramUniformSampler cloth_tex;
    ProgramUniformSampler light_map;

    ClothProgram(void)
     : QuickProgram(ObjectDesc("Cloth"), true, ClothFragmentShader())
     , color_1(self(), "Color1")
     , color_2(self(), "Color2")
     , cloth_tex(self(), "ClothTex")
     , light_map(self(), "LightMap")
    { }
};

class BallFragmentShader : public FragmentShader
{
    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ball.fs");
};

class BallProgram : public QuickProgram
{
    const Program& self(void) const { return *this; }
    ProgramUniform<Vec3f> color_1, color_2;
    ProgramUniformSampler number_tex, reflect_tex;
    ProgramUniform<GLint> ball_idx;

    BallProgram(void)
     : QuickProgram(ObjectDesc("Ball"), true, BallFragmentShader())
     , color_1(self(), "Color1")
     , color_2(self(), "Color2")
     , number_tex(self(), "NumberTex")
     , reflect_tex(self(), "ReflectTex")
     , ball_idx(self(), "BallIdx")
    { }
};


class LightmapVertShader : public VertexShader
{
    glsl_shader_t(GL_VERTEX_SHADER, "glsl/lightmap.vs");
};

class LightmapFragShader : public FragmentShader
{
    glsl_shader_t(GL_VERTEX_SHADER, "glsl/lightmap.fs");
};

class LightmapProgram : public QuickProgram
{
    glsl_program_t lightmap_program(LightmapVertShader(), LightmapFragShader());
    uniform_t uni_lp_transform_matrix = lightmap_program["TransformMatrix"];     // ProgramUniform<Mat4f> transform_matrix;
    uniform_t uni_lp_light_position   = lightmap_program["LightPosition"];       // ProgramUniform<Vec3f> light_position;
    uniform_t uni_lp_ball_positions   = lightmap_program["BallPositions"];       // ProgramUniform<Vec3f> ball_positions;
};

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

struct BilliardExample : public Example
{
    VertexProgram vert_prog;
    GeometryProgram geom_prog;
    ClothProgram cloth_prog;
    BallProgram ball_prog;

    ProgramPipeline cloth_pp, ball_pp;

    glm::vec3 plane_u, plane_v;
    Shape<shapes::Plane> plane;
    Shape<shapes::Sphere> sphere;

    const GLuint ball_count;

    std::vector<glm::vec3> ball_colors;
    std::vector<glm::vec3> ball_offsets;
    std::vector<glm::vec3> ball_rotations;

    Texture table_light_map;                        // The table light map and the associated framebuffer
    Texture numbers_texture;                        // The array texture storing the ball number decals
    Texture cloth_texture;                          // The texture used to render the cloth
    Array<Texture> reflect_textures;                // The array of cube-maps storing the environment maps of the balls

    BilliardExample(void)
     , geom_prog(DefaultGeomShader())
     , plane_u(16, 0, 0)
     , plane_v(0, 0,-16)
     , plane(vert_prog, shapes::Plane(plane_u, plane_v))
     , sphere(vert_prog, shapes::Sphere(1.0, 36, 24))
     , ball_count(OGLPLUS_EXAMPLE_034BB_BALL_COUNT)
     , ball_colors(
        {
            {0.8f, 0.5f, 0.2f},
            {0.2f, 0.2f, 0.5f},
            {0.6f, 0.2f, 0.4f},
            {0.1f, 0.1f, 0.3f},
            {0.0f, 0.0f, 0.0f},
            {0.3f, 0.1f, 0.2f},
            {0.2f, 0.5f, 0.2f},
            {0.6f, 0.3f, 0.2f}
        }
    ), ball_offsets(
        {
            { 3.0f, 1.0f, 6.5f},
            { 5.0f, 1.0f, 5.0f},
            { 3.0f, 1.0f,-1.0f},
            {-0.1f, 1.0f,-1.1f},
            {-3.0f, 1.0f, 3.0f},
            {-2.8f, 1.0f, 7.0f},
            {-1.1f, 1.0f, 9.0f},
            { 3.0f, 1.0f, 2.0f},
            {-7.0f, 1.0f, 3.0f},
            {-9.5f, 1.0f, 4.5f},
            { 1.0f, 1.0f, 5.2f},
            {-8.0f, 1.0f, 8.0f},
            {-5.0f, 1.0f, 1.0f},
            { 2.0f, 1.0f, 9.0f},
            { 8.0f, 1.0f, 7.5f}
        }
    ), ball_rotations(
        {
            { 0.3f,-0.2f,-0.1f},
            { 0.2f, 0.3f, 0.4f},
            {-0.4f,-0.4f, 0.2f},
            { 0.2f, 0.3f,-0.4f},
            {-0.7f,-0.2f, 0.6f},
            { 0.3f, 0.3f, 0.2f},
            { 0.5f, 0.2f, 0.3f},
            {-0.4f, 0.4f,-0.4f},
            { 0.3f,-0.3f, 0.1f},
            { 0.1f,-0.2f,-0.2f},
            {-0.2f,-0.3f,-0.0f},
            {-0.3f, 0.5f, 0.3f},
            {-0.4f, 0.1f, 0.1f},
            { 0.3f, 0.3f,-0.2f},
            {-0.2f,-0.2f, 0.4f}
        }
    ), reflect_textures(ball_count)
    {
        assert(ball_offsets.size() == ball_count);
        assert(ball_rotations.size() == ball_count);

        gl.RequireAtLeast(LimitQuery::MaxCombinedTextureImageUnits, 5);

        glUseProgram(0);

        cloth_pp.Bind();
        cloth_pp.UseStages(vert_prog).Vertex();
        cloth_pp.UseStages(geom_prog).Geometry();
        cloth_pp.UseStages(cloth_prog).Fragment();

        ball_pp.Bind();
        ball_pp.UseStages(vert_prog).Vertex();
        ball_pp.UseStages(geom_prog).Geometry();
        ball_pp.UseStages(ball_prog).Fragment();

        const Vec3f light_position(0.0f, 20.0f, -2.0f);

        vert_prog.light_position.Set(light_position);

        cloth_prog.color_1.Set(Vec3f(0.1f, 0.3f, 0.1f));
        cloth_prog.color_2.Set(Vec3f(0.3f, 0.4f, 0.3f));

        glActiveTexture(GL_TEXTURE0);
        cloth_prog.light_map.Set(0);
        GLuint light_map_side = 512;
        gl.Bound(Texture::Target::_2D, table_light_map)
            .MinFilter(TextureMinFilter::Linear)
            .MagFilter(TextureMagFilter::Linear)
            .WrapS(TextureWrap::ClampToEdge)
            .WrapT(TextureWrap::ClampToEdge)
            .Image2D(
                0,
                PixelDataInternalFormat::RGB,
                light_map_side,
                light_map_side,
                0,
                PixelDataFormat::RGB,
                PixelDataType::UnsignedByte,
                nullptr
            );

        PrerenderLightmap(light_position, light_map_side);

        glClearColor(0.12f, 0.13f, 0.11f, 0.0f);
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        Texture::Active(1);
        ball_prog.number_tex.Set(1);
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

        InitializeCubemaps(reflect_textures, cubemap_side);
        InitializeCubemaps(temp_cubemaps, cubemap_side);

        // prerender the cubemaps
        PrerenderCubemaps(temp_cubemaps, reflect_textures, cubemap_side);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }

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

    void PrerenderLightmap(const Vec3f& light_position, GLuint tex_side)
    {
        Framebuffer table_light_fbo;
        gl.Bound(Framebuffer::Target::Draw, table_light_fbo).AttachTexture( FramebufferAttachment::Color, table_light_map, 0);

        glViewport(0, 0, tex_side, tex_side);
        glClearColor(1.0 ,1.0, 1.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);

        LightmapProgram prog;

        Shape<shapes::Plane> tmp_plane(prog, shapes::Plane(plane_u, plane_v));

        GLfloat i_u = Length(plane_u);
        i_u = 1.0f / (i_u * i_u);
        GLfloat i_v = Length(plane_v);
        i_v = 1.0f / (i_v * i_v);
        prog.transform_matrix.Set(
            Mat4f(
                Vec4f(plane_u * i_u, 0.0f),
                Vec4f(plane_v * i_v, 0.0f),
                Vec4f(0.0f, 1.0f, 0.0f, 0.0f),
                Vec4f(0.0f, 0.0f, 0.0f, 1.0f)
            )
        );
        prog.light_position.Set(light_position);
        prog.ball_positions.Set(ball_offsets);

        glDisable(GL_DEPTH_TEST);
        tmp_plane.Draw();

        glEnable(GL_DEPTH_TEST);
        glUseProgram(0);
    }

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

    void RenderScene(Array<Texture>& cubemaps, const ProgramPipeline& cloth_prog_pipeline, const ProgramPipeline& ball_prog_pipeline, int skipped = -1)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cloth_prog_pipeline.Bind();                                 // Render the plane

        vert_prog.model_matrix = ModelMatrixf();
        vert_prog.texture_matrix.Set(Mat3f(
            16.0,  0.0,  0.0,
             0.0, 16.0,  0.0,
             0.0,  0.0,  1.0
        ));

        plane.Draw();

        // Render the balls
        ball_prog_pipeline.Bind();

        vert_prog.texture_matrix.Set(Mat3f(
            6.0f, 0.0f, 0.0f,
            0.0f, 3.0f,-1.0f,
            0.0f, 0.0f, 1.0f
        ));

        for(GLuint i = 0; i != ball_count; ++i)
        {
            if(int(i) == skipped) continue;
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

            cubemaps[i].Bind(Texture::Target::CubeMap);
            sphere.Draw();
        }
    }

    void Render(double time)
    {
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
        RenderScene(reflect_textures, cloth_pp, ball_pp);
    }

    void Reshape(GLuint width, GLuint height)
    {
        glViewport(0, 0, width, height);
        geom_prog.projection_matrix.Set(CamMatrixf::PerspectiveX(Degrees(60), width, height, 1, 80));
    }
};
