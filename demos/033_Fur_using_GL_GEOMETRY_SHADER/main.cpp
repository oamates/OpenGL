//========================================================================================================================================================================================================================
// DEMO 033 : Fur using GL_GEOMETRY_SHADER
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/noise.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "vertex.hpp"
#include "surface.hpp"
#include "image.hpp"



struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    bool show_fur = true;
    float fur_depth = 5.0f;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================

    void on_key(int key, int scancode, int action, int mods) override
    {
        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE)) 
            { show_fur = !show_fur; return; }

        if ((key == GLFW_KEY_KP_ADD)) 
            { fur_depth += 0.1; return; }

        if ((key == GLFW_KEY_KP_SUBTRACT)) 
            { fur_depth -= 0.1; return; }

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

vertex_pnt2_t torus_func(const glm::vec2& uv)
{
    vertex_pnt2_t vertex;
    vertex.uv = uv;

    float cos_2piu = glm::cos(constants::two_pi * uv.y + constants::pi);
    float sin_2piu = glm::sin(constants::two_pi * uv.y + constants::pi);
    float cos_2piv = glm::cos(constants::two_pi * uv.x);
    float sin_2piv = glm::sin(constants::two_pi * uv.x);

    float R = 0.7f;
    float r = 0.3f;

    vertex.position = 100.0f * glm::vec3(
                        (R + r * cos_2piu) * cos_2piv,
                        (R + r * cos_2piu) * sin_2piv,
                             r * sin_2piu);

    vertex.normal = glm::vec3(cos_2piu * cos_2piv, cos_2piu * sin_2piv, sin_2piu);

    return vertex;
}


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

    demo_window_t window("Fur using GL_GEOMETRY_SHADER", 4, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // create programs : one for particle compute, the other for render
    //===================================================================================================================================================================================================================
    glsl_program_t base_shader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/base.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/base.fs"));

    uniform_t base_view_matrix       = base_shader["view_matrix"];
    uniform_t base_projection_matrix = base_shader["projection_matrix"];

    glsl_program_t fur_geometry(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/fur.vs"),
                                glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/fur.gs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/fur.fs"));

    uniform_t fur_view_matrix       = fur_geometry["view_matrix"];
    uniform_t fur_projection_matrix = fur_geometry["projection_matrix"];
    uniform_t fur_depth_uniform     = fur_geometry["fur_depth"];
    //===================================================================================================================================================================================================================
    // generate random texture
    //===================================================================================================================================================================================================================

    glActiveTexture(GL_TEXTURE0);
    GLuint fur_texture;
    glGenTextures(1, &fur_texture);
    unsigned char * tex = (unsigned char *) malloc(1024 * 1024 * 4);
    memset(tex, 0, 1024 * 1024 * 4);

    int n, m;

    for (n = 0; n < 256; n++)
    {
        for (m = 0; m < 1270; m++)
        {
            int x = rand() & 0x3FF;
            int y = rand() & 0x3FF;
            tex[(y * 1024 + x) * 4 + 0] = (rand() & 0x3F) + 0xC0;
            tex[(y * 1024 + x) * 4 + 1] = (rand() & 0x3F) + 0xC0;
            tex[(y * 1024 + x) * 4 + 2] = (rand() & 0x3F) + 0xC0;
            tex[(y * 1024 + x) * 4 + 3] = n;
        }
    };

    glBindTexture(GL_TEXTURE_2D, fur_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    free(tex);

    surface_t torus1;
    torus1.generate_vao<vertex_pnt2_t>(torus_func, 100, 100);

    glActiveTexture(GL_TEXTURE1);
    GLuint torus_texture_id = image::png::texture2d("../../../resources/tex2d/rock.png");

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST disabled
    // * Blending enabled
    //===================================================================================================================================================================================================================
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(torus1.vao.ibo.pri);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LEQUAL);
    
    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        base_shader.enable();
        base_view_matrix = window.camera.view_matrix;
        base_projection_matrix = window.camera.projection_matrix;
        torus1.render();

        if (window.show_fur)
        {
            fur_geometry.enable();
            fur_depth_uniform = window.fur_depth;
            fur_view_matrix = window.camera.view_matrix;
            fur_projection_matrix = window.camera.projection_matrix;
            glEnable(GL_BLEND);
            glDepthMask(GL_FALSE);
            torus1.render();
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        window.end_frame();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}