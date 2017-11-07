//========================================================================================================================================================================================================================
// DEMO 030: Plane Shadows
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
#include <glm/gtc/matrix_transform.hpp> 

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "vbm.hpp"

#define DEPTH_TEXTURE_SIZE 1024
#define FRUSTUM_DEPTH       800.0f
/*
struct plane_projector
{
    fbo_t fbo;
    glm::mat4 view_matrix;
    

    plane_projector(GLuint res_x, GLuint res_y)
        : fbo(res_x, res_y)
    {
    }




    GLuint depth_fbo, depth_texture;

    // Create a depth texture
    glGenTextures(1, &depth_texture);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create FBO to render depth into
    glGenFramebuffers(1, &depth_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);
    glDrawBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    

}
*/


struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true /*, true */)
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

    demo_window_t window("OBJ Loader", 8, 3, 3, 1920, 1080, true);

	//===================================================================================================================================================================================================================
	// shadow and scene shaders initialization
	//===================================================================================================================================================================================================================
    glsl_program_t shadow_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/shadow.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/shadow.fs"));
    shadow_renderer.enable();
    uniform_t uniform_projection_view_model_matrix = shadow_renderer["projection_view_model_matrix"];

    glsl_program_t scene_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/scene.vs"),
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/scene.fs"));

    uniform_t uniform_model_matrix            = scene_renderer["model_matrix"];
    uniform_t uniform_view_matrix             = scene_renderer["view_matrix"];
    uniform_t uniform_projection_matrix       = scene_renderer["projection_matrix"];
    uniform_t uniform_shadow_matrix           = scene_renderer["shadow_matrix"];
    uniform_t uniform_light_position          = scene_renderer["light_position"];
    uniform_t uniform_material_ambient        = scene_renderer["material_ambient"];
    uniform_t uniform_material_diffuse        = scene_renderer["material_diffuse"];
    uniform_t uniform_material_specular       = scene_renderer["material_specular"];
    uniform_t uniform_material_specular_power = scene_renderer["material_specular_power"];
    uniform_t uniform_depth_texture           = scene_renderer["depth_texture"];

    scene_renderer.enable();
    uniform_depth_texture = 0;


    GLuint depth_fbo, depth_texture;

    // Create a depth texture
    glGenTextures(1, &depth_texture);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create FBO to render depth into
    glGenFramebuffers(1, &depth_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);
    glDrawBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    // Upload geometry for the ground plane
    static const float ground_vertices[] =
    {
        -500.0f, -50.0f, -500.0f, 1.0f,
        -500.0f, -50.0f,  500.0f, 1.0f,
         500.0f, -50.0f,  500.0f, 1.0f,
         500.0f, -50.0f, -500.0f, 1.0f,
    };

    static const float ground_normals[] =
    {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    GLuint ground_vao, ground_vbo;

    glGenVertexArrays(1, &ground_vao);
    glGenBuffers(1, &ground_vbo);
    glBindVertexArray(ground_vao);
    glBindBuffer(GL_ARRAY_BUFFER, ground_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ground_vertices) + sizeof(ground_normals), 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ground_vertices), ground_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(ground_vertices), sizeof(ground_normals), ground_normals);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)sizeof(ground_vertices));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    //===================================================================================================================================================================================================================
    // Load the object
    //===================================================================================================================================================================================================================
    VBObject object;
    object.LoadFromVBM("../../../resources/models/vbm/armadillo.vbm", 0, 1, 2);

    float inv_aspect = window.inv_aspect();

	//===================================================================================================================================================================================================================
	// The main loop
	//===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        float t = glfw::time() * 0.002f;
        static float q = 0.0f;
        static const glm::vec3 X(1.0f, 0.0f, 0.0f);
        static const glm::vec3 Y(0.0f, 1.0f, 0.0f);
        static const glm::vec3 Z(0.0f, 0.0f, 1.0f);

        glm::vec3 light_position = glm::vec3(sinf(t * 6.0f * 3.141592f) * 300.0f, 200.0f, cosf(t * 4.0f * 3.141592f) * 100.0f + 250.0f);

        // Setup
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // Matrices for rendering the scene
        glm::mat4 scene_model_matrix = glm::mat4(1.0f); //glm::rotate(t * 720.0f, Y);
        glm::mat4 scene_view_matrix = window.camera.view_matrix;//glm::translate(glm::vec3(0.0f, 0.0f, -300.0f));
        glm::mat4 scene_projection_matrix = window.camera.projection_matrix;//_matrixglm::frustum(-1.0f, 1.0f, -inv_aspect, inv_aspect, 1.0f, FRUSTUM_DEPTH);
        const glm::mat4 scale_bias_matrix = glm::mat4(glm::vec4(0.5f, 0.0f, 0.0f, 0.0f),
                                                      glm::vec4(0.0f, 0.5f, 0.0f, 0.0f),
                                                      glm::vec4(0.0f, 0.0f, 0.5f, 0.0f),
                                                      glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

    // Matrices used when rendering from the light's position
        glm::mat4 light_view_matrix = glm::lookAt(light_position, glm::vec3(0.0f), Y);
        glm::mat4 light_projection_matrix = window.camera.projection_matrix;//glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, FRUSTUM_DEPTH);

        // Now we render from the light's position into the depth buffer. Select the appropriate program
        shadow_renderer.enable();
        uniform_projection_view_model_matrix = light_projection_matrix * light_view_matrix * scene_model_matrix;

        // Bind the 'depth only' FBO and set the viewport to the size of the depth texture
        glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
        glViewport(0, 0, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE);

        // Clear
        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Enable polygon offset to resolve depth-fighting isuses
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 4.0f);

        // Draw from the light's point of view
        // Set material properties for the object

        // Draw the object
        object.Render();

        // Set material properties for the ground

        // Draw the ground
        glBindVertexArray(ground_vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        glDisable(GL_POLYGON_OFFSET_FILL);

        // Restore the default framebuffer and field of view
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window.res_x, window.res_y);

    // Now render from the viewer's position
        scene_renderer.enable();
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Setup all the matrices
        uniform_model_matrix      = scene_model_matrix;
        uniform_view_matrix       = scene_view_matrix;
        uniform_projection_matrix = scene_projection_matrix;
        uniform_shadow_matrix     = scale_bias_matrix * light_projection_matrix * light_view_matrix;
        uniform_light_position    = light_position;

    // Bind the depth texture
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glGenerateMipmap(GL_TEXTURE_2D);

    // Set material properties for the object
        uniform_material_ambient  = glm::vec3(0.1f, 0.0f, 0.2f);
        uniform_material_diffuse  = glm::vec3(0.3f, 0.2f, 0.8f);
        uniform_material_specular = glm::vec3(1.0f, 1.0f, 1.0f);
        uniform_material_specular_power = 25.0f;

        object.Render();

    // Set material properties for the ground
        uniform_material_ambient  = glm::vec3(0.1f, 0.1f, 0.1f);
        uniform_material_diffuse  = glm::vec3(0.1f, 0.5f, 0.1f);
        uniform_material_specular = glm::vec3(0.1f, 0.1f, 0.1f);
        uniform_material_specular_power = 3.0f;

    // Draw the ground
        glBindVertexArray(ground_vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        window.end_frame();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}