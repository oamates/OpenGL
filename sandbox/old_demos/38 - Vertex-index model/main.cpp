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

const float two_pi = 6.283185307179586476925286766559; 


const unsigned int TEXTURE_SIZE = 1024;

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
	// Loading models
	// ==================================================================================================================================================================================================================
	model demon;

	demon.load_vi("res/demon.obj");
	// ==================================================================================================================================================================================================================
	// Creating shaders and uniforms
	// ==================================================================================================================================================================================================================

    glsl_program geometry_pass(glsl_shader(GL_VERTEX_SHADER,   "glsl/geometry_pass.vs"),
                               glsl_shader(GL_FRAGMENT_SHADER, "glsl/geometry_pass.fs"));
	
	geometry_pass.enable();
	GLint gpass_mvp_matrix = simple_light.uniform_id("mvp_matrix");											// projection matrix uniform id



    glsl_program simple_light(glsl_shader(GL_VERTEX_SHADER,   "glsl/simple_light.vs"),
                              glsl_shader(GL_FRAGMENT_SHADER, "glsl/simple_light.fs"));

    simple_light.enable();

	GLint uniform_projection_matrix = simple_light.uniform_id("projection_matrix");											// projection matrix uniform id
	GLint uniform_view_matrix       = simple_light.uniform_id("view_matrix");                                               // 
	GLint uniform_model_matrix      = simple_light.uniform_id("model_matrix");                                               // 

	init_camera(window);
	glClearColor(0.01f, 0.0f, 0.05f, 0.0f);																					// dark blue background
	glm::mat4 projection_matrix = glm::infinitePerspective	(two_pi / 6, float(res_x) / float(res_y), 0.1f); 		        // projection matrix : 60° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glUniformMatrix4fv(uniform_projection_matrix, 1, GL_FALSE, glm::value_ptr(projection_matrix));

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glm::mat4 demon_model_matrix = glm::translate(glm::vec3(0.0f, 0.0f, -5.0f)) * glm::scale(glm::vec3(10.2f, 10.2f, 10.2f));

	while(!glfwWindowShouldClose(window))
	{
	    glm::mat4 mvp_matrix = projection_matrix * view_matrix;
		// geometry pass

		geometry_pass.enable();
		glUniformMatrix4fv(gpass_mvp_matrix, 1, GL_FALSE, glm::value_ptr(mvp_matrix));
		demon.render();

		glUniformMatrix4fv
		m_geomPassTech.Enable();        
        m_depthBuffer.BindForWriting();
        glClear(GL_DEPTH_BUFFER_BIT);
        m_pipeline.Orient(m_mesh.GetOrientation());
        m_geomPassTech.SetWVP(m_pipeline.GetWVPTrans());
        m_mesh.Render();       


		// ssao pass
        
        m_SSAOTech.Enable();        
        m_SSAOTech.BindDepthBuffer(m_depthBuffer);        
        m_aoBuffer.BindForWriting();
        glClear(GL_COLOR_BUFFER_BIT);                
        m_quad.Render();                


    	// blur pass
        m_blurTech.Enable();
        m_blurTech.BindInputBuffer(m_aoBuffer);
        m_blurBuffer.BindForWriting();
        glClear(GL_COLOR_BUFFER_BIT);                
        m_quad.Render();                

        
        // lighting pass
        m_lightingTech.Enable();
        m_lightingTech.SetShaderType(m_shaderType);                
        m_lightingTech.BindAOBuffer(m_blurBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_pipeline.Orient(m_mesh.GetOrientation());
        m_lightingTech.SetWVP(m_pipeline.GetWVPTrans());        
        m_lightingTech.SetWorldMatrix(m_pipeline.GetWorldTrans());        
        m_mesh.Render();               



		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);																    // clear the screen

		glUniformMatrix4fv(uniform_view_matrix, 1, GL_FALSE, glm::value_ptr(view_matrix));

		glUniformMatrix4fv(uniform_model_matrix, 1, GL_FALSE, glm::value_ptr(demon_model_matrix));
		demon.render();

        glfwSwapBuffers(window);																					
		glfwPollEvents();
	}; 
    
	glfwTerminate();																										// close OpenGL window and terminate GLFW
	return 0;
};