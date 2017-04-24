#include <GL/glew.h> 														                                                // OpenGL extensions
#include <GLFW/glfw3.h>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>

#include "log.hpp"
#include "shader.hpp"
#include "camera.hpp"

const float two_pi = 6.283185307179586476925286766559;
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

    glsl_program program1(glsl_shader(GL_VERTEX_SHADER,   "glsl/1.vs"),
                          glsl_shader(GL_GEOMETRY_SHADER, "glsl/1.gs"),
                          glsl_shader(GL_FRAGMENT_SHADER, "glsl/1.fs"));

    glsl_program acceleration1(glsl_shader(GL_COMPUTE_SHADER, "glsl/1.cs"));
    glsl_program acceleration2(glsl_shader(GL_COMPUTE_SHADER, "glsl/2.cs"));
    glsl_program integrator(glsl_shader(GL_COMPUTE_SHADER, "glsl/integrator.cs"));

	std::cout << "Shaders initialized" << std::endl;

    const int particles = 512;

    // randomly place particles in a cube
    std::vector<glm::vec4> positionData(particles);
    std::vector<glm::vec4> velocityData(particles);
    for(int i = 0; i < particles; ++i) 
	{
        // initial position
        positionData[i] = glm::vec4(glm::sphericalRand(1.0f), 1.0f);
        velocityData[i] = glm::vec4(glm::sphericalRand(1.0f), 0.0f);
    };

    GLuint vao, positions_vbo, velocities_vbo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &positions_vbo);
    glGenBuffers(1, &velocities_vbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocities_vbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4)*particles, &velocityData[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*particles, &positionData[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));

	const GLuint ssbos[] = {positions_vbo, velocities_vbo};
    glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 0, 2, ssbos);

    float dt = 1.0f / 60.0f;

	acceleration2.enable();
    glUniform1f(0, dt);

	acceleration1.enable();
    glUniform1f(0, dt);

    integrator.enable();
    glUniform1f(0, dt);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    GLuint query;
    glGenQueries(1, &query);

    bool tiled = false;
    bool space_down = false;

	init_camera(window);
	glm::mat4 projection_matrix = glm::infinitePerspective	(two_pi / 6, float(res_x) / float(res_y), 0.1f); 		        // projection matrix : 60° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units


    while(!glfwWindowShouldClose(window)) 
	{
        glfwPollEvents();

        // switch force calculation method
        if (glfwGetKey(window, GLFW_KEY_SPACE) && !space_down)  tiled = !tiled;
        space_down = glfwGetKey(window, GLFW_KEY_SPACE);

		// compute accelerations
        glBeginQuery(GL_TIME_ELAPSED, query);
        if (tiled) 
			acceleration2.enable();
		else
			acceleration1.enable();
        glDispatchCompute(particles / 256, 1, 1);
        glEndQuery(GL_TIME_ELAPSED);

	    integrator.enable();
        glDispatchCompute(particles / 256, 1, 1);

        // clear first
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // use the shader program
		program1.enable();

        // set the uniforms
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(view_matrix));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, particles);
/*
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) 
		{
            std::cerr << error << std::endl;
            break;
        }
*/
        glfwSwapBuffers(window);

        GLuint64 result;
        glGetQueryObjectui64v(query, GL_QUERY_RESULT, &result);
        std::cout << result * 1.e-6 << " ms/frame" << std::endl;
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &positions_vbo);
    glDeleteBuffers(1, &velocities_vbo);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
