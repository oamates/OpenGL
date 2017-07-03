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

struct sdf_header_t
{
    glm::ivec3 size;
    GLenum internal_format;
    GLuint data_size;    
};

struct sdf_texture_t
{
    glm::ivec3 size;
	GLuint texture_id;
    GLenum texture_unit;
    GLenum internal_format;

    sdf_texture_t() {}

    sdf_texture_t(const glm::ivec3& size, GLenum texture_unit, GLenum internal_format)
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

    // ;
    GLenum format2type(GLenum format)
        { return GL_UNSIGNED_INT; }

    GLuint data_size()
        { return size.x * size.y * size.z * sizeof(GLuint); }

    void clear(GLuint value)
        { glClearTexImage(texture_id, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &value); }

    void store(const char* file_name)
    {
        FILE* f = fopen(file_name, "wb");
        sdf_header_t header = 
        {
            .size = size,
            .internal_format = internal_format,
            .data_size = data_size()
        };

        fwrite(&header, sizeof(sdf_header_t), 1, f);

        GLuint pixel_data_size = data_size();
        GLvoid* pixels = (GLvoid*) malloc(pixel_data_size);

        glActiveTexture(texture_unit);
        glBindTexture(GL_TEXTURE_3D, texture_id);
        glGetTexImage(GL_TEXTURE_3D, 0, internal_format, format2type(internal_format), pixels);

        fwrite(pixels, pixel_data_size, 1, f);
        fclose(f);

        free(pixels);
    }

    static sdf_texture_t load(GLenum texture_unit, const char* file_name)
    {
        FILE* f = fopen(file_name, "rb");

        sdf_header_t header;
        fread(&header, sizeof(sdf_header_t), 1, f);

        GLvoid* pixels = (GLvoid*) malloc(header.data_size);
        fread(pixels, header.data_size, 1, f);
        fclose(f);

        sdf_texture_t sdf_texture;

        sdf_texture.size = header.size;
        sdf_texture.texture_unit = texture_unit;
        sdf_texture.internal_format = header.internal_format;

        glActiveTexture(texture_unit);
        glGenTextures(1, &sdf_texture.texture_id);
        glBindTexture(GL_TEXTURE_3D, sdf_texture.texture_id);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glTexStorage3D(GL_TEXTURE_3D, 1, sdf_texture.internal_format, sdf_texture.size.x, sdf_texture.size.y, sdf_texture.size.z);

        free(pixels);
        return sdf_texture;
    }

    ~sdf_texture_t()
        { glDeleteTextures(1, &texture_id); }
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
    // step 1 :: geometry shader that generates point cloud at a given (signed) distance around a model
    //===================================================================================================================================================================================================================
    glsl_program_t cloud_gen(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/cloud_gen.vs"),
                             glsl_shader_t(GL_TESS_CONTROL_SHADER,    "glsl/cloud_gen.tcs"),
                             glsl_shader_t(GL_TESS_EVALUATION_SHADER, "glsl/cloud_gen.tes"),
                             glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/cloud_gen.gs"),
                             glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/cloud_gen.fs"));

    const char* varyings[] = {"cloud_point"};
    glTransformFeedbackVaryings(cloud_gen.id, 1, varyings, GL_INTERLEAVED_ATTRIBS);
    cloud_gen.link();
    cloud_gen.dump_info();
    cloud_gen.enable();
    cloud_gen["sigma"] = 0.03125f;
    cloud_gen["inv_max_edge"] = 1.0f / 0.0125f;

    //===================================================================================================================================================================================================================
    // step 2 :: load demon model
    //===================================================================================================================================================================================================================
    vao_t model;
    model.init("../../../resources/models/vao/demon.vao");
    GLuint size = 16 * model.ibo.size;

    debug_msg("Model loaded :: index buffer size = %u", model.ibo.size);
    debug_msg("Model primitive type = %u", model.ibo.mode);
    debug_msg("GL_TRIANGLES = %u, GL_TRIANGLE_STRIP = %u", GL_TRIANGLES, GL_TRIANGLE_STRIP);

    //===================================================================================================================================================================================================================
    // step 3 :: create transform feedback buffer
    //===================================================================================================================================================================================================================
    GLuint tfb_id;
    glGenBuffers(1, &tfb_id);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_id);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(glm::vec3), 0, GL_STATIC_READ);

    //===================================================================================================================================================================================================================
    // step 4 :: create query object to collect the number of output vertices and perform feedback transform
    //===================================================================================================================================================================================================================
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
    GLuint cloud_size;
    glGetQueryObjectuiv(query_id, GL_QUERY_RESULT, &cloud_size);

    debug_msg("Point cloud generated. Size = %u", cloud_size);

    //===================================================================================================================================================================================================================
    // step 5 :: done! buffer tbo_id contains the point cloud and cloud_size is the number of points in it
    // now create 3D texture of unsigned integral internal format (GL_R32UI) -- we are going to use imageAtomicMin a lot
    //===================================================================================================================================================================================================================
    const int TEXTURE_SIZE = 256;
    sdf_texture_t sdf_tex(glm::ivec3(TEXTURE_SIZE), GL_TEXTURE0, GL_R32UI);

    //===================================================================================================================================================================================================================
    // step 6 :: create GL_TEXTURE_BUFFER to fetch the output of transform feedback to compute shader
    //===================================================================================================================================================================================================================
    GLuint tbo_id;
    glGenTextures(1, &tbo_id);
    glBindTexture(GL_TEXTURE_BUFFER, tbo_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tfb_id);
    glBindImageTexture(1, tbo_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGB32F);    

    //===================================================================================================================================================================================================================
    // step 7 :: compile unsigned sdf compute shader and 
    // compute shader that combines external and internal unsigned fields into a single signed (float) 
    // 3D distance field texture
    //===================================================================================================================================================================================================================
    glsl_program_t sdf_compute(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/sdf_compute.cs"));
    sdf_compute.enable();
    sdf_compute["cloud_size"] = (int) cloud_size;
    sdf_tex.clear(-1);
    glDispatchCompute(cloud_size >> 8, 1, 1);

    glsl_program_t sdf_combine(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/sdf_combine.cs"));
    sdf_combine.enable();
    glDispatchCompute(TEXTURE_SIZE >> 3, TEXTURE_SIZE >> 3, TEXTURE_SIZE >> 3);

    //===================================================================================================================================================================================================================
    // done! save the generated sdf texture into a file
    //===================================================================================================================================================================================================================
    sdf_tex.store("demon.sdf");

    //===================================================================================================================================================================================================================
    // now show the raymarched result
    //===================================================================================================================================================================================================================
    glsl_program_t skybox_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/skybox.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/skybox.fs"));

    skybox_renderer.enable();
    uniform_t uni_sbox_pv_matrix = skybox_renderer["projection_view_matrix"];
    skybox_renderer["environment_tex"] = 2;

    //===================================================================================================================================================================================================================
    // volume raymarch shader
    //===================================================================================================================================================================================================================
    glsl_program_t crystal_raymarch(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/raymarch_crystal.vs"),
                                    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/raymarch_crystal.fs"));

    crystal_raymarch.enable();

    uniform_t uni_cm_pv_matrix = crystal_raymarch["projection_view_matrix"];
    uniform_t uni_cm_camera_ws = crystal_raymarch["camera_ws"];         
    uniform_t uni_cm_light_ws  = crystal_raymarch["light_ws"];

    crystal_raymarch["scale"] = 3.0f;                                                             
    crystal_raymarch["tb_tex"] = 0;
    crystal_raymarch["value_tex"] = 1;

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
    glActiveTexture(GL_TEXTURE2);

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