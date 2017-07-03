//========================================================================================================================================================================================================================
// DEMO 076 : Transform Feedback
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "glsl_noise.hpp"
#include "plato.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "polyhedron.hpp"
#include "image.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.01f);
    }

    //===================================================================================================================================================================================================================
    // mouse handlers
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
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    int res_x = 1920;
    int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("SDF Texture 3D generator", 4, 4, 3, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // geometry shader that generates point cloud at a given (signed) distance around a model
    //===================================================================================================================================================================================================================
    glsl_program_t cloud_gen(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/cloud_gen.vs"),
                             glsl_shader_t(GL_TESS_CONTROL_SHADER,    "glsl/cloud_gen.tcs"),
                             glsl_shader_t(GL_TESS_EVALUATION_SHADER, "glsl/cloud_gen.tes"),
                             glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/cloud_gen.gs"),
                             glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/cloud_gen.fs"));

    const char* varyings[] = {"cloud_point"};
    glTransformFeedbackVaryings(cloud_gen.id, 1, varyings, GL_INTERLEAVED_ATTRIBS);
    cloud_gen.link();
    cloud_gen.dump_info();
    cloud_gen.enable();
    cloud_gen["sigma"] = 0.03125f;
    cloud_gen["inv_max_edge"] = 1.0f / 0.0125f;

    //===================================================================================================================================================================================================================
    // load demon model
    //===================================================================================================================================================================================================================
    vao_t model;
    model.init("../../../resources/models/vao/demon.vao");
    GLuint size = 16 * model.ibo.size;


    debug_msg("Model loaded :: index buffer size = %u", model.ibo.size);
    debug_msg("Model primitive type = %u", model.ibo.mode);
    debug_msg("GL_TRIANGLES = %u, GL_TRIANGLE_STRIP = %u", GL_TRIANGLES, GL_TRIANGLE_STRIP);

    //===================================================================================================================================================================================================================
    // create transform feedback buffer
    //===================================================================================================================================================================================================================
    GLuint tbo_id;
    glGenBuffers(1, &tbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, tbo_id);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(glm::vec3), 0, GL_STATIC_READ);

    //===================================================================================================================================================================================================================
    // create query object to collect the number of output vertices and perform feedback transform
    //===================================================================================================================================================================================================================
    GLuint query_id;
    glGenQueries(1, &query_id);

    glEnable(GL_RASTERIZER_DISCARD);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo_id);
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query_id);
    glBeginTransformFeedback(GL_POINTS);
    model.render(GL_PATCHES);
    glEndTransformFeedback();
    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

    glDisable(GL_RASTERIZER_DISCARD);

    glFinish();
    GLuint cloud_size;
    glGetQueryObjectuiv(query_id, GL_QUERY_RESULT, &cloud_size);

    debug_msg("Point cloud generated. Size = %u", cloud_size);

    //===================================================================================================================================================================================================================
    // create point renderer and point vao
    //===================================================================================================================================================================================================================
    glsl_program_t cloud_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/cloud_render.vs"),
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/cloud_render.fs"));
    cloud_renderer.enable();
    uniform_t uni_cr_pv_matrix = cloud_renderer["projection_view_matrix"];

    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : background color -- dark blue
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.0f, 0.08f, 0.0f);

    //===================================================================================================================================================================================================================
    // main program loop : just clear the color and depth buffer in a loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        uni_cr_pv_matrix = window.camera.projection_view_matrix();
        glDrawArrays(GL_POINTS, 0, cloud_size);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glDeleteQueries(1, &query_id);
    glDeleteBuffers(1, &tbo_id);
    glDeleteVertexArrays(1, &vao_id);

    glfw::terminate();
    return 0;
}




