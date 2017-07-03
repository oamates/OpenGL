//========================================================================================================================================================================================================================
// DEMO 058 : SDF Texture 3D generator
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
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "glsl_noise.hpp"
#include "plato.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "polyhedron.hpp"
#include "image.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.01f);
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

struct skybox_t
{
    GLuint vao_id, vbo_id, ibo_id;

    skybox_t() 
    {
        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);

        const GLfloat cube_vertices[] =
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

        const GLubyte cube_indices[] =
        {
            0, 1, 2, 3, 6, 7, 4, 5,
            2, 6, 0, 4, 1, 5, 3, 7
        };

        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glGenBuffers(1, &ibo_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);
    }

    void render()
    {
        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_BYTE, 0);
        glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_BYTE, (const GLvoid *)(8 * sizeof(GLubyte)));        
    }

    ~skybox_t()
    {
        glDeleteBuffers(1, &ibo_id);
        glDeleteBuffers(1, &vbo_id);
        glDeleteVertexArrays(1, &vao_id);
    }
};

struct tex3d_header_t
{
    glm::ivec3 size;
    GLenum internal_format;
    GLuint data_size;    
};

struct texture3d_t
{
    glm::ivec3 size;
	GLuint texture_id;
    GLenum texture_unit;
    GLenum internal_format;

    texture3d_t() {}

    texture3d_t(const glm::ivec3& size, GLenum texture_unit, GLenum internal_format)
        : size(size), texture_unit(texture_unit), internal_format(internal_format)
    {
    	glActiveTexture(texture_unit);
	    glGenTextures(1, &texture_id);
	    glBindTexture(GL_TEXTURE_3D, texture_id);
	    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glTexStorage3D(GL_TEXTURE_3D, 1, internal_format, size.x, size.y, size.z);
    }

    void bind_as_image(GLuint image_unit, GLenum access = GL_READ_WRITE)
        { glBindImageTexture(image_unit, texture_id, 0, GL_TRUE, 0, access, internal_format); }

    GLenum format2type(GLenum format)
        { return GL_UNSIGNED_INT; }

    GLuint data_size()
        { return size.x * size.y * size.z * sizeof(GLuint); }

    void clear(GLuint value)
        { glClearTexImage(texture_id, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &value); }

    void store(const char* file_name)
    {
        FILE* f = fopen(file_name, "wb");
        tex3d_header_t header = 
        {
            .size = size,
            .internal_format = internal_format,
            .data_size = data_size()
        };

        fwrite(&header, sizeof(tex3d_header_t), 1, f);

        GLuint pixel_data_size = data_size();
        GLvoid* pixels = (GLvoid*) malloc(pixel_data_size);

        glActiveTexture(texture_unit);
        glBindTexture(GL_TEXTURE_3D, texture_id);
        glGetTexImage(GL_TEXTURE_3D, 0, internal_format, format2type(internal_format), pixels);

        fwrite(pixels, pixel_data_size, 1, f);
        fclose(f);

        free(pixels);
    }

    static texture3d_t load(GLenum texture_unit, const char* file_name)
    {
        FILE* f = fopen(file_name, "rb");

        tex3d_header_t header;
        fread(&header, sizeof(tex3d_header_t), 1, f);

        GLvoid* pixels = (GLvoid*) malloc(header.data_size);
        fread(pixels, header.data_size, 1, f);
        fclose(f);

        texture3d_t texture3d;

        texture3d.size = header.size;
        texture3d.texture_unit = texture_unit;
        texture3d.internal_format = header.internal_format;

        glActiveTexture(texture_unit);
        glGenTextures(1, &texture3d.texture_id);
        glBindTexture(GL_TEXTURE_3D, texture3d.texture_id);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glTexStorage3D(GL_TEXTURE_3D, 1, texture3d.internal_format, texture3d.size.x, texture3d.size.y, texture3d.size.z);

        free(pixels);
        return texture3d;
    }

    ~texture3d_t()
        { glDeleteTextures(1, &texture_id); }
};

struct sdf_compute_t
{
    int size;

    glsl_program_t compute_shader;
    uniform_t uni_udf_size;

    glsl_program_t march_shader;
    glsl_program_t combine_shader;

