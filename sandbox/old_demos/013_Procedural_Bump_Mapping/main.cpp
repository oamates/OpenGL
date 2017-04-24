//========================================================================================================================================================================================================================
// DEMO 013 : Procedural Textures
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>

#include "constants.hpp"
#include "glfw_window.hpp"
#include "log.hpp"
#include "camera3d.hpp"
#include "shader.hpp"
#include "texture.hpp"

//========================================================================================================================================================================================================================
// 3d moving camera : standard initial orientation in space
//========================================================================================================================================================================================================================
const float linear_velocity = 0.7f;
const float angular_rate = 0.0001f;
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
// program entry point
//=======================================================================================================================================================================================================================
int main()
{
    //===================================================================================================================================================================================================================
    // GLFW window creation + GLEW library initialization
    // 8AA samples, OpenGL 4.0 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    glfw_window window("Procedural Textures", 8, 4, 0, 1920, 1080);
    window.log_info();
    window.mouse_handler = mouse_handler;
    window.keyboard_handler = keyboard_handler;
    camera.infinite_perspective(constants::two_pi / 6.0f, window.aspect_ratio(), 0.1f);

	// ==================================================================================================================================================================================================================
	// loading cubemap texture
	// ==================================================================================================================================================================================================================
	const char * galaxy_files[6] = {"res/galaxy_positive_x.png", "res/galaxy_negative_x.png",
									"res/galaxy_positive_y.png", "res/galaxy_negative_y.png",
									"res/galaxy_positive_z.png", "res/galaxy_negative_z.png"};
	glActiveTexture(GL_TEXTURE0);
	GLuint cubemap = texture::cubemap_png(galaxy_files);

	// ==================================================================================================================================================================================================================
	// program cubemap texturing
	// ==================================================================================================================================================================================================================
    glsl_program procedural_cube(glsl_shader(GL_VERTEX_SHADER, "glsl/procedural_cube.vs"),
                                 glsl_shader(GL_FRAGMENT_SHADER, "glsl/procedural_cube.fs"));
	procedural_cube.enable();

	GLint model_matrix_id = procedural_cube.uniform_id("model_matrix");
	GLint view_matrix_id = procedural_cube.uniform_id("view_matrix");
	GLint projection_matrix_id = procedural_cube.uniform_id("projection_matrix");
	GLint time_id = procedural_cube.uniform_id("time");

	glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(camera.projection_matrix));

	const GLuint SUBROUTINE_COUNT = 12;
	GLuint procedural_subroutine[SUBROUTINE_COUNT];

	procedural_subroutine[0] = glGetSubroutineIndex(procedural_cube.id, GL_FRAGMENT_SHADER, "gradient_noise_color");
	procedural_subroutine[1] = glGetSubroutineIndex(procedural_cube.id, GL_FRAGMENT_SHADER, "turbulence_color");
	procedural_subroutine[2] = glGetSubroutineIndex(procedural_cube.id, GL_FRAGMENT_SHADER, "fractal_sum_color");
	procedural_subroutine[3] = glGetSubroutineIndex(procedural_cube.id, GL_FRAGMENT_SHADER, "cellular_noise_color");
	procedural_subroutine[4] = glGetSubroutineIndex(procedural_cube.id, GL_FRAGMENT_SHADER, "flow_noise_color");
	procedural_subroutine[5] = glGetSubroutineIndex(procedural_cube.id, GL_FRAGMENT_SHADER, "moving_cellular_color");
	procedural_subroutine[6] = glGetSubroutineIndex(procedural_cube.id, GL_FRAGMENT_SHADER, "moving_spots_color");
	procedural_subroutine[7] = glGetSubroutineIndex(procedural_cube.id, GL_FRAGMENT_SHADER, "star_plasma_color");
	procedural_subroutine[8] = glGetSubroutineIndex(procedural_cube.id, GL_FRAGMENT_SHADER, "unknown_color");
	procedural_subroutine[9] = glGetSubroutineIndex(procedural_cube.id, GL_FRAGMENT_SHADER, "marble_color");
	procedural_subroutine[10] = glGetSubroutineIndex(procedural_cube.id, GL_FRAGMENT_SHADER, "chessboard_color");
	procedural_subroutine[11] = glGetSubroutineIndex(procedural_cube.id, GL_FRAGMENT_SHADER, "experimental_color");

	// ==================================================================================================================================================================================================================
	// cubes
	// ==================================================================================================================================================================================================================
	glm::vec3 vertices [] =
	{
		glm::vec3(-1.0f, -1.0f, -1.0f),	
		glm::vec3(-1.0f,  1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f,  1.0f),
		glm::vec3(-1.0f,  1.0f,  1.0f),	
		glm::vec3( 1.0f, -1.0f, -1.0f),
		glm::vec3( 1.0f,  1.0f, -1.0f),
		glm::vec3( 1.0f, -1.0f,  1.0f),	
		glm::vec3( 1.0f,  1.0f,  1.0f)
	};
	GLubyte indices[] = {0,1,3, 0,3,2, 2,3,6, 3,7,6, 4,1,0, 4,5,1, 2,4,0, 2,6,4, 1,5,3, 5,7,3, 7,5,6, 6,5,4};

	GLuint vao_id, vbo_id, ibo_id;
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);
	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);     
	glEnable(GL_DEPTH_TEST);

	
	glm::mat4 model_matrix[SUBROUTINE_COUNT] = 
	{
		glm::translate(glm::vec3(-50.0f,   0.0f,   0.0f)),
		glm::translate(glm::vec3(  0.0f,   0.0f,   0.0f)),			
		glm::translate(glm::vec3( 50.0f,   0.0f,   0.0f)),
		glm::translate(glm::vec3(  0.0f, -50.0f,   0.0f)),
		glm::translate(glm::vec3(  0.0f,  50.0f,   0.0f)),
		glm::translate(glm::vec3( 50.0f,  50.0f,   0.0f)),
		glm::translate(glm::vec3(-50.0f,  50.0f,   0.0f)),
		glm::translate(glm::vec3( 50.0f, -50.0f,   0.0f)),			
		glm::translate(glm::vec3(-50.0f, -50.0f,   0.0f)),
		glm::translate(glm::vec3(  0.0f,   0.0f, -50.0f)),
		glm::translate(glm::vec3(  0.0f,   0.0f,  50.0f)),
		glm::translate(glm::vec3(-50.0f,   0.0f,  50.0f))
	};

	while(!window.should_close())
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);																    // clear the screen
		glUniform1f(time_id, glfwGetTime());
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(camera.view_matrix));

		for (GLuint i = 0; i < SUBROUTINE_COUNT; ++i)
		{
			glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, glm::value_ptr(model_matrix[i]));
			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &procedural_subroutine[i]);
        	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);
		};

        window.swap_buffers();
        window.poll_events();
    };
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
	return 0;
};