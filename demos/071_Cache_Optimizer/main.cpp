//========================================================================================================================================================================================================================
// DEMO 071 : Vertex Cache Optimizer
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <thread>
#include <atomic>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "vao.hpp"
#include "cache_optimizer.hpp"
#include "font.hpp"
#include "fonts/kentucky_fireplace_regular.hpp"

struct demo_window_t : public glfw_window_t
{
    enum {TESTING, OPTIMIZING, FREE_MODE};

    camera_t camera;

    static std::atomic_bool cache_optimized;
    unsigned int cache_size = 32;
    std::thread optimizer_thread;
    int state = FREE_MODE;

    GLuint vao_id;
    vbo_t vbo;
    ibo_t ibo;
    ibo_t opt_ibo;
    GLvoid *data_ptr = 0, *index_ptr = 0, *opt_index_ptr = 0;
    GLuint index_size;    

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
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

        if ((key == GLFW_KEY_R) && (action == GLFW_RELEASE))        // request to run cache optimization routine, only valid in free mode state
        {
            if (state == FREE_MODE)
            {
                state = OPTIMIZING;
                cache_optimized = false;
                optimizer_thread = std::thread(&optimize_cache, ibo.type, index_ptr, ibo.size, vbo.size, opt_index_ptr, cache_size);
                return;
            }
        }

        if ((key == GLFW_KEY_T) && (action == GLFW_RELEASE))        // request to run test with the given index buffer reordering, only valid in free mode state
        {
            if (state == FREE_MODE)
            {
                state = TESTING;
                return;
            }
        }

        if ((key == GLFW_KEY_KP_ADD ) && (cache_size < 63)) cache_size++;
        if ((key == GLFW_KEY_KP_SUBTRACT) && (cache_size > 8)) cache_size--;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
    
    static void optimize_cache(GLenum index_type, GLvoid *index_ptr, GLuint I, GLuint V, GLvoid *opt_index_ptr, GLuint cache_size)
    {
        if (index_type == GL_UNSIGNED_INT)
        {
            FaceOptimizer<GLuint> uint_optimizer;
            uint_optimizer.OptimizeFaces((GLuint*) index_ptr, I, V, (GLuint*) opt_index_ptr, (GLuint) cache_size);
        }
        else if (index_type == GL_UNSIGNED_SHORT)
        {
            FaceOptimizer<GLushort> ushort_optimizer;
            ushort_optimizer.OptimizeFaces((GLushort*) index_ptr, I, V, (GLushort*) opt_index_ptr, (GLushort) cache_size);

        }
        else // index_type == GL_UNSIGNED_BYTE
        {
            FaceOptimizer<GLubyte> ubyte_optimizer;
            ubyte_optimizer.OptimizeFaces((GLubyte*) index_ptr, I, V, (GLubyte*) opt_index_ptr, (GLubyte) cache_size);
        }
        cache_optimized = true;            
    }

    ~demo_window_t()
    {
        free(data_ptr);
        free(index_ptr);
        free(opt_index_ptr);
    }
};

