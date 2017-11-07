//========================================================================================================================================================================================================================
// DEMO 043: Strip visualizer
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "vao.hpp"
#include "vertex.hpp"
#include "font.hpp"
#include "fonts/kentucky_fireplace_regular.hpp"
#include "fonts/linguistics_pro_regular.hpp"
#include "fonts/poly_regular.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    GLuint strip_count = 0;
    GLuint strip_total;

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

        if ((key == GLFW_KEY_KP_ADD) && (strip_count < strip_total)) strip_count++;
        if ((key == GLFW_KEY_KP_SUBTRACT) && (strip_count != 0)) strip_count--;

        if ((key == GLFW_KEY_KP_MULTIPLY) && (strip_count + 8 <= strip_total)) strip_count += 8;
        if ((key == GLFW_KEY_KP_DIVIDE) && (strip_count >= 8)) strip_count -= 8;

        if ((key == GLFW_KEY_KP_9) && (strip_count + 64 <= strip_total)) strip_count += 64;
        if ((key == GLFW_KEY_KP_8) && (strip_count >= 64)) strip_count -= 64;

        if ((key == GLFW_KEY_KP_6) && (strip_count + 512 <= strip_total)) strip_count += 512;
        if ((key == GLFW_KEY_KP_5) && (strip_count >= 512)) strip_count -= 512;

        if ((key == GLFW_KEY_KP_3) && (strip_count + 4096 <= strip_total)) strip_count += 4096;
        if ((key == GLFW_KEY_KP_2) && (strip_count >= 4096)) strip_count -= 4096;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

int main(int argc, char* argv[])
{
    //===================================================================================================================================================================================================================
    // read and make sure we have a valid vao input file with GL_TRIANGLE strip primitive
    //===================================================================================================================================================================================================================
    if (argc != 2)
        exit_msg("\nUsage : %s <filename> ... \n", argv[0]);

    const char* file_name = argv[1];
    debug_msg("Reading %s", file_name);

    //================================================================================================================================================================================================================
    // read VAO params
    //================================================================================================================================================================================================================
    FILE* file = fopen(file_name, "rb");
    if (!file) 
        exit_msg("File %s not found.\n", file_name);

    vao_t::header_t header;
    debug_msg("Reading %s header ... \n", file_name);
    fread (&header, sizeof(vao_t::header_t), 1, file);
        
    //================================================================================================================================================================================================================
    // check that the primitive mode is GL_TRIANGES and the number of indices is divisible by 3
    //================================================================================================================================================================================================================
    if((header.mode != GL_TRIANGLE_STRIP) || (header.type != GL_UNSIGNED_INT))
    {
        if ((header.mode != GL_TRIANGLE_STRIP))
            debug_msg("\nIndex primitive mode (%x) is not GL_TRIANGLE_STRIP(%x).\n", header.mode, GL_TRIANGLE_STRIP);
        if (header.type != GL_UNSIGNED_INT)
            debug_msg("\nUnsupported index type : %x ...\n", header.type);
        fclose(file);
        return -1;
    }
    
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Strip visualizer", 8, 4, 0, 1920, 1080, true);

    //================================================================================================================================================================================================================
    // create VAO
    //================================================================================================================================================================================================================
    vao_t vao;
    glGenVertexArrays(1, &vao.id);
    glBindVertexArray(vao.id);
    
    //================================================================================================================================================================================================================
    // calculate attribute stride size in memory
    //================================================================================================================================================================================================================
    vao.vbo.size = header.vbo_size;
    vao.vbo.layout = header.layout;
    glGenBuffers(1, &vao.vbo.id);
    glBindBuffer(GL_ARRAY_BUFFER, vao.vbo.id);
    GLsizei stride = 0;        
    for(GLuint layout = header.layout; layout; layout >>= 2)
        stride += (1 + (layout & 0x3)) * sizeof(GLfloat);

    debug_msg("\n\tPrimitive mode is GL_TRIANGLE_STRIP (%x).\n\tNumber of indices : %i.\n", GL_TRIANGLE_STRIP, header.ibo_size);        
    debug_msg("\n\tAttribute buffer layout : %i.\n\tVBO size : %i vertices.\n\tAttribute size : %i bytes.\n\tTotal size : %i bytes.\n", header.layout, header.vbo_size, stride, header.vbo_size * stride);
        
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
    // map attribute buffer to memory and read file data directly to the address provided by OpenGL
    //================================================================================================================================================================================================================
    glBufferData(GL_ARRAY_BUFFER, header.vbo_size * stride, 0, GL_STATIC_DRAW);
    GLvoid* buf_ptr = vbo_t::map(GL_WRITE_ONLY);
    fread(buf_ptr, stride, header.vbo_size, file);
    vbo_t::unmap();
    
    //================================================================================================================================================================================================================
    // read index buffer to memory, store the number of strips and their length
    //================================================================================================================================================================================================================
    vao.ibo.size = header.ibo_size;
    vao.ibo.mode = header.mode;
    vao.ibo.type = header.type;
    glGenBuffers(1, &vao.ibo.id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao.ibo.id);

    GLuint* indices = (GLuint*) malloc(sizeof(GLuint) * header.ibo_size);
    fread(indices, sizeof(GLuint), header.ibo_size, file);            
    fclose(file);    

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * header.ibo_size, indices, GL_STATIC_DRAW);
    std::vector<GLuint> lengths;

    lengths.push_back(0);
    int id = 0;
    for (GLuint i = 0; i < header.ibo_size; ++i)
    {
        if (indices[i] == -1)
            lengths.push_back(i);

    }
    window.strip_total = lengths.size();
    lengths.push_back(header.ibo_size);
    free(indices);

    std::vector<GLint> strip_id;
    for (int i = 1; i < lengths.size(); ++i)
    {
        int l = (i == 1) ? lengths[1] - 2 : lengths[i] - lengths[i - 1] - 3;
        for (int j = 0; j < l; ++j)
           strip_id.push_back(i - 1);
    }

    debug_msg("The number of triangles is : %u", (unsigned int) strip_id.size());

    //===================================================================================================================================================================================================================
    // point data initialization and filling GL_SHADER_STORAGE_BUFFER
    //===================================================================================================================================================================================================================
    GLuint ssbo_id;
    glGenBuffers(1, &ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ssbo_id, 0, strip_id.size() * sizeof(GLint));
    glBufferData(GL_SHADER_STORAGE_BUFFER, strip_id.size() * sizeof(GLint), strip_id.data(), GL_DYNAMIC_COPY);

    //===================================================================================================================================================================================================================
    // initialize font shader and Kentucky fireplace font
    //===================================================================================================================================================================================================================
    glsl_program_t font_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/text2d.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/text2d.fs"));

    font_renderer.enable();
    uniform_t uniform_color_id   = font_renderer["text_color"];
    uniform_t uniform_shift_id   = font_renderer["shift"];
    uniform_t uniform_scale_id   = font_renderer["scale"];

    glActiveTexture(GL_TEXTURE0);
    text::font_t kf_font(text::kentucky_fireplace::regular::desc);
