//========================================================================================================================================================================================================================
// DEMO 062 : GLSL Noise Speed Test
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

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "utils.hpp"
#include "vao.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "glsl_noise.hpp"
#include "font.hpp"
#include "fonts/kentucky_fireplace_regular.hpp"
#include "fonts/linguistics_pro_regular.hpp"
#include "fonts/poly_regular.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    double scroll_y = 0.0;
    GLenum mode = GL_FILL;
    bool animate = true;
    int active_program;
    int PROGRAM_COUNT;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, frue */)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        camera.view_matrix = glm::translate(glm::vec3(0.0f, 0.0f, -9000.0f));
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
            active_program = (active_program == PROGRAM_COUNT - 1) ? 0 : active_program + 1;

        if ((key == GLFW_KEY_KP_SUBTRACT) && (action == GLFW_RELEASE))
            active_program = (active_program == 0) ? PROGRAM_COUNT - 1 : active_program - 1;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }

    void on_scroll(double xoffset, double yoffset)
        { scroll_y += yoffset; }

};

struct program_desc_t
{
    const char* name;
    const char* tes_fname;
};

struct program_data_t
{
    glsl_program_t tesselator;
    GLuint uniform_projection_view_matrix,
           uniform_camera_ws, 
           uniform_time;
    GLuint desc_vao_id;
    vbo_t desc_vbo;
    GLuint source_vao_id;
    vbo_t source_vbo;
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

    demo_window_t window("OBJ Loader", 8, 4, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // generate noise texture and load grass/dirt/snow textures for terrain diffuse coloring
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0); GLuint noise_tex = glsl_noise::randomRGBA_shift_tex256x256(glm::ivec2(37, 17));
    glActiveTexture(GL_TEXTURE1); GLuint grass_tex = image::png::texture2d("../../../resources/tex2d/grass.png");
    glActiveTexture(GL_TEXTURE2); GLuint dirt_tex  = image::png::texture2d("../../../resources/tex2d/dirt.png");
    glActiveTexture(GL_TEXTURE3); GLuint snow_tex  = image::png::texture2d("../../../resources/tex2d/snow.png");

    //===================================================================================================================================================================================================================
    // initialize font shader and Kentucky fireplace font
    //===================================================================================================================================================================================================================
    glsl_program_t font_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/text2d.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/text2d.fs"));

    font_renderer.enable();
    uniform_t uniform_color_id = font_renderer["text_color"];
    uniform_t uniform_shift_id = font_renderer["shift"];
    uniform_t uniform_scale_id = font_renderer["scale"];

    glActiveTexture(GL_TEXTURE4);
    text::font_t kf_font(text::kentucky_fireplace::regular::desc);
//    text::font_t kf_font(text::linguistics_pro::regular::desc);
//    text::font_t kf_font(text::poly::regular::desc);
    font_renderer["font_texture"] = 4;
    
    //===================================================================================================================================================================================================================
    // initialize font shader and Kentucky fireplace font
    //===================================================================================================================================================================================================================
    program_desc_t program_desc[] = 
    {
        {"Function :: value 2D Noise with analytic derivative",    "glsl/tess_v2d.tes"},
        {"Function :: gradient 2D Noise with analytic derivative", "glsl/tess_g2d.tes"},
        {"Function :: simplex 2D Noise with analytic derivative",  "glsl/tess_s2d.tes"}
    };

    const int PROGRAM_COUNT = sizeof(program_desc) / sizeof(program_desc_t);

    program_data_t program_data[PROGRAM_COUNT];

    //===================================================================================================================================================================================================================
    // compile tesselation shaders based on different evaluation stage
    //===================================================================================================================================================================================================================
    const int N = 128;
    const int SQR_SIZE = N * N;

    glsl_shader_t vs(GL_VERTEX_SHADER,        "glsl/tess.vs");
    glsl_shader_t tcs(GL_TESS_CONTROL_SHADER, "glsl/tess.tcs");
    glsl_shader_t fs(GL_FRAGMENT_SHADER,      "glsl/tess.fs");

