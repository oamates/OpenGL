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


const float two_pi = 6.283185307179586476925286766559;
const unsigned int res_x = 1920;
const unsigned int res_y = 1080;
const unsigned int POINT_COUNT = 0x20000;



glm::vec2 iterate(const glm::vec2 v)
{
	static const float a = -0.966918, b = 2.879879, c = 0.765145, d = 0.744728;
	return glm::vec2 (sin(v.y * b) + c * sin(v.x * b), sin(v.x * a) + d * sin(v.y * a));
};

glm::vec3 iterate(const glm::vec3 v)
{
	static const float a = -0.966918, b = 2.879879, c = 0.765145, d = 0.744728;


	static const glm::vec3 vX = glm::vec3( 3.7651451f, -3.1652437f,  0.2349375f);
	static const glm::vec3 vY = glm::vec3(-0.3678459f,  1.6240835f, -1.2175415f);
	static const glm::vec3 vZ = glm::vec3( 2.1251451f, -0.1652437f,  2.9124321f);
	static const glm::vec3 angular_velocity = glm::vec3(-0.966918f, 2.879879f, 5.3287621f);

	return vX * glm::sin(v.x * angular_velocity) + vY * glm::sin(v.y * angular_velocity) + vZ * glm::sin(v.y * angular_velocity);
};




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
	window = glfwCreateWindow(res_x, res_y, "Skybox", glfwGetPrimaryMonitor(), 0); 
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

    double startup_time = glfwGetTime();                                                                                    // set the time uniform

	// ==================================================================================================================================================================================================================
	// Skybox rendering shader program
	// ==================================================================================================================================================================================================================
    glsl_program skybox(glsl_shader(GL_VERTEX_SHADER,   "glsl/skybox.vs"),
                        glsl_shader(GL_FRAGMENT_SHADER, "glsl/skybox.fs"));
    skybox.enable();
	GLuint skybox_projection_view_matrix = skybox.uniform_id("projection_view_matrix");
	GLuint skybox_global_time = skybox.uniform_id("global_time");

	// ==================================================================================================================================================================================================================
	// Shader and uniform initialization
	// ==================================================================================================================================================================================================================
    glsl_program point_lighting(glsl_shader(GL_VERTEX_SHADER,   "glsl/point.vs"),
                                glsl_shader(GL_FRAGMENT_SHADER, "glsl/point.fs"));
    point_lighting.enable();
	GLuint uniform_projection_view_matrix = point_lighting.uniform_id("projection_view_matrix");						    // projection_view matrix uniform id
	GLuint uniform_shift = point_lighting.uniform_id("shift");								                                // view matrix uniform id
	GLuint uniform_global_time = point_lighting.uniform_id("global_time");								                    // time uniform id

	// ==================================================================================================================================================================================================================
	// Point data initialization 
	// ==================================================================================================================================================================================================================

	glm::vec3* data = (glm::vec3*) malloc(POINT_COUNT * sizeof(glm::vec3));
    GLuint vao_id, vbo_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// ==================================================================================================================================================================================================================
	// Camera, view_matrix and projection_matrix initialization
	// ==================================================================================================================================================================================================================

	init_camera(window);
    const float two_pi = 6.283185307179586476925286766559;
	glm::mat4 projection_matrix = glm::infinitePerspective	(two_pi / 6, float(res_x) / float(res_y), 0.1f); 		        // projection matrix : 60° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glClearColor(0.01f, 0.0f, 0.08f, 1.0f);																					// dark blue background
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); 																									// accept fragment if it closer to the camera than the former one
	glCullFace(GL_FRONT);

    unsigned int frame = 0;

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);    
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	// ==================================================================================================================================================================================================================
	// Camera, view_matrix and projection_matrix initialization
	// ==================================================================================================================================================================================================================
	const glm::vec3 cube_vertices[] = 
	{
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3( 1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f,  1.0f, -1.0f),
		glm::vec3( 1.0f,  1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f,  1.0f),
		glm::vec3( 1.0f, -1.0f,  1.0f),
		glm::vec3(-1.0f,  1.0f,  1.0f),
		glm::vec3( 1.0f,  1.0f,  1.0f)
	};

	const GLubyte cube_indices[] = 
	{
		0, 2, 3, 0, 3, 1,
		4, 5, 7, 4, 7, 6,
		0, 4, 6, 0, 6, 2,
		1, 3, 7, 1, 7, 5,
		0, 1, 5, 0, 5, 4,
		2, 6, 7, 2, 7, 3
	};   

    GLuint cube_vao_id, cube_vbo_id, cube_ibo_id;

    glGenVertexArrays(1, &cube_vao_id);
    glBindVertexArray(cube_vao_id);

	// ==================================================================================================================================================================================================================
	// fill all the data buffers
	// ==================================================================================================================================================================================================================

    glGenBuffers(1, &cube_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), glm::value_ptr(cube_vertices[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &cube_ibo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

	const char * galaxy_files[6] = {"res/galaxy_positive_x.png",
									"res/galaxy_negative_x.png",
									"res/galaxy_positive_y.png",
									"res/galaxy_negative_y.png",
									"res/galaxy_positive_z.png",
									"res/galaxy_negative_z.png"};

	const char * moon_files[6] =   {"res/moon_positive_x.png",
									"res/moon_negative_x.png",
									"res/moon_positive_y.png",
									"res/moon_negative_y.png",
									"res/moon_positive_z.png",
									"res/moon_negative_z.png"};

	glActiveTexture(GL_TEXTURE0);
	GLuint galaxy_cubemap = texture::cubemap_png(galaxy_files);

	double current_time = startup_time;
	// ==================================================================================================================================================================================================================
	// The main loop
	// ==================================================================================================================================================================================================================

	while(!glfwWindowShouldClose(window))
	{
        glm::mat4 projection_view_matrix = projection_matrix * view_matrix;                                         

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);															

		skybox.enable();
        glUniform1f(skybox_global_time, (float) current_time);
		glUniformMatrix4fv(skybox_projection_view_matrix, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));		
	    glBindVertexArray(cube_vao_id);
	 	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLubyte), GL_UNSIGNED_BYTE, 0);



	    point_lighting.enable();

		float t = current_time * 0.0000003f;
		glm::vec3 v = glm::vec3(cos(t), sin(t), 1.0f);

		glUniformMatrix4fv(uniform_projection_view_matrix, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));		    // 

		for (unsigned int i = 0; i < POINT_COUNT; ++i)
		{
			data[i] = v;
			v = iterate(v);
		};

	    glBindVertexArray(vao_id);
    	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	    glBufferData(GL_ARRAY_BUFFER, POINT_COUNT * sizeof(glm::vec3), data, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_POINTS, 0, POINT_COUNT);        

    
        current_time = glfwGetTime();

        if (((++frame) & 0x3FF) == 0) 
            debug_msg("frame#%u : \nprojection_matrix = %s\nview_matrix = %s\ntime = %f\nfps = %f\n", frame, glm::to_string(projection_matrix).c_str(), glm::to_string(view_matrix).c_str(), current_time, frame / (current_time - startup_time));

        glfwSwapBuffers(window);																					
		glfwPollEvents();
	}; 
    
	glfwTerminate();																								
	return 0;
}