    sdf_compute_t(int size)
    {
        //===============================================================================================================================================================================================================
        // 1. compute shader that generates unsigned distance field from initial cloud
        // 2. compute shader that marches and extends unsigned distance field to the whole texture
        // 3. compute shader that combines external and internal unsigned distance fields into a single (signed) distance field function
        //===============================================================================================================================================================================================================
        compute_shader = glsl_program_t(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/sdf_compute.cs"));
        uni_udf_size = compute_shader["cloud_size"];

        march_shader = glsl_program_t(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/sdf_march.cs"));
        combine_shader = glsl_program_t(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/sdf_combine.cs"));
    }

    texture3d_t compute(GLuint tbo_id, int cloud_size)
    {
        debug_msg("sdf_compute_t::compute begin");

        texture3d_t udf_texture(glm::ivec3(size), GL_TEXTURE0, GL_R32UI);
        udf_texture.clear(0xFFFFFFFF);
        udf_texture.bind_as_image(0, GL_WRITE_ONLY);
        uni_udf_size = cloud_size;
        glDispatchCompute(cloud_size >> 8, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        debug_msg("sdf_compute_t::compute end");

        return udf_texture;
    }

    void march(texture3d_t& udf_texture, GLuint iterations)
    {
        debug_msg("sdf_compute_t::march begin");

        udf_texture.bind_as_image(0, GL_READ_WRITE);
        for(int i = 0; i < iterations; ++i)
        {
            glDispatchCompute(size >> 3, size >> 3, size >> 3);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }

        debug_msg("sdf_compute_t::march end");

    }

    texture3d_t combine(texture3d_t& external, texture3d_t& internal, GLenum texture_unit)
    {
        debug_msg("sdf_compute_t::combine begin");

        combine_shader.enable();

        external.bind_as_image(0, GL_READ_ONLY);
        internal.bind_as_image(1, GL_READ_ONLY);

        texture3d_t sdf_texture(glm::ivec3(size), texture_unit, GL_R32F);
        sdf_texture.bind_as_image(2, GL_WRITE_ONLY);

        glDispatchCompute(size >> 3, size >> 3, size >> 3);

        debug_msg("sdf_compute_t::combine end");

        return sdf_texture;
    }

};

struct cloud_gen_t
{
    glsl_program_t program;
    uniform_t uni_sigma;
    uniform_t uni_inv_max_edge;

    cloud_gen_t()
    {
        //===============================================================================================================================================================================================================
        // tesselator + geometry shader that generates point cloud at a given (signed) distance around a model
        //===============================================================================================================================================================================================================
        program = glsl_program_t(glsl_shader_t(GL_VERTEX_SHADER,          "glsl/cloud_gen.vs"),
                                 glsl_shader_t(GL_TESS_CONTROL_SHADER,    "glsl/cloud_gen.tcs"),
                                 glsl_shader_t(GL_TESS_EVALUATION_SHADER, "glsl/cloud_gen.tes"),
                                 glsl_shader_t(GL_GEOMETRY_SHADER,        "glsl/cloud_gen.gs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER,        "glsl/cloud_gen.fs"));

        const char* varyings[] = {"cloud_point"};
        glTransformFeedbackVaryings(program.id, 1, varyings, GL_INTERLEAVED_ATTRIBS);
        program.link();
        program.dump_info();
        program.enable();
        uni_sigma = program["sigma"];
        uni_inv_max_edge = program["inv_max_edge"];
    }

    void enable()
        { program.enable(); }

    GLuint process(vao_t& model, GLuint max_size, GLuint& actual_size)
    {
        GLuint tfb_id;
        glGenBuffers(1, &tfb_id);
        glBindBuffer(GL_ARRAY_BUFFER, tfb_id);
        glBufferData(GL_ARRAY_BUFFER, max_size * sizeof(glm::vec3), 0, GL_STATIC_READ);

        GLuint query_id;
        glGenQueries(1, &query_id);

        glEnable(GL_RASTERIZER_DISCARD);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfb_id);
        glPatchParameteri(GL_PATCH_VERTICES, 3);

        glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query_id);
        glBeginTransformFeedback(GL_POINTS);
        model.render(GL_PATCHES);
        glEndTransformFeedback();
        glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

        glDisable(GL_RASTERIZER_DISCARD);
        glFinish();
        glGetQueryObjectuiv(query_id, GL_QUERY_RESULT, &actual_size);

        debug_msg("Point cloud generated. Size = %u", actual_size);
        return tfb_id;
    }

};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    int res_x = 1920;
    int res_y = 1080;

    //===================================================================================================================================================================================================================
    // step 0 :: initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("SDF Texture 3D generator", 4, 4, 3, res_x, res_y, true);


    //===================================================================================================================================================================================================================
    // step 2 :: load demon model
    //===================================================================================================================================================================================================================
    vao_t model;
    model.init("../../../resources/models/vao/demon.vao");

    debug_msg("Model loaded :: index buffer size = %u", model.ibo.size);
    debug_msg("Model primitive type = %u", model.ibo.mode);
    debug_msg("GL_TRIANGLES = %u, GL_TRIANGLE_STRIP = %u", GL_TRIANGLES, GL_TRIANGLE_STRIP);

    //===================================================================================================================================================================================================================
    // step 3 :: create cloud generator and get two transform feedback cloud point buffers
    //           create GL_TEXTURE_BUFFER for each generated tfb to fetch data to compute shader
    //===================================================================================================================================================================================================================

    cloud_gen_t generator;
    generator.enable();
    generator.uni_inv_max_edge = 1.0f / 0.0125f;

    GLuint max_size = 16 * model.ibo.size;
    GLuint external_cloud_size, internal_cloud_size;

    generator.uni_sigma =  0.03125f;
    GLuint external_tfb_id = generator.process(model, max_size, external_cloud_size);

    GLuint external_tbo_id;
    glGenTextures(1, &external_tbo_id);
    glBindTexture(GL_TEXTURE_BUFFER, external_tbo_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, external_tfb_id);

    generator.uni_sigma = -0.03125f;
    GLuint internal_tfb_id = generator.process(model, max_size, internal_cloud_size);

    GLuint internal_tbo_id;
    glGenTextures(1, &internal_tbo_id);
    glBindTexture(GL_TEXTURE_BUFFER, internal_tbo_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, internal_tfb_id);

    //===================================================================================================================================================================================================================
    // step 4 :: compile three compute shaders for cloud processing, process the data
    //           and store the resulting signed distance texture in a file
    //===================================================================================================================================================================================================================
    sdf_compute_t sdf_tex3d_generator(256);

    sdf_tex3d_generator.compute_shader.enable();

    texture3d_t external_udf = sdf_tex3d_generator.compute(external_tbo_id, external_cloud_size);
    sdf_tex3d_generator.march(external_udf, 64);

    texture3d_t internal_udf = sdf_tex3d_generator.compute(internal_tbo_id, internal_cloud_size);
    sdf_tex3d_generator.march(internal_udf, 64);

    texture3d_t sdf_texture = sdf_tex3d_generator.combine(external_udf, internal_udf, GL_TEXTURE2);
    sdf_texture.store("demon.t3d");

    //===================================================================================================================================================================================================================
    // now show the raymarched result
    //===================================================================================================================================================================================================================
    glsl_program_t skybox_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/skybox.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/skybox.fs"));

    skybox_renderer.enable();
    uniform_t uni_sbox_pv_matrix = skybox_renderer["projection_view_matrix"];
    skybox_renderer["environment_tex"] = 3;

    //===================================================================================================================================================================================================================
    // volume raymarch shader
    //===================================================================================================================================================================================================================
    glsl_program_t crystal_raymarch(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/raymarch_crystal.vs"),
                                    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/raymarch_crystal.fs"));

    crystal_raymarch.enable();

    uniform_t uni_cm_pv_matrix = crystal_raymarch["projection_view_matrix"];
    uniform_t uni_cm_camera_ws = crystal_raymarch["camera_ws"];         
    uniform_t uni_cm_light_ws  = crystal_raymarch["light_ws"];

    crystal_raymarch["tb_tex"] = 0;
    crystal_raymarch["value_tex"] = 1;
    crystal_raymarch["distance_tex"] = 2;

    //===================================================================================================================================================================================================================
    // create dodecahecron buffer
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint tb_tex_id = image::png::texture2d("../../../resources/tex2d/marble.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    
    glActiveTexture(GL_TEXTURE1);
    GLuint noise_tex = glsl_noise::randomRGBA_shift_tex256x256(glm::ivec2(37, 17));

    //===================================================================================================================================================================================================================
    // create skybox buffer and load skybox cubemap texture
    //===================================================================================================================================================================================================================
    skybox_t skybox;
    glActiveTexture(GL_TEXTURE3);

    const char* sunset_files[6] = {"../../../resources/cubemap/sunset/positive_x.png",
                                   "../../../resources/cubemap/sunset/negative_x.png",
                                   "../../../resources/cubemap/sunset/positive_y.png",
                                   "../../../resources/cubemap/sunset/negative_y.png",
                                   "../../../resources/cubemap/sunset/positive_z.png",
                                   "../../../resources/cubemap/sunset/negative_z.png"};
    GLuint env_tex_id = image::png::cubemap(sunset_files);

    //===================================================================================================================================================================================================================
    // light variables
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);    

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = window.frame_ts;
        glm::vec3 light_ws = glm::vec3(10.0f, 2.0f * glm::cos(time), 3.0f * glm::sin(time));

        /* automatic camera */
        float radius = 9.0f + 2.55f * glm::cos(0.25f * time);
        float z = 1.45f * glm::sin(0.25f * time);

        glm::vec3 camera_ws = glm::vec3(radius * glm::cos(0.3f * time), z, radius * glm::sin(0.3f * time));
        glm::vec3 up = glm::normalize(glm::vec3(glm::cos(0.41 * time), -6.0f, glm::sin(0.41 * time)));
        glm::mat4 view_matrix = glm::lookAt(camera_ws, glm::vec3(0.0f), up);
        glm::mat4 projection_view_matrix = window.camera.projection_matrix * view_matrix;

        /* hand-driven camera */
        // glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        // glm::vec3 camera_ws = window.camera.position();

        //===============================================================================================================================================================================================================
        // render skybox
        //===============================================================================================================================================================================================================
        glDisable(GL_BLEND);
        glCullFace(GL_FRONT);
        skybox_renderer.enable();
        uni_sbox_pv_matrix = projection_view_matrix;
        skybox.render();

        //===============================================================================================================================================================================================================
        // raymarch through sdf
        //===============================================================================================================================================================================================================
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glCullFace(GL_BACK);
        crystal_raymarch.enable();
        uni_cm_pv_matrix = projection_view_matrix;
        uni_cm_camera_ws = camera_ws;
        uni_cm_light_ws = light_ws;
        model.render();

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}