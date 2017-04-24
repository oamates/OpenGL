//========================================================================================================================================================================================================================
// DEMO 018: SSBO Fractal Dodecahedron
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>

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

    int buffer_size = l * sizeof(glm::vec4);
    glm::vec4* fractal_shift = (glm::vec4*) malloc(buffer_size);

    for (int j = count - 1; j >= 0; --j)
    {
        glm::vec4 scaled_shift = scale * glm::vec4(shift[j], (j & 3) * 0.5f);
        for (int i = 0; i < count; ++i)
            fractal_shift[count * j + i] = scaled_shift + glm::vec4(shift[i], (i & 3) * 0.5f); 
    };

    int q = count * count;
    for (int k = 0; k < fractal_depth - 2; ++k)
    {
        for (int j = q - 1; j >= 0; --j)
        {
            glm::vec4 scaled_shift = scale * fractal_shift[j];
            for (int i = 0; i < count; ++i)
                fractal_shift[count * j + i] = scaled_shift + glm::vec4(shift[i], (i & 3) * 0.5f); 
        };
        q *= count;
    };

    GLuint ssbo_id;
    glGenBuffers(1, &ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, glm::value_ptr(fractal_shift[0]), GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ssbo_id, 0, buffer_size);

    return l;
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("SSBO Fractal Dodecahedron", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Creating shaders and uniforms
    //===================================================================================================================================================================================================================
    glsl_program_t fractal_program(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/fractal.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/fractal.fs"));

    fractal_program.enable();
    uniform_t uniform_projection_view_matrix = fractal_program["projection_view_matrix"];
    fractal_program["diffuse_texture"] = 0;

    //===================================================================================================================================================================================================================
    // create dodecahedron fractal shifts
    //===============================+====================================================================================================================================================================================

    int fractal_size = generate_fractal_shifts(plato::dodecahedron::vertices, 20, 0.3f, 4);
    debug_msg("fractal_size = %d", fractal_size);

    polyhedron dodecahedron;
    dodecahedron.regular_pnt2_vao(20, 12, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);

    //===================================================================================================================================================================================================================
    // load diffuse texture from file
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint pentagon_texture_id = image::png::texture2d("../../../resources/plato_tex2d/pentagon.png");
    glBindTexture(GL_TEXTURE_2D, pentagon_texture_id);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST enabled, GL_LESS - accept fragment if it closer to the camera than the former one
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.00f, 0.05f, 1.0f);                                                                                    // dark blue background
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        uniform_projection_view_matrix = projection_view_matrix;
        dodecahedron.instanced_render(fractal_size);        

        window.end_frame();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}