//    text::font_t kf_font(text::linguistics_pro::regular::desc);
//    text::font_t kf_font(text::poly::regular::desc);
    font_renderer["font_texture"] = 0;

    //===================================================================================================================================================================================================================
    // Load standard Blinn-Phong shader : no texture coordinates
    //===================================================================================================================================================================================================================
    glsl_program_t blinn_phong(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/blinn-phong.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blinn-phong.fs"));

    blinn_phong.enable();

    uniform_t uniform_projection_view_matrix = blinn_phong["projection_view_matrix"];
    uniform_t uniform_camera_ws              = blinn_phong["camera_ws"];
    uniform_t uniform_light_ws               = blinn_phong["light_ws"];
    uniform_t uniform_Ks                     = blinn_phong["Ks"];
    uniform_t uniform_Ns                     = blinn_phong["Ns"];

    //===================================================================================================================================================================================================================
    // Global OpenGL state : DEPTH test enabled, culling enabled
    //===================================================================================================================================================================================================================
    glClearColor(0.015f, 0.005f, 0.045f, 1.0f);
    glEnable(GL_DEPTH_TEST);   
    //glEnable(GL_CULL_FACE);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(-1);

    glm::vec3 Ks = glm::vec3(0.33f);
    uniform_Ks = Ks;
    uniform_Ns = 20.0f;
    const float light_radius = 100.0f;
    float startup_ts = window.frame_ts;

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        blinn_phong.enable();
        float time = window.frame_ts;
        glm::vec3 light_ws = glm::vec3(200.0f + light_radius * cos(time), 350.0f, light_radius * sin(time));
        glm::vec3 camera_ws = window.camera.position();
        uniform_light_ws = light_ws;
        uniform_camera_ws = camera_ws;

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        uniform_projection_view_matrix = projection_view_matrix;

        vao.render(lengths[window.strip_count], 0);

        //===============================================================================================================================================================================================================
        // setup text shader and render description and FPS
        //===============================================================================================================================================================================================================
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        font_renderer.enable();

        char buffer[128];            
        sprintf(buffer, "FPS : %.2f", window.frame / (time - startup_ts));
        uniform_color_id = glm::vec4(0.77f, 0.75f, 0.12f, 0.5f);
        uniform_shift_id = glm::vec2(0.55f, 0.85f);
        uniform_scale_id = glm::vec2(0.35f, 0.35f * window.aspect());
        kf_font.render_text(buffer, glm::vec2(0.0), glm::vec2(1.0));

        sprintf(buffer, "Strips : %u.\n Indices : %u. ", window.strip_count, lengths[window.strip_count]);
        uniform_color_id = glm::vec4( 0.77f, 0.75f, 0.12f, 0.75f);
        uniform_shift_id = glm::vec2(-0.9f, 0.9f);
        uniform_scale_id = glm::vec2(0.25f, 0.25f * window.aspect());
        kf_font.render_text(buffer, glm::vec2(0.0), glm::vec2(1.0));

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================    return 0;
    glfw::terminate();
    return 0;
}                               