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
    for (int i = 0; i < l; ++i) fractal_shift[i].w = 0.33f;

    GLuint ssbo_id;
    glGenBuffers(1, &ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, l * sizeof(glm::vec4), glm::value_ptr(fractal_shift[0]), GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ssbo_id, 0, l * sizeof(glm::vec4));

    free(fractal_shift);
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

    list_builder.enable();
    uniform_t uniform_light_ws               = list_builder["light_ws"];
    uniform_t uniform_camera_ws              = list_builder["camera_ws"];
    uniform_t uniform_projection_view_matrix = list_builder["projection_view_matrix"];
    list_builder["tb_tex"] = 2;

    glsl_program_t list_sorter(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/list_sorter.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/list_sorter.fs"));

    //===================================================================================================================================================================================================================
    // create the atomic counter buffer with 1 element
    //===================================================================================================================================================================================================================
    GLuint acbo_id;
    glGenBuffers(1, &acbo_id);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, acbo_id);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), 0, GL_DYNAMIC_COPY);

    //===================================================================================================================================================================================================================
    // clear list root pointers texture and bind it to texture unit 0
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint hpointer_tex_id;
    glGenTextures(1, &hpointer_tex_id);
    glBindTexture(GL_TEXTURE_2D, hpointer_tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, window.res_x, window.res_y);
    glBindImageTexture(0, hpointer_tex_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    //===================================================================================================================================================================================================================
    // create the linked list storage buffer and bind it to a texture for use as a TBO
    //===================================================================================================================================================================================================================
    GLuint linked_list_buffer_id;
    glGenBuffers(1, &linked_list_buffer_id);
    glBindBuffer(GL_TEXTURE_BUFFER, linked_list_buffer_id);
    glBufferData(GL_TEXTURE_BUFFER, window.res_x * window.res_y * 32 * sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);

    //===================================================================================================================================================================================================================
    // buffer texture
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE1);
    GLuint linked_list_text_id;
    glGenTextures(1, &linked_list_text_id);
    glBindTexture(GL_TEXTURE_BUFFER, linked_list_text_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, linked_list_buffer_id);
    glBindImageTexture(1, linked_list_text_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32UI);

    //===================================================================================================================================================================================================================
    // generate dodecahedron model and SSBO buffer filled with shift vectors, the fourth component will encode object transparency
    //===================================================================================================================================================================================================================
    polyhedron dodecahedron;
    dodecahedron.regular_pnt2_vao(plato::dodecahedron::V, plato::dodecahedron::F, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);
    int fractal_size = generate_fractal_shifts(plato::dodecahedron::vertices, plato::dodecahedron::V, 0.40f, 2);

    //===================================================================================================================================================================================================================
    // create fake vao for full-screen quad rendering
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);

    //===================================================================================================================================================================================================================
    // Global OpenGL state : since there are no depth writes, depth buffer needs not be cleared
    //===================================================================================================================================================================================================================
    glDisable(GL_DEPTH_TEST);   
    glDisable(GL_CULL_FACE);

    glActiveTexture(GL_TEXTURE2);
    GLuint marble_tex_id = image::png::texture2d("../../../resources/tex2d/marble.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();

        float time = window.frame_ts;
        glm::vec3 light_ws = 15.0f * glm::vec3(cos(0.5f * time), sin(0.5f * time), 0.0f);
        glm::vec3 camera_ws = window.camera.position();
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        //===============================================================================================================================================================================================================
        // reset atomic counter and clear head pointers texture
        //===============================================================================================================================================================================================================
        GLuint zero = 0;
        glClearBufferData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
        glClearTexImage(hpointer_tex_id, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

        //===============================================================================================================================================================================================================
        // head pointers image and linked-list buffer texture are currently bound to image units 0 and 1
        //===============================================================================================================================================================================================================
        list_builder.enable();
        uniform_light_ws = light_ws;
        uniform_camera_ws = camera_ws;
        uniform_projection_view_matrix = projection_view_matrix;
        dodecahedron.instanced_render(fractal_size);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        //===============================================================================================================================================================================================================
        // bind full screen quad vao and process results with linked-list sorter-blender
        //===============================================================================================================================================================================================================
        glBindVertexArray(vao_id);
        list_sorter.enable();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}