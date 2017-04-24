#include <iostream>
#include <random>
#include <stdlib.h>
#define GLEW_STATIC
#include <GL/glew.h> 														                                                // OpenGL extensions
#include <GLFW/glfw3.h>														                                                // windows and event management library

#include <glm/glm.hpp>														                                                // OpenGL mathematics
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/matrix_transform.hpp>										                                                // for transformation matrices to work
#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext.hpp> 
#include <glm/gtc/random.hpp>



#include "log.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "texture.hpp"

const unsigned int res_x = 1920;
const unsigned int res_y = 1080;

int main()
{

	// ==================================================================================================================================================================================================================
	// GLFW library initialization
	// ==================================================================================================================================================================================================================

	if(!glfwInit()) exit_msg("Failed to initialize GLFW.");                                                                 // initialise GLFW

	glfwWindowHint(GLFW_SAMPLES, 4); 																						// 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 																			// we want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 																	
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 															// request core profile
							
	GLFWwindow* window; 																									// open a window and create its OpenGL context 
	window = glfwCreateWindow(res_x, res_y, "Empty glfw application", glfwGetPrimaryMonitor(), 0); 
	if(!window)
	{
	    glfwTerminate();
		exit_msg("Failed to open GLFW window. No open GL 3.3 support.");
	}

	glfwMakeContextCurrent(window); 																						
    debug_msg("GLFW initialization done ... ");

	// ==================================================================================================================================================================================================================
	// GLEW library initialization
	// ==================================================================================================================================================================================================================

	glewExperimental = true; 																								// needed in core profile 
	GLenum result = glewInit();                                                                                             // initialise GLEW
	if (result != GLEW_OK) 
	{
		glfwTerminate();
    	exit_msg("Failed to initialize GLEW : %s", glewGetErrorString(result));
	}
	glClearColor(0.05f, 0.0f, 0.15f, 0.0f);																					// set dark blue background
	debug_msg("GLEW library initialization done ... ");

    debug_msg("GL_VENDOR = %s.", glGetString(GL_VENDOR));                                       
    debug_msg("GL_RENDERER = %s.", glGetString(GL_RENDERER));                                   
    debug_msg("GL_VERSION = %s.", glGetString(GL_VERSION));                                     
    debug_msg("GL_SHADING_LANGUAGE_VERSION = %s.", glGetString(GL_SHADING_LANGUAGE_VERSION));   
    debug_msg("GL_EXTENSIONS = %s.", glGetString(GL_EXTENSIONS));                               

	// ==================================================================================================================================================================================================================
	// init camera
	// ==================================================================================================================================================================================================================
	init_camera(window);
	const float two_pi = 6.283185307179586476925286766559; 
	glm::mat4 projection_matrix = glm::infinitePerspective (two_pi / 6.0f, float(res_x) / float(res_y), 0.1f); 		        						// projection matrix : 60° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units

	// ==================================================================================================================================================================================================================
	// loading cubemap texture
	// ==================================================================================================================================================================================================================
	const char * galaxy_files[6] = {"res/galaxy_positive_x.png",
									"res/galaxy_negative_x.png",
									"res/galaxy_positive_y.png",
									"res/galaxy_negative_y.png",
									"res/galaxy_positive_z.png",
									"res/galaxy_negative_z.png"};

	glActiveTexture(GL_TEXTURE0);
	GLuint cubemap0 = texture::cubemap_png(galaxy_files);

	// ==================================================================================================================================================================================================================
	// program cubemap texturing
	// ==================================================================================================================================================================================================================
    glsl_program cubemap_texturing(glsl_shader(GL_VERTEX_SHADER,   "glsl/cubemap_texturing.vs"),
                                   glsl_shader(GL_FRAGMENT_SHADER, "glsl/cubemap_texturing.fs"));
	cubemap_texturing.enable();

	GLint model_matrix_id = cubemap_texturing.uniform_id("model_matrix");
	GLint view_matrix_id = cubemap_texturing.uniform_id("view_matrix");
	GLint projection_matrix_id = cubemap_texturing.uniform_id("projection_matrix");
	GLint time_id = cubemap_texturing.uniform_id("time");

	glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

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

	GLubyte indices[] = { 0,1,3, 0,3,2,
						  2,3,6, 3,7,6,
                          4,1,0, 4,5,1,
                          2,4,0, 2,6,4,
                          1,5,3, 5,7,3,
                          7,5,6, 6,5,4};



	GLuint vao_id, vbo_id, tbo_id, ibo_id;

	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &tbo_id);                                                                                 
	glBindBuffer(GL_ARRAY_BUFFER, tbo_id);                                                                    
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), glm::value_ptr(vertices[0]), GL_STATIC_DRAW);     
	glEnableVertexAttribArray(1);                         
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);                                                    


	glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);     

	glm::mat4 model_matrix_cube[512];

	GLuint index = 0;
	for(int i = 0; i < 8; ++i)
		for(int j = 0; j < 8; ++j)
			for(int k = 0; k < 8; ++k)
				model_matrix_cube[index++] = glm::translate(5.0f * glm::vec3(i - 3.5f, j - 3.5f, k - 3.5f));


	glEnable(GL_DEPTH_TEST);

	glUniformMatrix4fv(model_matrix_id, 512, GL_FALSE, glm::value_ptr(model_matrix_cube[0]));

	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);																    // clear the screen
		
		glUniform1f(time_id, glfwGetTime());
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(view_matrix));
        glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0, 1);

		glfwSwapBuffers(window);																							// swap buffers
		glfwPollEvents();
	}; 

	glfwTerminate();																										// close OpenGL window and terminate GLFW
	return 0;
}