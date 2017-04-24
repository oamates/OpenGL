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
#include "model.hpp"
#include "solid.hpp"
#include "shadowmap.hpp"

const float two_pi = 6.283185307179586476925286766559; 


const unsigned int TEXTURE_SIZE = 1024;

const unsigned int res_x = 1920;
const unsigned int res_y = 1080;

float func (const glm::vec2& argument)
{
	float x = argument.x;
	float y = argument.y;
	return 1.5f * glm::sin(5.0f * x) + 
		   1.4f * glm::sin(7.0f * y) +
		   0.3f * glm::sin(21.0f * x + 22.0f * y) +
		   0.1f * glm::sin(30.0f * x - 17.0f * y) +
		   0.05f * glm::sin(73.1235f * x + 53.73457f * y) +
		   0.02f * glm::sin(42.0234f * x - 63.90423f * y);
};


glm::vec3 toral_func (const glm::vec2& v)
{

	float x = v.x;
	float y = v.y;
	const double R = 3.5;
	const double r = 1.6;
	double cos_2piu = cos(two_pi * x);
	double sin_2piu = sin(two_pi * x);
	double cos_2piv = cos(two_pi * y);
	double sin_2piv = sin(two_pi * y);

	glm::vec3 q = glm::vec3((R + r * cos_2piv) * cos_2piu, (R + r * cos_2piv) * sin_2piu, r * sin_2piv);		
	glm::vec3 n = glm::vec3(cos_2piv * cos_2piu, cos_2piv * sin_2piu, sin_2piv);		
	

	return q + 0.14500f * glm::perlin(2.0f * q) * n +
               0.05989f * glm::vec3(glm::perlin(4.0f * q), glm::perlin(4.4f * q), glm::perlin(4.7f * q)) +
			   0.02712f * glm::vec3(glm::perlin(8.0f * q), glm::perlin(9.0f * q), glm::perlin(11.0f * q)) +
			   0.01045f * glm::vec3(glm::perlin(16.0f * q), glm::perlin(15.0f * q), glm::perlin(17.0f * q)) +
			   0.00645f * glm::vec3(glm::perlin(32.0f * q), glm::perlin(31.0f * q), glm::perlin(33.0f * q));

};




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
	float fov = two_pi / 6.0f;
	float aspect_ratio = float(res_x) / float(res_y);
	float znear = 1.0f;
	glm::vec2 scale_xy = -glm::vec2(glm::tan(fov / 2.0f));
	scale_xy.x *= aspect_ratio;
	glm::mat4 projection_matrix = glm::infinitePerspective (fov, aspect_ratio, znear); 		        						// projection matrix : 60° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units

	// ==================================================================================================================================================================================================================
	// shader program for geometry pass
	// ==================================================================================================================================================================================================================
    glsl_program geometry(glsl_shader(GL_VERTEX_SHADER,   "glsl/geometry_pass.vs"),
                          glsl_shader(GL_FRAGMENT_SHADER, "glsl/geometry_pass.fs"));
	geometry.enable();
	GLint geometry_pass_mvp_matrix = geometry.uniform_id("mvp_matrix");

	// ==================================================================================================================================================================================================================
	// shader program for screen-space ambient occlusion pass
	// ==================================================================================================================================================================================================================
    glsl_program ssao(glsl_shader(GL_VERTEX_SHADER,   "glsl/ssao.vs"),
                      glsl_shader(GL_FRAGMENT_SHADER, "glsl/ssao.fs"));
	ssao.enable();
	glUniform2fv(ssao.uniform_id("scale_xy"), 1, glm::value_ptr(scale_xy));
	glUniformMatrix4fv(ssao.uniform_id("projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));

	const unsigned int MAX_KERNEL_SIZE = 256;
	glm::vec3 rnd[MAX_KERNEL_SIZE];
	GLint ssao_spherical_rand = ssao.uniform_id("spherical_rand");
	for (unsigned int i = 0; i < MAX_KERNEL_SIZE; ++i)
	{
		rnd[i] = glm::sphericalRand(1.0f);
		debug_msg("spherical_rand[%d] = %s", i, glm::to_string(rnd[i]).c_str());
	};
	glUniform3fv(ssao_spherical_rand, MAX_KERNEL_SIZE, glm::value_ptr(rnd[0]));

	


	// ==================================================================================================================================================================================================================
	// shader program for ssao image blur 
	// ==================================================================================================================================================================================================================
    glsl_program blur(glsl_shader(GL_VERTEX_SHADER,   "glsl/blur.vs"),
                      glsl_shader(GL_FRAGMENT_SHADER, "glsl/blur.fs"));
	blur.enable();

	// ==================================================================================================================================================================================================================
	// lighting shader program 
	// ==================================================================================================================================================================================================================
    glsl_program simple_light(glsl_shader(GL_VERTEX_SHADER,   "glsl/simple_light.vs"),
                              glsl_shader(GL_FRAGMENT_SHADER, "glsl/simple_light.fs"));

    simple_light.enable();

	GLint simple_light_time = simple_light.uniform_id("time");                                            
	GLint simple_light_view_matrix = simple_light.uniform_id("view_matrix");                                            
	GLint simple_light_model_matrix = simple_light.uniform_id("model_matrix");
	GLint simple_light_light_position = simple_light.uniform_id("light_position");
	glUniformMatrix4fv(simple_light.uniform_id("projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));


	// ==================================================================================================================================================================================================================
	// creating landscape mesh
	// ==================================================================================================================================================================================================================

	glm::mat4 model_matrix = glm::scale(glm::vec3(100.0f, 100.0f, 100.0f));

	toral_surface cave;
    cave.generate_vao(toral_func, glm::ivec2(512, 400));
    debug_msg("Cave generated");


	depth_map zbuffer(res_x, res_y);
	color_map ssao_buffer(res_x, res_y);
	color_map blur_buffer(res_x, res_y);

	// ==================================================================================================================================================================================================================
	// generate quad VAO
	// ==================================================================================================================================================================================================================
	GLuint quad_vao_id, quad_vbo_id;
	
	glm::vec2 quad_data [] = 
	{
		glm::vec2(-1.0f, -1.0f),
		glm::vec2( 1.0f, -1.0f),
		glm::vec2( 1.0f,  1.0f),
		glm::vec2(-1.0f,  1.0f)
	};

	glGenVertexArrays(1, &quad_vao_id);
	glBindVertexArray(quad_vao_id);

	glGenBuffers(1, &quad_vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), glm::value_ptr(quad_data[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	debug_msg("projection_matrix = %s", glm::to_string(projection_matrix).c_str());



	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(-1);

	glm::mat3 orientation = glm::mat3(1.0f);


	while(!glfwWindowShouldClose(window))
	{
		float t = glfwGetTime();

		orientation[2] += 0.2 * glm::vec3(sin(t/3.5), cos(t/3.5), 0.3);
		orientation[2] = glm::normalize(orientation[2]);
		orientation[1] -= glm::dot(orientation[1], orientation[2]) * orientation[2]; 
		orientation[1] = glm::normalize(orientation[1]);
		orientation[0] = glm::cross(orientation[1], orientation[2]);
		orientation[0] = glm::normalize(orientation[0]);

		glm::mat4 v = glm::translate(glm::mat4(orientation), glm::vec3(320.0f * cos(t / 3.11f), 320.0f * sin(t / 3.11f), 0.0f));
		view_matrix = v;

		glm::mat4 mvp_matrix;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);																					// dark blue background

		// geometry pass
		geometry.enable();
		zbuffer.bind();
	
		glClear(GL_DEPTH_BUFFER_BIT);	

		mvp_matrix = projection_matrix * view_matrix * model_matrix;
		glUniformMatrix4fv(geometry_pass_mvp_matrix, 1, GL_FALSE, glm::value_ptr(mvp_matrix));
		cave.render();

		glBindVertexArray(quad_vao_id);

		// ssao pass
    	ssao.enable();
		ssao_buffer.bind();
		glClear(GL_COLOR_BUFFER_BIT);
		zbuffer.bind_texture(GL_TEXTURE0);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    	// blur pass
    	blur.enable();
		blur_buffer.bind();
		glClear(GL_COLOR_BUFFER_BIT);
        ssao_buffer.bind_texture(GL_TEXTURE0);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // lighting pass
		
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	    glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.01f, 0.0f, 0.05f, 0.0f);																					// dark blue background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    	simple_light.enable();
        blur_buffer.bind_texture(GL_TEXTURE0);

		glUniform1f(simple_light_time, t);


		float x = t / 15.0;
		float y = 5 * x;
		const double R = 3.5;
		const double r = 0.9;
		double cos_2piu = cos(two_pi * x);
		double sin_2piu = sin(two_pi * x);
		double cos_2piv = cos(two_pi * y);
		double sin_2piv = sin(two_pi * y);

		glm::vec3 q = glm::vec3((R + r * cos_2piv) * cos_2piu, (R + r * cos_2piv) * sin_2piu, r * sin_2piv);		


		glm::vec4 light_position = glm::vec4(200.0f * cos(t), 200.0f * sin(t), 0.0f, 1.0f);

		glUniform4fv(simple_light_light_position, 1, glm::value_ptr(light_position));

		glUniformMatrix4fv(simple_light_view_matrix, 1, GL_FALSE, glm::value_ptr(view_matrix));
		glUniformMatrix4fv(simple_light_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix));
		cave.render();

        glfwSwapBuffers(window);																					
		glfwPollEvents();
	}; 
    
	glfwTerminate();																										// close OpenGL window and terminate GLFW
	return 0;
};