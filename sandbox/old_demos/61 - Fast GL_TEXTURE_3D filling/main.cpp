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

const float two_pi = 6.283185307179586476925286766559; 

const unsigned int TEXTURE_SIZE = 256;

const unsigned int res_x = 1920;
const unsigned int res_y = 1080;

int main()
{

	// ==================================================================================================================================================================================================================
	// GLFW library initialization
	// ==================================================================================================================================================================================================================

	if(!glfwInit()) exit_msg("Failed to initialize GLFW.");                                                                 // initialise GLFW

	glfwWindowHint(GLFW_SAMPLES, 4); 																						// 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); 																			// we want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 																	
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 															// request core profile
							
	GLFWwindow* window; 																									// open a window and create its OpenGL context 
	window = glfwCreateWindow(res_x, res_y, "GL_COMPUTE_SHADER", glfwGetPrimaryMonitor(), 0); 
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
	debug_msg("GLEW library initialization done ... ");
    debug_msg("GL_VENDOR = %s.", glGetString(GL_VENDOR));                                       
    debug_msg("GL_RENDERER = %s.", glGetString(GL_RENDERER));                                   
    debug_msg("GL_VERSION = %s.", glGetString(GL_VERSION));                                     
    debug_msg("GL_SHADING_LANGUAGE_VERSION = %s.", glGetString(GL_SHADING_LANGUAGE_VERSION));   
    debug_msg("GL_EXTENSIONS = %s.", glGetString(GL_EXTENSIONS));                               

	// ==================================================================================================================================================================================================================
	// Set up camera and projection matrix
	// ==================================================================================================================================================================================================================
	init_camera(window);

	// ==================================================================================================================================================================================================================
	// Creating shaders and uniforms
	// ==================================================================================================================================================================================================================
    glsl_program isosurface(glsl_shader(GL_VERTEX_SHADER,   "glsl/isosurface.vs"),
                            glsl_shader(GL_FRAGMENT_SHADER, "glsl/isosurface.fs"));

    isosurface.enable();
	GLint isosurface_view_matrix = isosurface.uniform_id("view_matrix");                                            

	// ==================================================================================================================================================================================================================
	// Density compute shader
	// ==================================================================================================================================================================================================================
    glsl_program density_compute(glsl_shader(GL_COMPUTE_SHADER, "glsl/density_compute.cs"));
    density_compute.enable();

	// ==================================================================================================================================================================================================================
	// Create image texture to be used for the GL_COMPUTE_SHADER output
	// ==================================================================================================================================================================================================================
	GLuint density_texture_id;
	glGenTextures(1, &density_texture_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, density_texture_id);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, TEXTURE_SIZE, TEXTURE_SIZE, TEXTURE_SIZE);
	glBindImageTexture(0, density_texture_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);						// Note : GL_TRUE is necessary as GL_TEXTURE_3D is layered




	// ==================================================================================================================================================================================================================
	// Create fake VAO with no attribute buffers, all the input data will come from GL_SHADER_STORAGE_BUFFER
	// ==================================================================================================================================================================================================================
	GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);


	GLuint cycle = 0;
	float initial_time = glfwGetTime();


	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);																    // clear the screen
		float time = glfwGetTime();

		// ==============================================================================================================================================================================================================
		// Compute density texture3D
		// ==============================================================================================================================================================================================================
		density_compute.enable();
		glDispatchCompute(TEXTURE_SIZE >> 5, TEXTURE_SIZE >> 4, TEXTURE_SIZE >> 4);

		// ==============================================================================================================================================================================================================
		// Finally, render the mesh constructed by marching cubes compute shader
		// ==============================================================================================================================================================================================================
		isosurface.enable();
		glUniformMatrix4fv(isosurface_view_matrix, 1, GL_FALSE, glm::value_ptr(view_matrix));
		glDrawArrays(GL_POINTS, 0, TEXTURE_SIZE * TEXTURE_SIZE * TEXTURE_SIZE);

		debug_msg("Computation #%u done. FPS = %f. time = %f.", cycle++, double(cycle) / (time - initial_time), time);

		// ==============================================================================================================================================================================================================
		// Done.
		// ==============================================================================================================================================================================================================
        glfwSwapBuffers(window);																					
		glfwPollEvents();
	}; 
    
	glfwTerminate();																										// close OpenGL window and terminate GLFW
	return 0;
}




















