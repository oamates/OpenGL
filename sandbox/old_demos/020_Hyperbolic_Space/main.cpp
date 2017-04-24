//========================================================================================================================================================================================================================
// DEMO 020: Hyperbolic Space Tesselation
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

#include "constants.hpp"
#include "glfw_window.hpp"
#include "log.hpp"
#include "camera3d.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "hyperbolic.hpp"

//========================================================================================================================================================================================================================
// 3d moving camera : standard initial orientation in space
//========================================================================================================================================================================================================================
const double linear_velocity = 0.01f;
const double angular_rate = 0.0001f;
static hyperbolic_camera3d camera;

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



int main()
{
    //===================================================================================================================================================================================================================
    // GLFW window creation + GLEW library initialization
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    glfw_window window("Hyperbolic space", 8, 4, 3, 1920, 1080);
    window.log_info();
    window.mouse_handler = mouse_handler;
    window.keyboard_handler = keyboard_handler;
    camera.infinite_perspective(constants::two_pi / 6.0f, window.aspect_ratio(), 0.1f);

	//===================================================================================================================================================================================================================
	// Creating shaders and uniforms
	//===================================================================================================================================================================================================================
    glsl_program hyperbolic(glsl_shader(GL_VERTEX_SHADER,          "glsl/hyperbolic.vs" ),
                            glsl_shader(GL_TESS_CONTROL_SHADER,    "glsl/hyperbolic.tcs"),
                            glsl_shader(GL_TESS_EVALUATION_SHADER, "glsl/hyperbolic.tes"),
                            glsl_shader(GL_GEOMETRY_SHADER,        "glsl/hyperbolic.gs" ),
                            glsl_shader(GL_FRAGMENT_SHADER,        "glsl/hyperbolic.fs" ));                                            

    hyperbolic.enable();
    GLuint uniform_view_matrix = hyperbolic.uniform_id("view_matrix");
	GLuint uniform_texture_sampler = hyperbolic.uniform_id("pentagon_texture");													        

	//===================================================================================================================================================================================================================
	// Right-angled hyperbolic dodecahedron triangulation initialization
	//===================================================================================================================================================================================================================
    GLuint vao_id = hyperbolic::dodecahedron::vao();
    GLuint ibo_id = hyperbolic::dodecahedron::ibo();
    GLuint ssbo_id = hyperbolic::dodecahedron::generate_SSBO5();

	//===================================================================================================================================================================================================================
	// Camera, view_matrix and projection_matrix initialization
	//===================================================================================================================================================================================================================
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    glActiveTexture(GL_TEXTURE0);
	GLuint pentagon_texture_id = texture::texture2d_png("res/pentagon.png");
    glBindTexture(GL_TEXTURE_2D, pentagon_texture_id);
    glUniform1i(uniform_texture_sampler, 0);																				// set our "texture_sampler" to use texture unit 0

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);


	//===================================================================================================================================================================================================================
	// The main loop
	//===================================================================================================================================================================================================================

	while(!window.should_close())
    {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 view_matrix_f = glm::mat4(camera.view_matrix);																    // clear the screen
        glUniformMatrix4fv(uniform_view_matrix, 1, GL_FALSE, glm::value_ptr(view_matrix_f));
        glDrawElementsInstanced(GL_PATCHES, 180, GL_UNSIGNED_BYTE, 0, 57741);
        window.swap_buffers();
        window.poll_events();
    }; 
    
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    return 0;
};