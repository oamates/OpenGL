#include <iostream>
#include <random>
#include <stdlib.h>
#define GLEW_STATIC
#include <GL/glew.h> 														                                                // OpenGL extensions
#include <GLFW/glfw3.h>														                                                // windows and event management library

//#define GLM_FORCE_SSE2 

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
#include "isosurface.hpp"
#include "shadowmap.hpp"

const float two_pi = 6.283185307179586476925286766559; 


const unsigned int TEXTURE_SIZE = 1024;

const unsigned int res_x = 1920;
const unsigned int res_y = 1080;

/*
float cave_func (const glm::vec3& v)
{
    static float factor = 0.5f;
	static const glm::vec3 points[8] = 
	{
		factor * glm::vec3(-1.0f, -1.0f, -1.0f),
		factor * glm::vec3( 1.0f, -1.0f, -1.0f),
		factor * glm::vec3(-1.0f,  1.0f, -1.0f),
		factor * glm::vec3( 1.0f,  1.0f, -1.0f),
		factor * glm::vec3(-1.0f, -1.0f,  1.0f),
		factor * glm::vec3( 1.0f, -1.0f,  1.0f),
		factor * glm::vec3(-1.0f,  1.0f,  1.0f),
		factor * glm::vec3( 1.0f,  1.0f,  1.0f)
	};
	float value = 1.5f;

	for (int i = 0; i < 8; ++i)
		value *= glm::length2(v - points[i]);

	return value + 0.1*glm::perlin(10.0f*v);
};
*/

/*
float cave_func (const glm::vec3& v)
{
	const glm::vec3 octahedron[] =
	{
		glm::vec3 ( 0.5f,  0.0f,  0.0f),
		glm::vec3 (-0.5f,  0.0f,  0.0f),
		glm::vec3 ( 0.0f,  0.5f,  0.0f),
		glm::vec3 ( 0.0f, -0.5f,  0.0f),
		glm::vec3 ( 0.0f,  0.0f,  0.5f),
		glm::vec3 ( 0.0f,  0.0f, -0.5f),
	};

	return 2000 * glm::length2(v - octahedron[0]) * 
                    glm::length2(v - octahedron[1]) *
                    glm::length2(v - octahedron[2]) *
                    glm::length2(v - octahedron[3]) *
                    glm::length2(v - octahedron[4]) *
                    glm::length2(v - octahedron[5]);
};
*/
/*
float cave_func (const glm::vec3& v)
{

	const glm::vec3 normal[12] = 
	{
		glm::vec3(0.0f, -0.525731, -0.850651),
		glm::vec3(0.0f, 0.525731, -0.850651),
		glm::vec3(0.0f, 0.525731, 0.850651),
		glm::vec3(0.0f, -0.525731, 0.850651),
		glm::vec3(-0.850651, 0.0f, -0.525731),
		glm::vec3(-0.850651, 0.0f, 0.525731),
		glm::vec3(0.850651, 0.0f, 0.525731),
		glm::vec3(0.850651, 0.0f, -0.525731),
		glm::vec3(-0.525731, -0.850651, 0.0f),
		glm::vec3(0.525731, -0.850651,  0.0f),
		glm::vec3(0.525731, 0.850651,   0.0f),
		glm::vec3(-0.525731, 0.850651,  0.0f)
	};

	float value = 0.1f;
	for (int i = 0; i < 12; ++i)
		value *= glm::length2(v - normal[i]);

	return value;
};
*/

/*
float cave_func (const glm::vec3& v)
{
	float x = v.x;
	float y = v.y;
	float z = v.z;

	float p = (x + 0.2) * (x + 0.2) + y * y - 0.03f;

	float q = (y - 0.2) * (y - 0.2) + z * z - 0.03f;
	return 117.0f * (p * p + z * z) * (q * q + x * x);
};
*/

/*
float cave_func(const glm::vec3& v)
{
	const float q = 0.5f;
	const glm::vec3 source_point[12] = 
	{
		q * glm::vec3(0.0f, -0.525731, -0.850651),
		q * glm::vec3(0.0f, 0.525731, -0.850651),
		q * glm::vec3(0.0f, 0.525731, 0.850651),
		q * glm::vec3(0.0f, -0.525731, 0.850651),
		q * glm::vec3(-0.850651, 0.0f, -0.525731),
		q * glm::vec3(-0.850651, 0.0f, 0.525731),
		q * glm::vec3(0.850651, 0.0f, 0.525731),
		q * glm::vec3(0.850651, 0.0f, -0.525731),
		q * glm::vec3(-0.525731, -0.850651, 0.0f),
		q * glm::vec3(0.525731, -0.850651,  0.0f),
		q * glm::vec3(0.525731, 0.850651,   0.0f),
		q * glm::vec3(-0.525731, 0.850651,  0.0f)
	};

	float value = 0.0f;
	for (int i = 0; i < 12; ++i)
		value += 1.0f / glm::length2(v - source_point[i]);

	return 0.0187 * value;

};
*/  
/*
float cave_func(const glm::vec3& v)
{
	const float q = 0.5f;
	const glm::vec3 source_point[12] = 
	{
		q * glm::vec3(0.0f, -0.525731, -0.850651),
		q * glm::vec3(0.0f, 0.525731, -0.850651),
		q * glm::vec3(0.0f, 0.525731, 0.850651),
		q * glm::vec3(0.0f, -0.525731, 0.850651),
		q * glm::vec3(-0.850651, 0.0f, -0.525731),
		q * glm::vec3(-0.850651, 0.0f, 0.525731),
		q * glm::vec3(0.850651, 0.0f, 0.525731),
		q * glm::vec3(0.850651, 0.0f, -0.525731),
		q * glm::vec3(-0.525731, -0.850651, 0.0f),
		q * glm::vec3(0.525731, -0.850651,  0.0f),
		q * glm::vec3(0.525731, 0.850651,   0.0f),
		q * glm::vec3(-0.525731, 0.850651,  0.0f)
	};

	float value = 0.0f;
	for (int i = 0; i < 12; ++i)
		value += 1.0f / glm::length(v - source_point[i]);

	return 0.0418 * value;

};
  
*/

float cave_func(const glm::vec3& v)
{
	return 
		glm::length(v) + 0.10000f * glm::simplex( 5.0f * v)
		               + 0.05000f * glm::simplex(10.0f * v);
             		   + 0.02500f * glm::simplex(20.0f * v);
		               + 0.01250f * glm::simplex(40.0f * v);
					   + 0.00625f * glm::simplex(80.0f * v);

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

	glm::mat4 model_matrix = glm::scale(glm::vec3(300.0f, 300.0f, 300.0f));

	isosurface cave;
    cave.generate_vao(cave_func, 0.7f);
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

	while(!glfwWindowShouldClose(window))
	{
		float t = glfwGetTime();

		glm::mat4 mvp_matrix;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
//		glDisable(GL_CULL_FACE);
//		glEnable(GL_CULL_FACE);
//		glCullFace(GL_BACK);
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
//		glDisable(GL_CULL_FACE);
//		glEnable(GL_CULL_FACE);
//		glCullFace(GL_BACK);
	    glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.01f, 0.0f, 0.05f, 0.0f);																					// dark blue background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    	simple_light.enable();
        blur_buffer.bind_texture(GL_TEXTURE0);

		glUniform1f(simple_light_time, t);

		glm::vec4 light_position = glm::vec4(400.0f * cos(t), 400.0f * sin(t), 0.0f, 1.0f);
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