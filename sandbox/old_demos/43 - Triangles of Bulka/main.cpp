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
	// loading cubemap textures
	// ==================================================================================================================================================================================================================

	glActiveTexture(GL_TEXTURE0);
	GLuint tex2d0 = texture::png("res/texture0.png");
	glActiveTexture(GL_TEXTURE1);
	GLuint tex2d1 = texture::png("res/texture1.png");
	glActiveTexture(GL_TEXTURE2);
	GLuint tex2d2 = texture::png("res/texture2.png");
	glActiveTexture(GL_TEXTURE3);
	GLuint tex2d3 = texture::png("res/texture3.png");




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

	const char * forest_files[6] =   {"res/forest_positive_x.png",
							     	  "res/forest_negative_x.png",
									  "res/forest_positive_y.png",
									  "res/forest_negative_y.png",
									  "res/forest_positive_z.png",
									  "res/forest_negative_z.png"};

	glActiveTexture(GL_TEXTURE4);
	GLuint cubemap0 = texture::cubemap_png(galaxy_files);
	glActiveTexture(GL_TEXTURE5);
	GLuint cubemap1 = texture::cubemap_png(moon_files);
	glActiveTexture(GL_TEXTURE6);
	GLuint cubemap2 = texture::cubemap_png(forest_files);
	glActiveTexture(GL_TEXTURE7);
	GLuint cubemap3 = texture::cubemap_png(moon_files);


	// ==================================================================================================================================================================================================================
	// program mifka : uses 2d textures
	// ==================================================================================================================================================================================================================

    glsl_program mifka(glsl_shader(GL_VERTEX_SHADER,   "glsl/mifka.vs"),
                       glsl_shader(GL_FRAGMENT_SHADER, "glsl/mifka.fs"));
	mifka.enable();

	GLint model_matrix_id = mifka.uniform_id("model_matrix");
	GLint view_matrix_id = mifka.uniform_id("view_matrix");
	GLint projection_matrix_id = mifka.uniform_id("projection_matrix");
	GLint time_id = mifka.uniform_id("time");

	glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));



	// ==================================================================================================================================================================================================================
	// program mifunja : uses cubemap textures
	// ==================================================================================================================================================================================================================


    glsl_program mifunja(glsl_shader(GL_VERTEX_SHADER,   "glsl/mifunja.vs"),
                         glsl_shader(GL_FRAGMENT_SHADER, "glsl/mifunja.fs"));
	mifunja.enable();

	GLint model_matrix_id1 = mifka.uniform_id("model_matrix");
	GLint view_matrix_id1 = mifka.uniform_id("view_matrix");
	GLint projection_matrix_id1 = mifka.uniform_id("projection_matrix");
	GLint time_id1 = mifka.uniform_id("time");

	glUniformMatrix4fv(projection_matrix_id1, 1, GL_FALSE, glm::value_ptr(projection_matrix));

	// ==================================================================================================================================================================================================================
	// pyramids
	// ==================================================================================================================================================================================================================

	const float scale = 7.652831f;

	glm::vec3 bulka_triangles [] =
	{
		scale * glm::vec3(-0.3f, -0.3f, -0.3f),	
		scale * glm::vec3( 1.0f,  0.0f,  0.0f),
		scale * glm::vec3( 0.0f,  0.0f,  1.0f),
		                          
		scale * glm::vec3(-0.3f, -0.3f, -0.3f),	
		scale * glm::vec3( 0.0f,  1.0f,  0.0f),
		scale * glm::vec3( 1.0f,  0.0f,  0.0f),
		                          
		scale * glm::vec3(-0.3f, -0.3f, -0.3f),	
		scale * glm::vec3( 0.0f,  0.0f,  1.0f),
		scale * glm::vec3( 0.0f,  1.0f,  0.0f),
		                          
		scale * glm::vec3( 1.0f,  0.0f,  0.0f),
		scale * glm::vec3( 0.0f,  1.0f,  0.0f),
		scale * glm::vec3( 0.0f,  0.0f,  1.0f)

	};

	glm::vec3 bulka_colors [] =
	{
		glm::vec3(0.0f, 0.0f, 0.0f),	
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		                  
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		                  
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		                  
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	};

	glm::vec2 bulka_texcoords [] =
	{
		glm::vec2(0.0f, 0.0f),	
		glm::vec2(1.0f, 0.0f),  
		glm::vec2(1.0f, 1.0f),
		                  
		glm::vec2(0.0f, 0.0f),  
		glm::vec2(0.0f, 1.0f),  
		glm::vec2(1.0f, 0.0f),  
		                  
		glm::vec2(0.0f, 0.0f),  
		glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f),  
		                  
		glm::vec2(1.0f, 0.0f),  
		glm::vec2(0.0f, 1.0f),  
		glm::vec2(1.0f, 1.0f)
	};


	GLuint vao_id, vbo_id, col_id, tbo_id;

	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bulka_triangles), glm::value_ptr(bulka_triangles[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);


	glGenBuffers(1, &col_id);
	glBindBuffer(GL_ARRAY_BUFFER, col_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bulka_colors), glm::value_ptr(bulka_colors[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);


	glGenBuffers(1, &tbo_id);                                                                                 
	glBindBuffer(GL_ARRAY_BUFFER, tbo_id);                                                                    
	glBufferData(GL_ARRAY_BUFFER, sizeof(bulka_texcoords), glm::value_ptr(bulka_texcoords[0]), GL_STATIC_DRAW);     
	glEnableVertexAttribArray(2);                                                                             
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);                                                    

	// ==================================================================================================================================================================================================================
	// cubes
	// ==================================================================================================================================================================================================================

	float cube_scale = 3.471298f;
	
	glm::vec3 bulka_quadrats [] =
	{
		cube_scale * glm::vec3(-1.0f, -1.0f, -1.0f),	
		cube_scale * glm::vec3(-1.0f,  1.0f, -1.0f),
		cube_scale * glm::vec3(-1.0f, -1.0f,  1.0f),
		cube_scale * glm::vec3(-1.0f,  1.0f,  1.0f),	
		cube_scale * glm::vec3( 1.0f, -1.0f, -1.0f),
		cube_scale * glm::vec3( 1.0f,  1.0f, -1.0f),
		cube_scale * glm::vec3( 1.0f, -1.0f,  1.0f),	
		cube_scale * glm::vec3( 1.0f,  1.0f,  1.0f)
	};

	glm::vec3 bulka_colors_of_quadrats [] =
	{
		glm::vec3(0.0f, 1.0f, 0.0f),	
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
	};

	glm::vec3 bulka_texcoords_of_quadrats [] =
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

	GLubyte indices[] = {  0,1,3, 0,3,2,
							2,3,6, 3,7,6,
                            4,1,0, 4,5,1,
                            2,4,0, 2,6,4,
                            1,5,3, 5,7,3,
                            7,5,6, 6,5,4};



	GLuint vao_id1, vbo_id1, col_id1, tbo_id1, ibo_id1;

	glGenVertexArrays(1, &vao_id1);
	glBindVertexArray(vao_id1);

	glGenBuffers(1, &vbo_id1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bulka_quadrats), glm::value_ptr(bulka_quadrats[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);


	glGenBuffers(1, &col_id1);
	glBindBuffer(GL_ARRAY_BUFFER, col_id1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bulka_colors_of_quadrats), glm::value_ptr(bulka_colors_of_quadrats[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);


	glGenBuffers(1, &tbo_id1);                                                                                 
	glBindBuffer(GL_ARRAY_BUFFER, tbo_id1);                                                                    
	glBufferData(GL_ARRAY_BUFFER, sizeof(bulka_texcoords_of_quadrats), glm::value_ptr(bulka_texcoords_of_quadrats[0]), GL_STATIC_DRAW);     
	glEnableVertexAttribArray(2);                                                                             
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);                                                    


	glGenBuffers(1, &ibo_id1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);     



	glm::mat4 model_matrix_pyramids[512];
	glm::mat4 model_matrix_cube[512];

	int index = 0;

	for(int i = 0; i < 8; ++i)
	for(int j = 0; j < 8; ++j)
	for(int k = 0; k < 8; ++k)
		model_matrix_pyramids[index++] = glm::translate(glm::vec3(5.0f) + 12.78f * glm::vec3(i,j,k));

	index = 0;
	for(int i = 0; i < 8; ++i)
	for(int j = 0; j < 8; ++j)
	for(int k = 0; k < 8; ++k)
		model_matrix_cube[index++] = glm::translate(glm::vec3(-5.0f) + -12.78f * glm::vec3(i,j,k));


	glEnable(GL_DEPTH_TEST);

	mifka.enable();
	glUniformMatrix4fv(model_matrix_id, 512, GL_FALSE, glm::value_ptr(model_matrix_pyramids[0]));
	mifunja.enable();
	glUniformMatrix4fv(model_matrix_id1, 512, GL_FALSE, glm::value_ptr(model_matrix_cube[0]));

	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);																    // clear the screen
		
		mifka.enable();
		glUniform1f(time_id, glfwGetTime());
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(view_matrix));
     	glBindVertexArray(vao_id);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 12, 512);

		mifunja.enable();
		glUniform1f(time_id1, glfwGetTime());
		glUniformMatrix4fv(view_matrix_id1, 1, GL_FALSE, glm::value_ptr(view_matrix));
        glBindVertexArray(vao_id1);
        glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0, 512);

		glfwSwapBuffers(window);																							// swap buffers
		glfwPollEvents();
	}; 














	glfwTerminate();																										// close OpenGL window and terminate GLFW
	return 0;
}