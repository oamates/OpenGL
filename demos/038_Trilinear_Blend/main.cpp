//========================================================================================================================================================================================================================
// DEMO 038: Trilinear blend textures
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "vao.hpp"
#include "vertex.hpp"
#include "momenta.hpp"
#include "glsl_noise.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    bool show_normals = false;
    int active_model = 0;
    int model_count;

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
    
        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
            show_normals = !show_normals;

        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE))
            active_model = (active_model + 1) % model_count;

        if ((key == GLFW_KEY_KP_SUBTRACT) && (action == GLFW_RELEASE))
            active_model = (active_model + model_count - 1) % model_count;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};
         
struct model_data_t
{
    const char* model_path;
    const char* texture_path;
    glm::mat4 model_matrix;
    glm::vec3 Ka;
    glm::vec3 Kd;
    glm::vec3 Ks;
    float Ns;
    float bf;
    GLuint texture_id;

    model_data_t(const char* model_path, const char* texture_path, const glm::mat4& model_matrix, const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, float Ns, float bf) 
        : model_path(model_path), texture_path(texture_path), model_matrix(model_matrix), Ka(Ka), Kd(Kd), Ks(Ks), Ns(Ns), bf(bf), texture_id(0) {}
};

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Trilinear Blend", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Load trilinear blend shader which produces nice 3-dimensional material texture from arbitrary 2 dimensional input
    //===================================================================================================================================================================================================================
    glsl_program_t trilinear_blend(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/tb.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/tb.fs"));

    trilinear_blend.enable();
    uniform_t uniform_model_matrix           = trilinear_blend["model_matrix"];
    uniform_t uniform_projection_view_matrix = trilinear_blend["projection_view_matrix"];
    uniform_t uniform_camera_ws              = trilinear_blend["camera_ws"];
    uniform_t uniform_light_ws               = trilinear_blend["light_ws"];
    uniform_t uniform_Ka                     = trilinear_blend["Ka"];
    uniform_t uniform_Kd                     = trilinear_blend["Kd"];
    uniform_t uniform_Ks                     = trilinear_blend["Ks"];
    uniform_t uniform_Ns                     = trilinear_blend["Ns"];
    uniform_t uniform_bf                     = trilinear_blend["bf"];
    trilinear_blend["tb_tex2d"] = 0;

    //===================================================================================================================================================================================================================
    // Shader to verify normal directions
    //===================================================================================================================================================================================================================
    glsl_program_t normal_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/normal_renderer.vs"),
                                   glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/normal_renderer.gs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/normal_renderer.fs"));
    normal_renderer.enable();
    uniform_t uni_nr_projection_view_matrix = normal_renderer["projection_view_matrix"];
    uniform_t uni_nr_model_matrix = normal_renderer["model_matrix"];

    //===================================================================================================================================================================================================================
    // Shader to render ground plane
    // Terrain will be completely procedural, but we need (attribute-less) vertex array object anyway
    //===================================================================================================================================================================================================================
    glsl_program_t terrain_shader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/terrain.vs"),
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/terrain.fs"));

    terrain_shader.enable();
    uniform_t uni_ts_projection_view_matrix = terrain_shader["projection_view_matrix"];
    uniform_t uni_ts_camera_ws = terrain_shader["camera_ws"];
    uniform_t uni_ts_light_ws = terrain_shader["light_ws"];

    const int N = 256;
    GLuint I = 2 * N * (4 * N + 3);
    GLuint ibo_size = I * sizeof(unsigned int);

    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    GLuint ibo_id;
    glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibo_size, 0, GL_STATIC_DRAW);

    unsigned int* indices = (unsigned int*) glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

    unsigned int idx = 0;
    unsigned int i = 0;
    for (int q = -N; q < N; ++q)
    {
        for (int p = -N; p <= N; ++p)
        {
            indices[idx++] = i;             
            indices[idx++] = i + 2 * N + 1;         
            ++i;
        }
        indices[idx++] = -1;
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    terrain_shader["value_texture"] = 1;
    terrain_shader["grass"] = 2;
    terrain_shader["dirt"] = 3;

    glActiveTexture(GL_TEXTURE1);
    GLuint noise_tex = glsl_noise::randomRGBA_shift_tex256x256(glm::ivec2(37, 17));
    glActiveTexture(GL_TEXTURE2);
    GLuint grass_tex = image::png::texture2d("../../../resources/tex2d/grass.png");
    glActiveTexture(GL_TEXTURE3);
    GLuint dirt_tex  = image::png::texture2d("../../../resources/tex2d/dirt.png");
    


    //===================================================================================================================================================================================================================
    // model descriptions :: path to model + path to texture + model matrix + ambient/diffuse/specular light components intencity + specular exponent + bump factor
    //===================================================================================================================================================================================================================
    const float sX = 25.0;
    const float sY = 32.0;

    model_data_t model_data[] = {
        model_data_t("../../../resources/models/vao/ashtray.vao",
                     "../../../resources/tex2d/malahite.png",
                     glm::scale(glm::translate(glm::vec3(0.0, 2.72f, 0.0)), glm::vec3(0.64f)),
                     glm::vec3(0.17f),                                                      // Ka
                     glm::vec3(1.0f),                                                       // Kd
                     glm::vec3(0.33f),                                                      // Ks
                     20.0f,                                                                 // Ns
                     0.1875f                                                                // bf
                    ),

        model_data_t("../../../resources/models/vao/azog.vao",
                     "../../../resources/tex2d/clay.png",
                     glm::scale(glm::rotate(glm::rotate(glm::translate(glm::vec3(-25.0f, 12.0f, -48.0f)), -1.507f, glm::vec3(1.0f, 0.0f, 0.0f)), 0.000f, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(1.35f)),
                     glm::vec3(0.17f),                                                      // Ka
                     glm::vec3(1.0f),                                                       // Kd
                     glm::vec3(0.33f),                                                      // Ks
                     20.0f,                                                                 // Ns
                     0.1875f                                                                // bf
                    ),

        model_data_t("../../../resources/models/vao/bust.vao",
                     "../../../resources/tex2d/crystalline.png",
                     glm::rotate(glm::rotate(glm::scale(glm::translate(glm::vec3(-70.0, 15.139f, -16.0)), glm::vec3(1.45f)), 1.507f, glm::vec3(0.0f, 1.0f, 0.0f)), -0.057f, glm::vec3(0.0f, 0.0f, 1.0f)),
                     glm::vec3(0.17f),                                                      // Ka
                     glm::vec3(1.0f),                                                       // Kd
                     glm::vec3(0.33f),                                                      // Ks
                     20.0f,                                                                 // Ns
                     0.1875f                                                                // bf
                    ),

        model_data_t("../../../resources/models/vao/chubby_girl.vao",
                     "../../../resources/tex2d/sapphire.png",
                     glm::scale(glm::rotate(glm::translate(glm::vec3(-55.4f, 21.8f, 32.9f)), 1.81f, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(1.72f)),
                     glm::vec3(0.17f),                                                      // Ka
                     glm::vec3(1.0f),                                                       // Kd
                     glm::vec3(0.33f),                                                      // Ks
                     20.0f,                                                                 // Ns
                     0.1875f                                                                // bf
                    ),

        model_data_t("../../../resources/models/vao/demon.vao",
                     "../../../resources/tex2d/marble.png",
                     glm::scale(glm::rotate(glm::rotate(glm::scale(glm::translate(glm::vec3(-33.15f, 19.76f, 57.0f)), glm::vec3(1.0f)), 2.19f, glm::vec3(0.0f, 1.0f, 0.0f)), 0.27f, glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(1.31f)),
                     glm::vec3(0.43f, 0.31f, 0.17f),                                        // Ka
                     glm::vec3(1.31f, 1.07f, 0.91f),                                        // Kd
                     glm::vec3(4.73f, 3.76f, 2.41f),                                        // Ks
                     80.0f,                                                                 // Ns
                     0.01875f                                                               // bf
                    ),

        model_data_t("../../../resources/models/vao/dragon.vao",
                     "../../../resources/tex2d/green_mineral.png",
                     glm::rotate(glm::translate(glm::vec3(-3.0, 8.8f, 74.0)), 0.61f, glm::vec3(1.0f, 0.0f, 0.0f)), 
                     glm::vec3(0.17f),                                                      // Ka
                     glm::vec3(1.0f),                                                       // Kd
                     glm::vec3(0.33f),                                                      // Ks
                     20.0f,                                                                 // Ns
                     0.1875f                                                                // bf
                    ),

        model_data_t("../../../resources/models/vao/female_01.vao",
                     "../../../resources/tex2d/ice.png",
                     glm::scale(glm::translate(glm::vec3(14.76f, 12.5f, 0.0f)), glm::vec3(0.97f)), 
                     glm::vec3(0.17f),                                                      // Ka
                     glm::vec3(1.0f),                                                       // Kd
                     glm::vec3(0.33f),                                                      // Ks
                     20.0f,                                                                 // Ns
                     0.1875f                                                                // bf
                    ),

        model_data_t("../../../resources/models/vao/female_02.vao",
                     "../../../resources/tex2d/raw_plumbum.png",
                     glm::scale(glm::translate(glm::vec3(-7.4f, 12.5f, 11.445f)), glm::vec3(0.94f)), 
                     glm::vec3(0.17f),                                                      // Ka
                     glm::vec3(1.0f),                                                       // Kd
                     glm::vec3(0.33f),                                                      // Ks
                     20.0f,                                                                 // Ns
                     0.1875f                                                                // bf
                    ),

        model_data_t("../../../resources/models/vao/female_03.vao",
                     "../../../resources/tex2d/liquid_metal.png",
                     glm::scale(glm::translate(glm::vec3(-8.1f, 11.9f, -11.445f)), glm::vec3(0.87f)),
                     glm::vec3(0.17f),                                                      // Ka
                     glm::vec3(1.0f),                                                       // Kd
                     glm::vec3(0.33f),                                                      // Ks
                     20.0f,                                                                 // Ns
                     0.1875f                                                                // bf
                    ),

        model_data_t("../../../resources/models/vao/skull.vao",
                     "../../../resources/tex2d/lava2.png",                  
                     glm::rotate(glm::rotate(glm::translate(glm::vec3(25.0f, 7.25f, -48.0f)), -0.45f, glm::vec3(0.0f, 1.0f, 0.0f)), 0.24f, glm::vec3(1.0f, 0.0f, 0.0f)),
                     glm::vec3(0.17f, 0.17f, 0.17f),                                        // Ka
                     glm::vec3(0.95f, 0.95f, 0.95f),                                        // Kd
                     glm::vec3(0.0f),                                                       // Ks
                     20.0f,                                                                 // Ns
                     -0.4875f                                                               // bf
                    ),


        model_data_t("../../../resources/models/vao/king_kong.vao",
                     "../../../resources/tex2d/azure_dirt.png",
                     glm::scale(glm::rotate(glm::translate(glm::vec3(52.97, 19.0f, 19.57f)), -1.83f, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(1.241f)),
                     glm::vec3(0.17f),                                                      // Ka
                     glm::vec3(1.0f),                                                       // Kd
                     glm::vec3(0.33f),                                                      // Ks
                     20.0f,                                                                 // Ns
                     0.1875f                                                                // bf
                    ),

        model_data_t("../../../resources/models/vao/predator.vao",
                     "../../../resources/tex2d/marble.png",
                     glm::rotate(glm::translate(glm::vec3(29.0f, 12.0f, 66.0)), -2.41f, glm::vec3(0.0f, 1.0f, 0.0f)), 
                     glm::vec3(0.17f),                                                      // Ka
                     glm::vec3(1.0f),                                                       // Kd
                     glm::vec3(0.33f),                                                      // Ks
                     20.0f,                                                                 // Ns
                     0.1875f                                                                // bf
                    ),


        model_data_t("../../../resources/models/vao/trefoil.vao",
                     "../../../resources/tex2d/rock.png",
                     glm::translate(glm::vec3(0.0f, 41.7f, 0.0f)),
                     glm::vec3(0.17f),                                                      // Ka
                     glm::vec3(0.50f),                                                      // Kd
                     glm::vec3(0.33f),                                                      // Ks
                     20.0f,                                                                 // Ns
                     0.1875f                                                                // bf
                    )
    };


    //===================================================================================================================================================================================================================
    // Load models and textures
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    const int MODEL_COUNT = sizeof(model_data) / sizeof(model_data_t);
    window.model_count = MODEL_COUNT;

    vao_t models[MODEL_COUNT];
    for (int i = 0; i < MODEL_COUNT; ++i)
    {
        //================================================================================================================================================================================================================
        // Load PN - model from file. First, read buffer params
        //================================================================================================================================================================================================================
        debug_msg("Loading model :: %s ... \n", model_data[i].model_path);
        vao_t& model = models[i];
        vao_t::header_t header;

        FILE* f = fopen(model_data[i].model_path, "rb");
        fread (&header, sizeof(vao_t::header_t), 1, f);

        glGenVertexArrays(1, &model.id);
        glBindVertexArray(model.id);
        
        //================================================================================================================================================================================================================
        // check that the input is valid PN - vao file and set up vertex attributes layout in buffer
        //================================================================================================================================================================================================================
        if (header.layout != vertex_pn_t::layout)
            exit_msg("File %s does not contain a valid PN - model. Layout = %d", model_data[i].model_path, header.layout);
        model.vbo.size = header.vbo_size;
        model.vbo.layout = header.layout;
        glGenBuffers(1, &model.vbo.id);
        glBindBuffer(GL_ARRAY_BUFFER, model.vbo.id);
        GLsizei stride = vertex_pn_t::total_dimension * sizeof(GLfloat);        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const GLfloat*) offsetof(vertex_pn_t, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const GLfloat*) offsetof(vertex_pn_t, normal));
        
        //================================================================================================================================================================================================================
        // load buffer to memory and normalize the model, e.g put its center of mass to the origin and align
        // principal momenta axes with coordinate axes
        //================================================================================================================================================================================================================
        vertex_pn_t* vbo_ptr = (vertex_pn_t*) malloc(header.vbo_size * stride);
        fread(vbo_ptr, stride, header.vbo_size, f);

        int V = header.vbo_size;

        glm::dvec3 mass_center;
        glm::dmat3 covariance_matrix;

        momenta::calculate(vbo_ptr, V, mass_center, covariance_matrix);
    
        glm::dquat q = diagonalizer(covariance_matrix);
        glm::dmat3 Q = mat3_cast(q);
        glm::dmat3 Qt = glm::transpose(Q);


        //================================================================================================================================================================================================================
        // after position --> Q * (position - mass_center) transform this (diagonal !) matrix will be the new covariance
        // matrix of the new point cloud, to normalize it remains to apply scale transform position --> position / sqrt(max momenta) 
        //================================================================================================================================================================================================================
        covariance_matrix = Q * covariance_matrix * Qt;
        double max_momentum = glm::max(glm::max(covariance_matrix[0][0], covariance_matrix[1][1]), covariance_matrix[2][2]);
        double inv_momentum = 7.25 / glm::sqrt(max_momentum);                                             
                                                                                                    
        for (int i = 0; i < V; ++i)
        {
            vertex_pn_t& vertex = vbo_ptr[i];        
            vertex.position = glm::vec3(inv_momentum * Q * (glm::dvec3(vertex.position) - mass_center));
            vertex.normal = glm::vec3(Qt * vertex.normal);
        }

        momenta::calculate(vbo_ptr, V, mass_center, covariance_matrix);
        debug_msg("After normalization :: mass_center = %s", glm::to_string(mass_center).c_str());
        debug_msg("                    :: covariance matrix = %s", glm::to_string(covariance_matrix).c_str());


        glBufferData(GL_ARRAY_BUFFER, header.vbo_size * stride, vbo_ptr, GL_STATIC_DRAW);
        free(vbo_ptr);
        
        //================================================================================================================================================================================================================
        // map index buffer to memory and read file data directly to it
        //================================================================================================================================================================================================================
        model.ibo.size = header.ibo_size;
        model.ibo.mode = header.mode;
        model.ibo.type = header.type;
        unsigned int index_size = (header.type == GL_UNSIGNED_INT) ? sizeof(GLuint) : (header.type == GL_UNSIGNED_SHORT) ? sizeof(GLushort) : sizeof(GLubyte);    
        glGenBuffers(1, &model.ibo.id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ibo.id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size * header.ibo_size, 0, GL_STATIC_DRAW);

        GLvoid* ibo_ptr = ibo_t::map(GL_WRITE_ONLY);
        fread(ibo_ptr, index_size, header.ibo_size, f);
        ibo_t::unmap();
        fclose(f);

        debug_msg("VAO Loaded :: \n\tvertex_count = %d. \n\tvertex_layout = %d. \n\tindex_type = %d. \n\tprimitive_mode = %d. \n\tindex_count = %d\n\n\n", 
                  model.vbo.size, model.vbo.layout, model.ibo.type, model.ibo.mode, model.ibo.size);

        debug_msg("Loading texture :: %s ... \n", model_data[i].texture_path);
        model_data[i].texture_id = image::png::texture2d(model_data[i].texture_path, 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    }

    //===================================================================================================================================================================================================================
    // Global OpenGL state
    //===================================================================================================================================================================================================================
    glClearColor(0.015f, 0.005f, 0.045f, 1.0f);
    glEnable(GL_DEPTH_TEST);   
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(-1); 
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    trilinear_blend.enable();

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        float time = glfw::time();
        const float light_radius = 43.0f;
        glm::vec3 light_ws = glm::vec3(light_radius * cos(0.5f * time), 25.0f, light_radius * sin(0.5f * time));
        glm::vec3 camera_ws = window.camera.position();
        //===============================================================================================================================================================================================================
        // Render terrain
        //===============================================================================================================================================================================================================
        terrain_shader.enable();
        uni_ts_projection_view_matrix = projection_view_matrix;
        uni_ts_camera_ws = camera_ws;
        uni_ts_light_ws = light_ws;
        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLE_STRIP, I, GL_UNSIGNED_INT, 0);

        //===============================================================================================================================================================================================================
        // Render models
        //===============================================================================================================================================================================================================
        trilinear_blend.enable();
        uniform_projection_view_matrix = projection_view_matrix;
        uniform_light_ws = light_ws;
        uniform_camera_ws = camera_ws;

        for (int i = 0; i < MODEL_COUNT; ++i)
        {
            uniform_model_matrix = model_data[i].model_matrix;
            uniform_Ka = model_data[i].Ka;
            uniform_Kd = model_data[i].Kd;
            uniform_Ks = model_data[i].Ks;
            uniform_Ns = model_data[i].Ns;
            uniform_bf = model_data[i].bf;
            glBindTexture(GL_TEXTURE_2D, model_data[i].texture_id);
            
            models[i].render();
        }

        //===============================================================================================================================================================================================================
        // Render normals of a currently active model
        //===============================================================================================================================================================================================================
        if (window.show_normals)
        {
            normal_renderer.enable();
            uni_nr_projection_view_matrix = projection_view_matrix;
            uni_nr_model_matrix = model_data[window.active_model].model_matrix;
            models[window.active_model].render();
        }        

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================    return 0;
    glfw::terminate();
    return 0;
}