std::atomic_bool demo_window_t::cache_optimized;

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

    demo_window_t window("Vertex Cache Optimizer", 8, 3, 3, 1920, 1080, true);


    //===================================================================================================================================================================================================================
    // initialize font shader and Kentucky fireplace font
    //===================================================================================================================================================================================================================
    glsl_program_t font_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/text2d.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/text2d.fs"));

    font_renderer.enable();
    uniform_t text_color = font_renderer["text_color"];
    uniform_t text_shift = font_renderer["shift"];
    uniform_t text_scale = font_renderer["scale"];

    const int FONT_TEXTURE = 0;
    glActiveTexture(GL_TEXTURE0 + FONT_TEXTURE);
    text::font_t kf_font(text::kentucky_fireplace::regular::desc);
    font_renderer["font_texture"] = FONT_TEXTURE;

    //===================================================================================================================================================================================================================
    // Load standard Blinn-Phong shader : no texture coordinates
    //===================================================================================================================================================================================================================
    glsl_program_t blinn_phong(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/blinn-phong.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blinn-phong.fs"));

    blinn_phong.enable();

    uniform_t projection_view_matrix = blinn_phong["projection_view_matrix"];
    uniform_t camera_ws              = blinn_phong["camera_ws"];
    uniform_t light_ws               = blinn_phong["light_ws"];

    blinn_phong["Ka"] = glm::vec3(0.17f);
    blinn_phong["Kd"] = glm::vec3(0.50f);
    blinn_phong["Ks"] = glm::vec3(0.33f);
    blinn_phong["Ns"] = 20.0f;
    
    //===================================================================================================================================================================================================================
    // Global OpenGL state : since there are no depth writes, depth buffer needs not be cleared
    //===================================================================================================================================================================================================================
    glClearColor(0.015f, 0.005f, 0.045f, 1.0f);
    glGenVertexArrays(1, &window.vao_id);
    glBindVertexArray(window.vao_id);

    //================================================================================================================================================================================================================
    // read buffer params
    //================================================================================================================================================================================================================
    const char* file_name = "../../../resources/models/vao/female_01.vao";
    vao_t::header_t header;
    FILE* f = fopen(file_name, "rb");
    fread (&header, sizeof(vao_t::header_t), 1, f);
        
    //================================================================================================================================================================================================================
    // calculate attribute stride size in memory
    //================================================================================================================================================================================================================
    window.vbo.size = header.vbo_size;
    window.vbo.layout = header.layout;
    glGenBuffers(1, &window.vbo.id);
    glBindBuffer(GL_ARRAY_BUFFER, window.vbo.id);
    GLsizei stride = 0;        
    for(GLuint layout = header.layout; layout; layout >>= 2)
        stride += (1 + (layout & 0x3)) * sizeof(GLfloat);

    debug_msg("V = %d", window.vbo.size);
    debug_msg("Layout = %d", window.vbo.layout);
    
    //================================================================================================================================================================================================================
    // set up vertex attributes layout in buffer
    //================================================================================================================================================================================================================
    unsigned int attr_id = 0, attr_offset = 0;
    
    for(GLuint layout = header.layout; layout; layout >>= 2)
    {
        glEnableVertexAttribArray(attr_id);
        GLuint attr_size = 1 + (layout & 0x3);
        glVertexAttribPointer(attr_id, attr_size, GL_FLOAT, GL_FALSE, stride, (float *) 0 + attr_offset);
        attr_offset += attr_size;
        attr_id++;
    }
    
    //================================================================================================================================================================================================================
    // read attribute data directly to memory
    //================================================================================================================================================================================================================
    GLuint attr_data_size = header.vbo_size * stride;
    window.data_ptr = malloc(attr_data_size);
    fread(window.data_ptr, stride, header.vbo_size, f);
    glBufferData(GL_ARRAY_BUFFER, attr_data_size, window.data_ptr, GL_STATIC_DRAW);
    
    //================================================================================================================================================================================================================
    // read index data directly to memory
    //================================================================================================================================================================================================================
    window.ibo.size = header.ibo_size;
    window.ibo.mode = header.mode;
    window.ibo.type = header.type;
    debug_msg("I = %d", window.ibo.size);
    debug_msg("Primitive Mode = %d", window.ibo.mode);
    debug_msg("Index type = %d", window.ibo.type);

    unsigned int index_size = (header.type == GL_UNSIGNED_INT) ? sizeof(GLuint) : (header.type == GL_UNSIGNED_SHORT) ? sizeof(GLushort) : sizeof(GLubyte);    
    GLuint index_data_size = index_size * header.ibo_size;
    window.index_ptr = malloc(index_data_size);
    fread(window.index_ptr, index_size, header.ibo_size, f);

    glGenBuffers(1, &window.opt_ibo.id);

    glGenBuffers(1, &window.ibo.id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, window.ibo.id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data_size, window.index_ptr, GL_STATIC_DRAW);            
    fclose(f);
    
    window.opt_index_ptr = malloc(index_data_size);
    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    const float light_radius = 200.0f;
    double startup_ts = window.frame_ts;

    while(!window.should_close())
    {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        float time = window.frame_ts;
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        blinn_phong.enable();
        projection_view_matrix = window.camera.projection_view_matrix();
        light_ws = glm::vec3(200.0f + light_radius * cos(time), 350.0f, light_radius * sin(time));
        camera_ws = window.camera.position();

        glBindVertexArray(window.vao_id);
        glDrawElements(window.ibo.mode, window.ibo.size, window.ibo.type, 0);

        //===============================================================================================================================================================================================================
        // check if the second thread has finished doing his job if there is any
        //===============================================================================================================================================================================================================
        if ((window.state == demo_window_t::OPTIMIZING) && (window.cache_optimized))
        {
            debug_msg("Index buffer optimized.");
            window.optimizer_thread.join();
            window.state = demo_window_t::FREE_MODE;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, window.opt_ibo.id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data_size, window.index_ptr, GL_STATIC_DRAW);            

        }

        //===============================================================================================================================================================================================================
        // show some text based on the current application state
        //===============================================================================================================================================================================================================
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        font_renderer.enable();

        char buffer[128];
        double fps = window.frame/ (window.frame_ts - startup_ts);            
        sprintf(buffer, "FPS : %.2f", fps);
        text_color = glm::vec4(0.77f, 0.75f, 0.12f, 0.5f);
        text_shift = glm::vec2(-0.70f, -0.85f);
        text_scale = glm::vec2(0.35f, 0.35f * window.aspect());
        kf_font.render_text(buffer, glm::vec2(0.0), glm::vec2(1.0));

        text_shift = glm::vec2(-0.70f, 0.85f);
        text_scale = glm::vec2(0.35f, 0.35f * window.aspect());
        const char* mode = (window.state == demo_window_t::FREE_MODE ) ? "Mode : free." : 
                           (window.state == demo_window_t::OPTIMIZING) ? "Mode : optimizing ..." : 
                                                                         "Mode : testing.";
        kf_font.render_text(mode, glm::vec2(0.0), glm::vec2(1.0));

        text_shift = glm::vec2(-0.70f, 0.75f);
        text_scale = glm::vec2(0.35f, 0.35f * window.aspect());
        char cache_size_info[128]; 
        sprintf(cache_size_info, "cache size : %u", window.cache_size);
        kf_font.render_text(cache_size_info, glm::vec2(0.0), glm::vec2(1.0));

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================    return 0;
    glfw::terminate();
    return 0;
}