    for(int i = 0; i < PROGRAM_COUNT; ++i)
    {
        char* shader_source = utils::file_read(program_desc[i].tes_fname);
        glsl_shader_t tes(shader_source, GL_TESS_EVALUATION_SHADER);



        glsl_program_t& tesselator = program_data[i].tesselator;
        tesselator.link(vs, tcs, tes, fs);
        tesselator.enable();

        program_data[i].uniform_projection_view_matrix = tesselator["projection_view_matrix"];
        program_data[i].uniform_camera_ws              = tesselator["camera_ws"];
        program_data[i].uniform_time                   = tesselator["time"];

        tesselator["value_texture"] = 0;
        tesselator["grass"] = 1;
        tesselator["dirt"] = 2;
        tesselator["snow"] = 3;
        tesselator["lattice_scale"] = 512.0f;
        tesselator["normalizer"] = 1.0f / N;

        glGenVertexArrays(1, &program_data[i].desc_vao_id);
        glBindVertexArray(program_data[i].desc_vao_id);
        program_data[i].desc_vbo = kf_font.create_vbo(program_desc[i].name, glm::vec2(0.0), glm::vec2(1.0));

        glGenVertexArrays(1, &program_data[i].source_vao_id);
        glBindVertexArray(program_data[i].source_vao_id);
        program_data[i].source_vbo = kf_font.create_vbo(shader_source, glm::vec2(0.0), glm::vec2(1.0));

        free(shader_source);
    }   

    window.PROGRAM_COUNT = PROGRAM_COUNT;
    window.active_program = 0;


    //===================================================================================================================================================================================================================
    // N x N grid buffer setup
    //===================================================================================================================================================================================================================

    GLuint vao_id, vbo_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, SQR_SIZE * sizeof(glm::vec2), 0, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glm::vec2* data = (glm::vec2*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    int index = 0;
    float q = -0.5f * (N - 1);
    glm::vec2 lattice_point;
    lattice_point.y = q;
    for(int y = 0; y < N; ++y)
    {
        lattice_point.x = q;
        for(int x = 0; x < N; ++x)
        {
            data[index++] = lattice_point;
            lattice_point.x += 1.0f;
        }
        lattice_point.y += 1.0f;
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);


    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * DEPTH_TEST disabled for this simple benchmark
    //===================================================================================================================================================================================================================
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glPatchParameteri(GL_PATCH_VERTICES, 1);
    double startup_ts = glfw::time();

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, window.mode);

        //===============================================================================================================================================================================================================
        // setup tesselation shader and render terrain
        //===============================================================================================================================================================================================================
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glBindVertexArray(vao_id);

        int q = window.active_program;
        program_data[q].tesselator.enable();

        float time = glfw::time();
        glm::vec3 camera_ws = window.camera.position();
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        glUniform1f(program_data[q].uniform_time, time);
        glUniform3fv(program_data[q].uniform_camera_ws, 1, glm::value_ptr(camera_ws));
        glUniformMatrix4fv(program_data[q].uniform_projection_view_matrix, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));

        glDrawArrays(GL_PATCHES, 0, SQR_SIZE);

        //===============================================================================================================================================================================================================
        // setup text shader and render description and FPS
        //===============================================================================================================================================================================================================
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        font_renderer.enable();

        glBindVertexArray(program_data[q].desc_vao_id);
        uniform_color_id = glm::vec4(0.77f, 0.75f, 0.12f, 0.75f);
        uniform_shift_id = glm::vec2(0.05f, 0.925f);
        uniform_scale_id = glm::vec2(0.25f, 0.25f * window.aspect());
        program_data[q].desc_vbo.render(GL_TRIANGLES);

        glBindVertexArray(program_data[q].source_vao_id);
        uniform_color_id = glm::vec4( 0.37f, 0.95f, 0.22f, 0.45f);
        uniform_shift_id = glm::vec2(-0.95f, -0.025 * window.scroll_y + 0.955f);
        uniform_scale_id = glm::vec2(0.2f, 0.2f * window.aspect());
        program_data[q].source_vbo.render(GL_TRIANGLES);

        char buffer[128];
        double fps = window.frame / (window.frame_ts - startup_ts);            
        sprintf(buffer, "FPS : %.2f", fps);
        uniform_color_id = glm::vec4( 0.77f, 0.75f, 0.12f, 0.5f);
        uniform_shift_id = glm::vec2(0.70f, -0.85f);
        uniform_scale_id = glm::vec2(0.35f, 0.35f * window.aspect());
        kf_font.render_text(buffer, glm::vec2(0.0), glm::vec2(1.0));

        //===============================================================================================================================================================================================================
        // show back buffer and process events
        //===============================================================================================================================================================================================================
        window.end_frame();
    } 

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glDeleteTextures(1, &noise_tex);
    glDeleteTextures(1, &grass_tex);
    glDeleteTextures(1, &dirt_tex);
    glDeleteTextures(1, &snow_tex);

    glfw::terminate();
    return 0;
}