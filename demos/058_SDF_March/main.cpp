//========================================================================================================================================================================================================================
// DEMO 058 : SDF Texture 3D raymarcher
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
#include "shader.hpp"
#include "camera.hpp"
#include "image.hpp"
#include "sdf.hpp"
#include "vao.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;
    bool position_changed = false;

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
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) { camera.move_forward(frame_dt);   position_changed = true; }
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) { camera.move_backward(frame_dt);  position_changed = true; }
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) { camera.straight_right(frame_dt); position_changed = true; }
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) { camera.straight_left(frame_dt);  position_changed = true; }
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


//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    int res_x = 1920;
    int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("SDF Volume RayMarch", 4, 3, 3, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // skybox and environment map shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t sorter(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/sorter.cs"));
    sorter.enable();
    uniform_t uni_sorter_shift = sorter["shift"];
    uniform_t uni_sorter_triangles = sorter["triangles"];
    uniform_t uni_sorter_camera_ws = sorter["camera_ws"];

    //===================================================================================================================================================================================================================
    // skybox and environment map shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t skybox_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/skybox.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/skybox.fs"));

    skybox_renderer.enable();
    uniform_t uni_sbox_pv_matrix = skybox_renderer["projection_view_matrix"];
    skybox_renderer["environment_tex"] = 2;

    //===================================================================================================================================================================================================================
    // volume raymarch shader
    //===================================================================================================================================================================================================================
    glsl_program_t crystal_raymarch(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/sdf_march.vs"),
                                    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/sdf_march.fs"));

    crystal_raymarch.enable();

    uniform_t uni_cm_pv_matrix = crystal_raymarch["projection_view_matrix"];
    uniform_t uni_cm_camera_ws = crystal_raymarch["camera_ws"];         
    uniform_t uni_cm_light_ws  = crystal_raymarch["light_ws"];

    const float scale = 4.0f;
    crystal_raymarch["scale"] = scale;
    crystal_raymarch["inv_scale"] = 1.0f / scale;
    crystal_raymarch["tb_tex"] = 0;
    crystal_raymarch["value_tex"] = 1;
    crystal_raymarch["sdf_tex"] = 3;

    //===================================================================================================================================================================================================================
    // create dodecahecron buffer
    //===================================================================================================================================================================================================================
    vao_t model;
    model.init("../../../resources/models/vao/demon.vao");
    debug_msg("VAO Loaded :: \n\tvertex_count = %d. \n\tvertex_layout = %d. \n\tindex_type = %d. \n\tprimitive_mode = %d. \n\tindex_count = %d\n\n\n", 
              model.vbo.size, model.vbo.layout, model.ibo.type, model.ibo.mode, model.ibo.size);

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
    // create texture buffer access to vertices and to indices
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE4);
    GLuint indices_tbo_id;
    glGenTextures(1, &indices_tbo_id);
    glBindTexture(GL_TEXTURE_BUFFER, indices_tbo_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, model.ibo.id);
    glBindImageTexture(0, indices_tbo_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);    

    glActiveTexture(GL_TEXTURE5);
    GLuint vertices_tbo_id;
    glGenTextures(1, &vertices_tbo_id);
    glBindTexture(GL_TEXTURE_BUFFER, vertices_tbo_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, model.vbo.id);
    glBindImageTexture(1, vertices_tbo_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

    //===================================================================================================================================================================================================================
    // light variables
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);    

    texture3d_t demon_sdf(GL_TEXTURE3, "../../../resources/sdf/demon.sdf");

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = 0.25 * window.frame_ts;
        glm::vec3 light_ws = glm::vec3(10.0f, 2.0f * glm::cos(time), 3.0f * glm::sin(time));

        /* automatic camera */

        //float radius = 9.0f + 2.55f * glm::cos(0.25f * time);
        //float z = 1.45f * glm::sin(0.25f * time);
        //glm::vec3 camera_ws = glm::vec3(radius * glm::cos(0.3f * time), z, radius * glm::sin(0.3f * time));
        //glm::vec3 up = glm::normalize(glm::vec3(glm::cos(0.41 * time), -6.0f, glm::sin(0.41 * time)));
        //glm::mat4 view_matrix = glm::lookAt(camera_ws, glm::vec3(0.0f), up);
        //glm::mat4 projection_view_matrix = window.camera.projection_matrix * view_matrix;
        window.position_changed = true;

        /* hand-driven camera */
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 camera_ws = window.camera.position();
        

        //===============================================================================================================================================================================================================
        // render skybox
        //===============================================================================================================================================================================================================
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        skybox_renderer.enable();
        uni_sbox_pv_matrix = projection_view_matrix;
        skybox.render();

        //===============================================================================================================================================================================================================
        // raymarch through polyhedron
        //===============================================================================================================================================================================================================
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glCullFace(GL_BACK);
        crystal_raymarch.enable();
        uni_cm_pv_matrix = projection_view_matrix;
        uni_cm_camera_ws = camera_ws;
        uni_cm_light_ws = light_ws;
        model.render();

        //===============================================================================================================================================================================================================
        // sort triangles by distance to camera
        //===============================================================================================================================================================================================================
        if (window.position_changed)
        {
            sorter.enable();
            const int workgroup_size = 128;
            int triangles = model.ibo.size / 3;
            int workgroups = (triangles + workgroup_size - 1) / workgroup_size;

            uni_sorter_triangles = triangles;
            uni_sorter_shift = 0;
            uni_sorter_camera_ws = camera_ws;
            glDispatchCompute(workgroups, 1, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            uni_sorter_shift = 1;
            glDispatchCompute(workgroups, 1, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            window.position_changed = false;
        }

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}