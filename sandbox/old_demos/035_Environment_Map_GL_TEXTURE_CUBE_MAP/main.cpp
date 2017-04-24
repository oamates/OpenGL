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

#include "constants.hpp"
#include "glfw_window.hpp"
#include "log.hpp"
#include "camera3d.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "plato.hpp"
#include "polyhedron.hpp"

//========================================================================================================================================================================================================================
// 3d moving camera : standard initial orientation in space
//========================================================================================================================================================================================================================
const double linear_velocity = 0.7f;
const double angular_rate = 0.0001f;
static camera3d camera;

//========================================================================================================================================================================================================================
// keyboard and mouse handlers
//========================================================================================================================================================================================================================

void keyboard_handler(int key, int scancode, int action, int mods)
{
    if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(linear_velocity);  
    else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(linear_velocity); 
    else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(linear_velocity);
    else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(linear_velocity);
};

void mouse_handler(double dx, double dy, double duration)
{
    duration = glm::max(duration, 0.01);    
    double norm = sqrt(dx * dx + dy * dy);
    if (norm > 0.01f)
    {
        dx /= norm; dy /= norm;
        double angle = angular_rate * sqrt(norm) / (duration + 0.01);
        camera.rotateXY(dx, dy, angle);
    };
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

    std::unique_ptr<glm::vec4[]> fractal_shift(new glm::vec4[l]);

    for (int j = count - 1; j >= 0; --j)
    {
        glm::vec4 scaled_shift = scale * glm::vec4(shift[j], (j & 3) * 0.5f);
        for (int i = 0; i < count; ++i)
            fractal_shift[count * j + i] = scaled_shift + glm::vec4(shift[i], 0.0f); 
    };

    int q = count * count;
    for (int k = 0; k < fractal_depth - 2; ++k)
    {
        for (int j = q - 1; j >= 0; --j)
        {
            glm::vec4 scaled_shift = scale * fractal_shift[j];
            for (int i = 0; i < count; ++i)
                fractal_shift[count * j + i] = scaled_shift + glm::vec4(shift[i], 0.0f); 
        };
        q *= count;
    };
    for (int i = 0; i < l; ++i) fractal_shift[i].w = glm::linearRand(0.33f, 0.66f);

    GLuint ssbo_id;
    glGenBuffers(1, &ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, l * sizeof(glm::vec4), glm::value_ptr(fractal_shift[0]), GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ssbo_id, 0, l * sizeof(glm::vec4));

    return l;
};

int main()
{
    //===================================================================================================================================================================================================================
    // GLFW window creation + GLEW library initialization
    // 8AA samples, OpenGL 4.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    glfw_window window("Order Independent Transparency", 8, 4, 3, 1920, 1080);
    window.log_info();
    window.mouse_handler = mouse_handler;
    window.keyboard_handler = keyboard_handler;
    camera.infinite_perspective(constants::two_pi / 6.0f, window.aspect_ratio(), 0.1f);

	//===================================================================================================================================================================================================================
	// shader that creates list of fragments covering the given one and shader that sorts that list
	//===================================================================================================================================================================================================================

    glsl_program list_builder(glsl_shader(GL_VERTEX_SHADER,   "glsl/list_builder.vs"),
                              glsl_shader(GL_FRAGMENT_SHADER, "glsl/list_builder.fs"));

    GLint uniform_time                   = list_builder.uniform_id("time");
    GLint uniform_light_ws               = list_builder.uniform_id("light_ws");
    GLint uniform_camera_ws              = list_builder.uniform_id("camera_ws");
    GLint uniform_projection_view_matrix = list_builder.uniform_id("projection_view_matrix");

    glsl_program list_sorter(glsl_shader(GL_VERTEX_SHADER,   "glsl/list_sorter.vs"),
                             glsl_shader(GL_FRAGMENT_SHADER, "glsl/list_sorter.fs"));


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
    // create the linked list storage buffer
    //===================================================================================================================================================================================================================
    glGenBuffers(1, &linked_list_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, linked_list_buffer);
    glBufferData(GL_TEXTURE_BUFFER, window.res_x * window.res_y * 32 * sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);

    //===================================================================================================================================================================================================================
    // bind it to a texture for use as a TBO
    //===================================================================================================================================================================================================================
    glGenTextures(1, &linked_list_texture);
    glBindTexture(GL_TEXTURE_BUFFER, linked_list_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, linked_list_buffer);
    glBindImageTexture(1, linked_list_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32UI);

    //===================================================================================================================================================================================================================
    // generate dodecahedron model and SSBO buffer filled with shaft vector, the fourth component will decode object transparency
    //===================================================================================================================================================================================================================
    polyhedron dodecahedron;
    dodecahedron.regular_pnt2_vao(20, 12, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);

    int fractal_size = generate_fractal_shifts(plato::dodecahedron::vertices, 20, 0.3f, 2);
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
    	float time = window.time();
        glm::vec4 light_ws = glm::vec4(light_radius * cos(0.5f * time), light_radius * sin(0.5f * time), 0.0f, 1.0f);
        glm::vec4 camera_ws = glm::vec4(camera.position(), 1.0f);
        glm::mat4 projection_view_matrix = camera.projection_view_matrix();

        glClear(GL_COLOR_BUFFER_BIT);

        //===============================================================================================================================================================================================================
        // reset atomic counter 
        //===============================================================================================================================================================================================================
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomic_counter_buffer);
        GLuint* data = (GLuint*) glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_WRITE_ONLY);
        *data = 0;
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

        //===============================================================================================================================================================================================================
        // clear list root pointers texture, note : head_pointer_clear_buffer is currently bound to GL_PIXEL_UNPACK_BUFFER target
        //===============================================================================================================================================================================================================
        glBindTexture(GL_TEXTURE_2D, head_pointer_texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, window.res_x, window.res_y, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);

        //===============================================================================================================================================================================================================
        // root pointers image and linked-list buffer texture are currently bound to image units
        //===============================================================================================================================================================================================================

        list_builder.enable();
        glUniform1f(uniform_time, time);
        glUniform3fv(uniform_light_ws, 1, glm::value_ptr(light_ws));
        glUniform3fv(uniform_camera_ws, 1, glm::value_ptr(camera_ws));
        glUniformMatrix4fv(uniform_projection_view_matrix, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));
        dodecahedron.instanced_render(fractal_size);

        //===============================================================================================================================================================================================================
        // bind full screen quad vao and process results with linked-list sorter-blender
        //===============================================================================================================================================================================================================
        glBindVertexArray(quad_vao_id);
        list_sorter.enable();
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        window.swap_buffers();
        window.poll_events();
    };

    glDeleteBuffers(1, &quad_vbo_id);
    glDeleteVertexArrays(1, &quad_vao_id);

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    return 0;
};