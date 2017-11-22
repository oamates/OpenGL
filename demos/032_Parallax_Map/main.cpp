//========================================================================================================================================================================================================================
// DEMO 032: Parallax mapping technique
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "log.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "gl_aux.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "vao.hpp"
#include "vertex.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    bool pause = false;
    bool division = true;
    bool parallax = true;
    float height_scale = -0.007f;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */),
          camera(16.0f, 0.5f, glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.125f);
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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

        if (action != GLFW_RELEASE) return;

        if (key == GLFW_KEY_RIGHT_CONTROL) pause = !pause;
        if (key == GLFW_KEY_SPACE)         parallax = !parallax;
        if (key == GLFW_KEY_ENTER)         division = !division;
        if (key == GLFW_KEY_KP_ADD)        height_scale += 0.001;
        if (key == GLFW_KEY_KP_SUBTRACT)   height_scale -= 0.001;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

struct quad_pft2_t
{
    GLuint vao_id;
    vbo_t vbo;

    quad_pft2_t()
    {
        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);

        glm::vec3 pos1(-1.0f,  1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3( 1.0f, -1.0f, 0.0f);
        glm::vec3 pos4( 1.0f,  1.0f, 0.0f);

        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);

        glm::vec3 n(0.0f, 0.0f, 1.0f);
        glm::vec3 tangent_x(1.0f, 0.0f, 0.0f);
        glm::vec3 tangent_y(0.0f, 1.0f, 0.0f);

        vertex_pft2_t vertices[] = 
        {
            vertex_pft2_t(pos1, n, tangent_x, tangent_y, uv1),    
            vertex_pft2_t(pos2, n, tangent_x, tangent_y, uv2),    
            vertex_pft2_t(pos3, n, tangent_x, tangent_y, uv3),    
            vertex_pft2_t(pos1, n, tangent_x, tangent_y, uv1),    
            vertex_pft2_t(pos3, n, tangent_x, tangent_y, uv3),    
            vertex_pft2_t(pos4, n, tangent_x, tangent_y, uv4)
        };

        vbo.init(vertices, 6);
    }    

    void render()
    {
        glBindVertexArray(vao_id);
        vbo.render(GL_TRIANGLES);
    }

    void instanced_render(GLsizei primcount)
    {
        glBindVertexArray(vao_id);
        vbo.instanced_render(GL_TRIANGLES, primcount);
    }

    ~quad_pft2_t()
    {
        glDeleteVertexArrays(1, &vao_id);
    }    
};

int main(int argc, char *argv[])
{
    const int res_x = 1920;
    const int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW ImGui window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Parallax mapping", 4, 3, 3, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // phong lighting model shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t parallax_map(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/parallax.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/parallax.fs"));

    parallax_map.enable();

    uniform_t uni_pm_pv_matrix    = parallax_map["projection_view_matrix"];
    uniform_t uni_pm_model_matrix = parallax_map["model_matrix"];
    uniform_t uni_pm_camera_ws    = parallax_map["camera_ws"];
    uniform_t uni_pm_light_ws     = parallax_map["light_ws"];
    uniform_t uni_pm_height_scale = parallax_map["height_scale"];
    uniform_t uni_pm_parallax     = parallax_map["parallax"];
    uniform_t uni_pm_division     = parallax_map["division"];

    parallax_map["diffuse_tex"] = 0;
    parallax_map["normal_tex"]  = 1;
    parallax_map["height_tex"]  = 2;

    /*
    glActiveTexture(GL_TEXTURE0); GLuint diffuse_tex_id = image::png::texture2d("res/sofa_diffuse.png");
    glActiveTexture(GL_TEXTURE1); GLuint normal_tex_id  = image::png::texture2d("res/sofa_normal.png");
    glActiveTexture(GL_TEXTURE2); GLuint height_tex_id  = image::png::texture2d("res/sofa_height.png");
    */

    glActiveTexture(GL_TEXTURE0); GLuint diffuse_tex_id = image::png::texture2d("res/brick_diffuse.png");
    glActiveTexture(GL_TEXTURE1); GLuint normal_tex_id  = image::png::texture2d("res/brick_normal.png");
    glActiveTexture(GL_TEXTURE2); GLuint height_tex_id  = image::png::texture2d("res/brick_height.png");

    quad_pft2_t quad;

    glEnable(GL_DEPTH_TEST);
    float t = 0.0f;
    
    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        parallax_map.enable();

        const float light_radius = 1.71f;  

        if (!window.pause)
            t += window.frame_dt;

        float t0 = 0.427f * t;
        float t1 = 0.219f * t;
        float cs0 = glm::cos(t0);
        float sn0 = glm::sin(t0);
        float cs1 = glm::cos(t1);
        float sn1 = glm::sin(t1);

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::mat4 model_matrix = glm::mat4(1.0f);

        glm::vec3 camera_ws = window.camera.position();
        glm::vec3 light_ws = light_radius * glm::vec3(cs0 * cs1, cs0 * sn1, 1.0f - sn0 * sn0);

        uni_pm_pv_matrix = projection_view_matrix;
        uni_pm_model_matrix = model_matrix;
        uni_pm_camera_ws = camera_ws;
        uni_pm_light_ws = light_ws;

        uni_pm_height_scale = window.height_scale;
        uni_pm_parallax = (int) window.parallax;
        uni_pm_division = (int) window.division;
        
        quad.render();

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    GLuint tex_ids[] = {diffuse_tex_id, normal_tex_id, height_tex_id};
    glDeleteTextures(sizeof(tex_ids) / sizeof(GLuint), tex_ids);
    glfw::terminate();
    return 0;
}
