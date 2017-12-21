//========================================================================================================================================================================================================================
// DEMO 078: Environment mapping
//========================================================================================================================================================================================================================
#include <random>
#include <cstdlib>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "fbo.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
          camera(8.0f, 0.5f, glm::lookAt(glm::vec3(4.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
    {
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("VAO Loader", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // init shader
    //===================================================================================================================================================================================================================
    glsl_program_t prog = glsl_program_t(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/cubemap.vs"),
                                         glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/cubemap.fs"));
    prog.enable();
    uniform_t uni_camera_matrix = prog["CameraMatrix"];
    uniform_t uni_model_matrix  = prog["ModelMatrix"];
    prog["ProjectionMatrix"] = window.camera.projection_matrix;
    prog["TexUnit"] = 0;
    prog["LightPos"] = glm::vec3(3.0f, 5.0f, 4.0f);


//#include <oglplus/shapes/spiral_sphere.hpp>

    //===================================================================================================================================================================================================================
    // create VAO
    //===================================================================================================================================================================================================================
    shapes::SpiralSphere make_shape;                    // helper object building shape vertex attributes
    shapes::DrawingInstructions shape_instr(make_shape.Instructions());         // helper object encapsulating shape drawing instructions
    shapes::SpiralSphere::IndexArray shape_indices(make_shape.Indices());       // indices pointing to shape primitive elements

    VertexArray shape;                              // A vertex array object for the rendered shape
    Buffer verts, normals;                          // VBOs for the shape's vertex attributes
    Texture tex;                                    // The environment cube map

        // bind the VAO for the shape
        shape.Bind();

        verts.Bind(Buffer::Target::Array);
        {
            std::vector<GLfloat> data;
            GLuint n_per_vertex = make_shape.Positions(data);
            Buffer::Data(Buffer::Target::Array, data);
            VertexArrayAttrib attr(prog, "Position");
            attr.Setup<GLfloat>(n_per_vertex);
            attr.Enable();
        }

        normals.Bind(Buffer::Target::Array);
        {
            std::vector<GLfloat> data;
            GLuint n_per_vertex = make_shape.Normals(data);
            Buffer::Data(Buffer::Target::Array, data);
            VertexArrayAttrib attr(prog, "Normal");
            attr.Setup<GLfloat>(n_per_vertex);
            attr.Enable();
        }

        GLuint tex_side = 256;

    //===================================================================================================================================================================================================================
    // generate cubemap texture
    //===================================================================================================================================================================================================================
    GLuint cmap_tex_id;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &cmap_tex_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cmap_tex_id);

//#include <oglplus/images/newton.hpp>

    auto image = images::NewtonFractal(
        tex_side, tex_side,
        glm::vec3(0.3f, 0.1f, 0.2f),
        glm::vec3(1.0f, 0.8f, 0.9f),
        glm::vec2(-1.0f, -1.0f),
        glm::vec2( 1.0f,  1.0f),
        images::NewtonFractal::X4Minus1(),
        images::NewtonFractal::DefaultMixer()
    );

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB, tex_side, tex_side);
    for(int i = 0; i != 6; ++i)
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, tex_side, tex_side, GL_RGB, GL_UNSIGNED_BYTE, image);

    //===================================================================================================================================================================================================================
    // global OpenGL settings
    //===================================================================================================================================================================================================================
    glClearColor(0.2f, 0.05f, 0.1f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    //===================================================================================================================================================================================================================
    // program main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        uni_camera_matrix = window.camera.view_matrix;
        uni_model_matrix = glm::mat4(1.0f);

        shape.Bind();
        shape_instr.Draw(shape_indices);

        //===============================================================================================================================================================================================================
        // show back buffer
        //===============================================================================================================================================================================================================
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}
