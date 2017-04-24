//========================================================================================================================================================================================================================
// DEMO 040: Order Independent Transparency
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

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "plato.hpp"
#include "polyhedron.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

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
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

//=======================================================================================================================================================================================================================
// function generates a buffer of fractal (geometric progression) shifts and returns its id
// the function can use arbitrary initial set of shift vectors therefore is made as generic as it could
// plato solid shifts among the most interesting examples, and there should be interesting others
// Serpinski cube mesh can be generated this way
//=======================================================================================================================================================================================================================
GLuint generate_fractal_shifts(const glm::vec3* shift, int count, float scale, int fractal_depth)
{
    //===================================================================================================================================================================================================================
    // for now, we just compute the displacement 
    //===================================================================================================================================================================================================================
    int l = 1;
    int d = fractal_depth;
    int c = count;
    while (d)
    {
        if (d & 1) l *= c;
        c *= c;
        d >>= 1;
    };

    glm::vec4* fractal_shift = (glm::vec4*) malloc(l * sizeof(glm::vec4));

    for (int j = count - 1; j >= 0; --j)
    {
        glm::vec4 scaled_shift = scale * glm::vec4(shift[j], (j & 3) * 0.5f);
        for (int i = 0; i < count; ++i)
            fractal_shift[count * j + i] = scaled_shift + glm::vec4(shift[i], 0.0f); 
    }

    int q = count * count;
    for (int k = 0; k < fractal_depth - 2; ++k)
    {
        for (int j = q - 1; j >= 0; --j)
        {
            glm::vec4 scaled_shift = scale * fractal_shift[j];
            for (int i = 0; i < count; ++i)
                fractal_shift[count * j + i] = scaled_shift + glm::vec4(shift[i], 0.0f); 
        }
        q *= count;
    }
    for (int i = 0; i < l; ++i) fractal_shift[i].w = 0.33f; //glm::linearRand(0.33f, 0.66f);

    GLuint ssbo_id;
    glGenBuffers(1, &ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, l * sizeof(glm::vec4), glm::value_ptr(fractal_shift[0]), GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ssbo_id, 0, l * sizeof(glm::vec4));

    delete fractal_shift;
    return l;
}

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Order Independent Transparency", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // shader that creates list of fragments covering the given one and shader that sorts that list
    //===================================================================================================================================================================================================================

    glsl_program_t list_builder(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/list_builder.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/list_builder.fs"));

    uniform_t uniform_time                   = list_builder["time"];
    uniform_t uniform_light_ws               = list_builder["light_ws"];
    uniform_t uniform_camera_ws              = list_builder["camera_ws"];
    uniform_t uniform_projection_view_matrix = list_builder["projection_view_matrix"];

    glsl_program_t list_sorter(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/list_sorter.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/list_sorter.fs"));


    GLuint head_pointer_texture, head_pointer_clear_buffer, atomic_counter_buffer, linked_list_buffer, linked_list_texture;

    //===================================================================================================================================================================================================================
    // clear list root pointers texture and bind it to texture unit 0
    //===================================================================================================================================================================================================================
    glGenTextures(1, &head_pointer_texture);
    glBindTexture(GL_TEXTURE_2D, head_pointer_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, window.res_x, window.res_y, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
    glBindImageTexture(0, head_pointer_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    //===================================================================================================================================================================================================================
    // create buffer for clearing the head pointer texture
    //===================================================================================================================================================================================================================
    glGenBuffers(1, &head_pointer_clear_buffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, head_pointer_clear_buffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, window.res_x * window.res_y * sizeof(GLuint), 0, GL_STATIC_DRAW);
    GLuint* data = (GLuint*) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    memset(data, 0x00, window.res_x * window.res_y * sizeof(GLuint));
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

    //===================================================================================================================================================================================================================
    // create the atomic counter buffer with 1 element
    //===================================================================================================================================================================================================================
    glGenBuffers(1, &atomic_counter_buffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomic_counter_buffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), 0, GL_DYNAMIC_COPY);

    //===================================================================================================================================================================================================================
    // create the linked list storage buffer and bind it to a texture for use as a TBO
    //===================================================================================================================================================================================================================
    glGenBuffers(1, &linked_list_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, linked_list_buffer);
    glBufferData(GL_TEXTURE_BUFFER, window.res_x * window.res_y * 32 * sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);
    glGenTextures(1, &linked_list_texture);
    glBindTexture(GL_TEXTURE_BUFFER, linked_list_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, linked_list_buffer);
    glBindImageTexture(1, linked_list_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32UI);

    //===================================================================================================================================================================================================================
    // generate dodecahedron model and SSBO buffer filled with shift vectors, the fourth component will encode object transparency
    //===================================================================================================================================================================================================================
    //polyhedron tetrahedron;
    //tetrahedron.regular_pnt2_vao(4, 4, plato::tetrahedron::vertices, plato::tetrahedron::normals, plato::tetrahedron::faces);
    polyhedron tetrahedron;
    tetrahedron.regular_pnt2_vao(plato::dodecahedron::V, plato::dodecahedron::F, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);
//    int fractal_size = generate_fractal_shifts(plato::tetrahedron::vertices, 4, 0.5f, 5);
    int fractal_size = generate_fractal_shifts(plato::dodecahedron::vertices, plato::dodecahedron::V, 0.40f, 2);
    debug_msg("fractal_size = %d", fractal_size);

    //===================================================================================================================================================================================================================
    // create full-screen quad buffer
    //===================================================================================================================================================================================================================
    GLuint quad_vao_id, quad_vbo_id;

    glGenVertexArrays(1, &quad_vao_id);
    glBindVertexArray(quad_vao_id);

    const glm::vec2 quad[] =
    {
        glm::vec2(-1.0f, -1.0f),
        glm::vec2( 1.0f, -1.0f),
        glm::vec2( 1.0f,  1.0f),
        glm::vec2(-1.0f,  1.0f)
    };

    glGenBuffers(1, &quad_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //===================================================================================================================================================================================================================
    // Global OpenGL state : since there are no depth writes, depth buffer needs not be cleared
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glDisable(GL_DEPTH_TEST);   
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    const float light_radius = 200.0f; 

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        float time = window.frame_ts;
        glm::vec4 light_ws = glm::vec4(light_radius * cos(0.5f * time), light_radius * sin(0.5f * time), 0.0f, 1.0f);
        glm::vec4 camera_ws = glm::vec4(window.camera.position(), 1.0f);
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        glClear(GL_COLOR_BUFFER_BIT);

        //===============================================================================================================================================================================================================
        // reset atomic counter 
        //===============================================================================================================================================================================================================
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomic_counter_buffer);
        GLuint* data = (GLuint*) glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_WRITE_ONLY);
        *data = 0;
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

        //===============================================================================================================================================================================================================
        // clear list root pointers texture, head_pointer_clear_buffer is currently bound to GL_PIXEL_UNPACK_BUFFER target
        //===============================================================================================================================================================================================================
        glBindTexture(GL_TEXTURE_2D, head_pointer_texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, window.res_x, window.res_y, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);

        //===============================================================================================================================================================================================================
        // root pointers image and linked-list buffer texture are currently bound to image units
        //===============================================================================================================================================================================================================
        list_builder.enable();
        uniform_time = time;
        uniform_light_ws = light_ws;
        uniform_camera_ws = camera_ws;
        uniform_projection_view_matrix = projection_view_matrix;
        tetrahedron.instanced_render(fractal_size);

        //===============================================================================================================================================================================================================
        // bind full screen quad vao and process results with linked-list sorter-blender
        //===============================================================================================================================================================================================================
        glBindVertexArray(quad_vao_id);
        list_sorter.enable();
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        window.end_frame();
    }

    glDeleteBuffers(1, &quad_vbo_id);
    glDeleteVertexArrays(1, &quad_vao_id);

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}