#include <cstdlib>
#include <iostream>
#include <map>
#include <random>

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
#include "plato.hpp"

const unsigned int res_x = 1920;
const unsigned int res_y = 1080;

struct spherical_mesh
{
	GLuint vao_id, vbo_id, hbo_id, nbo_id, ibo_id;
	GLuint triangles;

	spherical_mesh() {};
	~spherical_mesh()
	{
		glDeleteBuffers(1, &vbo_id);																				
		glDeleteBuffers(1, &hbo_id);
		glDeleteBuffers(1, &nbo_id);
		glDeleteBuffers(1, &ibo_id);
		glDeleteVertexArrays(1, &vao_id);
	};

	void generate_vao(GLuint level)
	{
		GLuint deg4 = 1 << (2 * level);
		GLuint V = deg4 * (plato::icosahedron::V - 2) + 2;
		GLuint E = deg4 * (plato::icosahedron::E);
		GLuint F = deg4 * (plato::icosahedron::F);
        triangles = F;
        
		debug_msg("V = %u. E = %u. F = %u.", V, E, F);
        
		GLuint VBO_SIZE = V * sizeof(glm::vec3);
		GLuint IBO_SIZE = F * sizeof(glm::ivec3);
		GLuint v, f;
        
		debug_msg("VBO_SIZE = %u. IBO_SIZE = %u.", VBO_SIZE, IBO_SIZE);
        
		glm::vec3* vertices = (glm::vec3*) malloc(VBO_SIZE);
		for(v = 0; v < plato::icosahedron::V; ++v) vertices[v] = glm::normalize(plato::icosahedron::vertex[v]);
	
		glm::ivec3* indices = (glm::ivec3*) malloc(IBO_SIZE);
		for(f = 0; f < plato::icosahedron::F; ++f) indices[f] = plato::icosahedron::triangle[f];
        
		// ==============================================================================================================================================================================================================
		// vertices and texture coordinates
		// ==============================================================================================================================================================================================================

		for (GLuint l = 0; l < level; ++l)
		{
			GLuint end = f;
			std::map<uvec2_lex, GLuint> center_index;
			for (GLuint triangle = 0; triangle < end; ++triangle)
			{
            	GLuint P = indices[triangle].x;
            	GLuint Q = indices[triangle].y;
            	GLuint R = indices[triangle].z;
				GLuint S, T, U;
        
				uvec2_lex PQ = (P < Q) ? uvec2_lex(P, Q) : uvec2_lex(Q, P);
				uvec2_lex QR = (Q < R) ? uvec2_lex(Q, R) : uvec2_lex(R, Q);
				uvec2_lex RP = (R < P) ? uvec2_lex(R, P) : uvec2_lex(P, R);
        
				std::map<uvec2_lex, GLuint>::iterator it = center_index.find(PQ); 
				if (it != center_index.end()) S = it->second;
				else
				{
					S = v++;
					center_index[PQ] = S;
					vertices[S] = glm::normalize(vertices[P] + vertices[Q]);
				};
				it = center_index.find(QR); 
				if (it != center_index.end()) T = it->second;
				else
				{
					T = v++;
					center_index[QR] = T;
					vertices[T] = glm::normalize(vertices[Q] + vertices[R]);
				};
				it = center_index.find(RP); 
				if (it != center_index.end()) U = it->second;
				else
				{
					U = v++;
					center_index[RP] = U;
					vertices[U] = glm::normalize(vertices[R] + vertices[P]);
				};
        
                indices[triangle] = glm::ivec3(S, T, U);
                indices[f++]      = glm::ivec3(P, S, U);
                indices[f++]      = glm::ivec3(Q, T, S);
                indices[f++]      = glm::ivec3(R, U, T);
			};
		};
        
		// ==============================================================================================================================================================================================================
		// vertices and texture coordinates
		// ==============================================================================================================================================================================================================        
        
		glGenVertexArrays(1, &vao_id);
		glBindVertexArray(vao_id);
        
		glGenBuffers(1, &vbo_id);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
		glBufferData(GL_ARRAY_BUFFER, VBO_SIZE, glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        
		glGenBuffers(1, &ibo_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IBO_SIZE, glm::value_ptr(indices[0]), GL_STATIC_DRAW);
        
		free(vertices);
		free(indices);

	};

	void render()
	{
		glBindVertexArray(vao_id);
		glDrawElements(GL_TRIANGLES, 3 * triangles, GL_UNSIGNED_INT, 0);
	};

  private:	

	struct uvec2_lex : public glm::uvec2
	{
		uvec2_lex(GLuint a, GLuint b) : glm::uvec2(a, b) {};

		friend bool operator < (const uvec2_lex a, const uvec2_lex b)
		{
			if (a.y < b.y) return true;
			if (a.y > b.y) return false;
			if (a.x < b.x) return true;
			return false;
		};
	};

};

//=======================================================================================================================================================================================================================
// Program entry point
//=======================================================================================================================================================================================================================

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
	// Loading shader
	// ==================================================================================================================================================================================================================
    glsl_program boiling(glsl_shader(GL_VERTEX_SHADER,   "glsl/boiling.vs"),
                         glsl_shader(GL_FRAGMENT_SHADER, "glsl/boiling.fs"));
	boiling.enable();

	GLint time_id = boiling.uniform_id("time");
	GLint view_matrix_id = boiling.uniform_id("view_matrix");
	GLint projection_matrix_id = boiling.uniform_id("projection_matrix");

	glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

	// ==================================================================================================================================================================================================================
	// planet
	// ==================================================================================================================================================================================================================

	spherical_mesh boiling_sphere;
    boiling_sphere.generate_vao(7);
    debug_msg("Spherical mesh generated.");

	// ==================================================================================================================================================================================================================
	// load snow texture
	// ==================================================================================================================================================================================================================

	glEnable(GL_DEPTH_TEST);

	while(!glfwWindowShouldClose(window))
	{
		float t = glfwGetTime();
		glUniform1f(time_id, t);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);																    // clear the screen

		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(view_matrix));
		boiling_sphere.render();

		glfwSwapBuffers(window);																							// swap buffers
		glfwPollEvents();
	}; 

	glfwTerminate();																										// close OpenGL window and terminate GLFW
	return 0;
}