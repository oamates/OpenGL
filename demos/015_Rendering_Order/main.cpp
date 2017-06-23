//========================================================================================================================================================================================================================
// DEMO 015: Rendering order
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    bool position_changed = false;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.5f);
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Alpha Blending", 8, 4, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // creating shaders and uniforms
    //=======================(============================================================================================================================================================================================
/*
    glsl_program_t sorter0(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/sorter0.cs"));
    sorter0.enable();
    uniform_t uni_s0_camera_ws = sorter0["camera_ws"];

    glsl_program_t sorter1(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/sorter1.cs"));
    sorter1.enable();
    uniform_t uni_s1_camera_ws = sorter1["camera_ws"];
*/
    glsl_program_t alpha_blender(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ab.vs"),
                                 glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/ab.gs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ab.fs"));

    alpha_blender.enable();

    uniform_t uni_ab_pv_matrix = alpha_blender["projection_view_matrix"];
    uniform_t uni_ab_camera_z  = alpha_blender["camera_z"];
    uniform_t uni_ab_camera_ws = alpha_blender["camera_ws"];
    uniform_t uni_ab_light_ws  = alpha_blender["light_ws"];
    uniform_t uni_ab_time      = alpha_blender["time"];

    alpha_blender["diffuse_tex"] = 0;
    alpha_blender["bump_tex"] = 1;

    //===================================================================================================================================================================================================================
    // point data initialization 
    //===================================================================================================================================================================================================================
    GLuint vao_id, pbo_id, vbo_id, ibo_id;

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    GLuint GROUP_SIZE = 2;
    GLuint GROUP_COUNT = 1;
    GLuint POINT_COUNT = GROUP_SIZE * GROUP_COUNT;
    
    std::vector<glm::mat3> point_frame;
    std::vector<glm::vec4> point_positions;

    for(GLuint i = 0; i < POINT_COUNT; ++i)
    {
        point_positions.push_back(glm::vec4(4.75f * glm::sphericalRand(1.0f), glm::gaussRand(0.0f, 0.25f)));

        glm::vec3 axis_x = glm::sphericalRand(1.0f);
        glm::vec3 axis_y = glm::normalize(glm::cross(axis_x, glm::sphericalRand(1.0f)));
        glm::vec3 axis_z = glm::sphericalRand(1.0f);
        point_frame.push_back(glm::mat3(axis_x, axis_y, axis_z));
    }

    glGenBuffers(1, &pbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, pbo_id);
    glBufferData(GL_ARRAY_BUFFER, POINT_COUNT * sizeof(glm::vec4), point_positions.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
/*
    GLuint position_tbo_id;
    glGenTextures(1, &position_tbo_id);
    glBindTexture(GL_TEXTURE_BUFFER, position_tbo_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, pbo_id);
    glBindImageTexture(1, position_tbo_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);    
*/
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, POINT_COUNT * sizeof(glm::mat3), point_frame.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (const GLvoid*) 0);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (const GLvoid*) 12);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (const GLvoid*) 24);

    //===================================================================================================================================================================================================================
    // prepare position buffer texture
    //===================================================================================================================================================================================================================
    glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, POINT_COUNT * sizeof(GLuint), 0, GL_DYNAMIC_COPY);

    GLuint* indices = (GLuint*) glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, POINT_COUNT * sizeof(GLuint), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    for (GLuint i = 0; i < POINT_COUNT; i++) indices[i] = i; 
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

/*
    GLuint indices_tbo_id;
    glGenTextures(1, &indices_tbo_id);
    glBindTexture(GL_TEXTURE_BUFFER, indices_tbo_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, ibo_id);
    glBindImageTexture(0, indices_tbo_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);    
*/
    //===================================================================================================================================================================================================================
    // Load diffuse texture
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint diff_tex_id = image::png::texture2d("../../../resources/plato_tex2d/cube_symm.png");

    glActiveTexture(GL_TEXTURE1);
    GLuint bump_tex_id = image::png::texture2d("../../../resources/plato_tex2d/cube_symm.png");

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST enabled, GL_LESS - accept fragment if it closer to the camera than the former one
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.00f, 0.05f, 1.0f);                                                                                // dark blue background
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        float time = window.frame_ts;
        float angle = 0.125 * time;
        glm::mat4& view_matrix = window.camera.view_matrix; 
        glm::vec3 light_ws = 15.0f * glm::vec3(glm::cos(angle), glm::sin(angle), 0.0f);
        glm::vec3 camera_z = glm::vec3(view_matrix[0][2], view_matrix[1][2], view_matrix[2][2]);
        glm::vec3 camera_ws = window.camera.position();
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        alpha_blender.enable();
        uni_ab_pv_matrix = projection_view_matrix;
        uni_ab_camera_z = camera_z;
        uni_ab_camera_ws = camera_ws;
        uni_ab_light_ws = light_ws;
        uni_ab_time = time;

        glDrawElements(GL_POINTS, POINT_COUNT, GL_UNSIGNED_INT, 0);

        if (window.position_changed)
        {
            /*
            sorter0.enable();
            uni_s0_camera_ws = camera_ws;
            glDispatchCompute(GROUP_COUNT / 2, 1, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);            

            sorter1.enable();
            uni_s1_camera_ws = camera_ws;
            glDispatchCompute(GROUP_COUNT / 2, 1, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            window.position_changed = false;
            */
        }

        window.end_frame();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}