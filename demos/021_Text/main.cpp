//========================================================================================================================================================================================================================
// DEMO 021: Signed Distance Field Text
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
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "font.hpp"

#include "fonts/apocalypse_regular.hpp"
#include "fonts/arvo_bold.hpp"
#include "fonts/arvo_bold_italic.hpp"
#include "fonts/arvo_italic.hpp"
#include "fonts/arvo_regular.hpp"
#include "fonts/comprehension_dark.hpp"
#include "fonts/comprehension_semibold.hpp"
#include "fonts/foliamix_regular.hpp"
#include "fonts/gravitas_one_regular.hpp"
#include "fonts/kentucky_fireplace_regular.hpp"
#include "fonts/linguistics_pro_bold.hpp"
#include "fonts/linguistics_pro_bold_italic.hpp"
#include "fonts/linguistics_pro_italic.hpp"
#include "fonts/linguistics_pro_regular.hpp"
#include "fonts/poly_italic.hpp"
#include "fonts/poly_regular.hpp"
#include "fonts/republica_minor_regular.hpp"
#include "fonts/sanlabello_solid.hpp"
#include "fonts/soria_regular.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    int fontID = 0;
    int FONT_COUNT;
    GLenum mode = GL_FILL;
    bool animate = true;

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
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
            mode = (mode == GL_FILL) ? GL_LINE : GL_FILL;

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE)) 
            animate = !animate;

        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE))
            fontID = (fontID == FONT_COUNT - 1) ? 0 : fontID + 1;

        if ((key == GLFW_KEY_KP_SUBTRACT) && (action == GLFW_RELEASE))
            fontID = (fontID == 0) ? FONT_COUNT - 1 : fontID - 1;
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

    demo_window_t window("Signed Distance Field Text", 8, 4, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // initialize fonts
    //===================================================================================================================================================================================================================
    text::font_desc_t font_desc[] = 
    {
        text::apocalypse::regular::desc,  
        text::arvo::bold::desc,                  
        text::arvo::bold_italic::desc,           
        text::arvo::italic::desc,                
        text::arvo::regular::desc,               
        text::comprehension::dark::desc,         
        text::comprehension::semibold::desc,     
        text::foliamix::regular::desc,           
        text::gravitas_one::regular::desc,       
        text::kentucky_fireplace::regular::desc, 
        text::linguistics_pro::bold::desc,       
        text::linguistics_pro::bold_italic::desc,
        text::linguistics_pro::italic::desc,     
        text::linguistics_pro::regular::desc,    
        text::poly::italic::desc,                
        text::poly::regular::desc,               
        text::republica_minor::regular::desc,    
        text::sanlabello::solid::desc,           
        text::soria::regular::desc
    };

    const int FONT_COUNT = sizeof(font_desc) / sizeof(font_desc[0]);
    window.FONT_COUNT = FONT_COUNT;

    text::font_t fonts[FONT_COUNT];
    GLuint font_vao_id[FONT_COUNT];
    vbo_t text_vbo[FONT_COUNT];

    glGenVertexArrays(FONT_COUNT, font_vao_id);

    for (int i = 0; i < FONT_COUNT; ++i)
    {
        glActiveTexture(GL_TEXTURE1 + i);
        fonts[i].init(font_desc[i]);
        glBindVertexArray(font_vao_id[i]);
        text_vbo[i] = fonts[i].create_vbo("0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz\nSpecial symbols : <>+-=?{}().,_,!@#$%^&*\nPress +/- to switch the font.", glm::vec2(0.0), glm::vec2(1.0));
    }



    glsl_program_t font_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/text2d.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/text2d.fs"));

    uniform_t uniform_sampler_id = font_renderer["font_texture"];
    uniform_t uniform_color_id   = font_renderer["text_color"];
    uniform_t uniform_shift_id   = font_renderer["shift"];
    uniform_t uniform_scale_id   = font_renderer["scale"];

    //===================================================================================================================================================================================================================
    // Tesselation shader
    //===================================================================================================================================================================================================================
    glsl_program_t tesselator(glsl_shader_t(GL_VERTEX_SHADER,          "glsl/simple.vs" ),
                              glsl_shader_t(GL_TESS_CONTROL_SHADER,    "glsl/simple.tcs"),
                              glsl_shader_t(GL_TESS_EVALUATION_SHADER, "glsl/simple.tes"),
                              glsl_shader_t(GL_FRAGMENT_SHADER,        "glsl/simple.fs" )); 

    tesselator.enable();
    uniform_t uniform_projection_view_matrix = tesselator["projection_view_matrix"];
    uniform_t uniform_camera_ws              = tesselator["camera_ws"];
    uniform_t uniform_base_scale             = tesselator["base_scale"];
    uniform_t uniform_time                   = tesselator["time"];

    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST enabled, GL_LESS - accept fragment if it closer to the camera than the former one
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glPatchParameteri(GL_PATCH_VERTICES, 1);


    uniform_base_scale = 3000.0f;
    float time = 0.0f;
    glActiveTexture(GL_TEXTURE0);
    GLuint water_texture_id = image::png::texture2d("../../../resources/tex2d/water2.png");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        glEnable(GL_DEPTH_TEST);
        tesselator.enable();
        glPolygonMode(GL_FRONT_AND_BACK, window.mode);
        glBindVertexArray(vao_id);

        if (window.animate)
        {
            time += 0.01;
            uniform_time = time;
        };

        glm::vec3 camera_ws = window.camera.position();
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();


        uniform_camera_ws = camera_ws;
        uniform_projection_view_matrix = projection_view_matrix;
        glDrawArraysInstanced(GL_PATCHES, 0, 1, 0x1000);


        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        font_renderer.enable();
        glDisable(GL_DEPTH_TEST);

        glBindVertexArray(font_vao_id[window.fontID]);
        uniform_sampler_id = window.fontID + 1;

        for (int i = 0; i < 5; ++i)
        {
            uniform_color_id = glm::vec3(0.77f, 0.85f - 0.125f * i * i, 0.12f);
            uniform_shift_id = glm::vec2(-0.85f, 0.85f - 0.15f * i * glm::sqrt(i));
            uniform_scale_id = glm::vec2(0.1f * (i + 1), 0.1f * (i + 1) * window.aspect());
            text_vbo[window.fontID].render(GL_TRIANGLES);
        }
  
        window.end_frame();
    }

    glfw::terminate();
    return 0;
}