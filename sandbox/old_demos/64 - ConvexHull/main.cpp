#include <iostream>
#include <random>
#include <stdlib.h>
#define GLEW_STATIC
#include <GL/glew.h> 														                                                // OpenGL extensions
#include <GLFW/glfw3.h>														                                                // windows and event management library
#include <algorithm>

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
#include "hull3d.hpp"

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
	// init shader
	// ==================================================================================================================================================================================================================
    glsl_program hull_shader(glsl_shader(GL_VERTEX_SHADER,   "glsl/convex_hull.vs"),
                       		 glsl_shader(GL_FRAGMENT_SHADER, "glsl/convex_hull.fs"));
	hull_shader.enable();

	GLint model_matrix_id = hull_shader.uniform_id("model_matrix");
	GLint view_matrix_id = hull_shader.uniform_id("view_matrix");
	GLint projection_matrix_id = hull_shader.uniform_id("projection_matrix");

	glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glm::mat4 model_matrix = glm::mat4(1.0f);
	glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, glm::value_ptr(model_matrix));


	// ==================================================================================================================================================================================================================
	// create point cloud
	// ==================================================================================================================================================================================================================

	std::random_device rd;
	std::mt19937 randgen(rd());
	std::normal_distribution<double> gauss_dist;

	const int CLOUD_COUNT = 256;
	const int CLOUD_SIZE = 1024;

	std::vector<R3> points;

	int index = 0;
	for(int i = 0; i < CLOUD_COUNT; ++i)
	{
		double q = gauss_dist(randgen);
		glm::dvec3 center = (25.0 / (1.0 + q * q)) * glm::normalize(glm::dvec3(gauss_dist(randgen), gauss_dist(randgen), gauss_dist(randgen)));
		for(int j = 0; j < CLOUD_SIZE; ++j)
		{
			R3 point;
			point.id = index;
			double q = gauss_dist(randgen);
			point.position = center + 5.0 / (1.0) * glm::normalize(glm::dvec3(gauss_dist(randgen), gauss_dist(randgen), gauss_dist(randgen)));
			points.push_back(point);
			++index;
		};
	};
	
	std::sort(points.begin(), points.end());
	std::vector<Triangle> hull = hull3d(points);
	unsigned int cloud_size = points.size();
	unsigned int hull_size = hull.size();

	// ==================================================================================================================================================================================================================
	// re-index both vertex buffer and triangle buffer
	// ==================================================================================================================================================================================================================
	// 
	unsigned int* pindex = (unsigned int*) malloc (cloud_size * sizeof(unsigned int));
	unsigned int* tindex = (unsigned int*) malloc (hull_size * sizeof(unsigned int));
	for (unsigned int i = 0; i < cloud_size; ++i) pindex[i] = 0;

	unsigned int t = 0;
	for(unsigned int i = 0; i < hull_size; ++i)
		if(hull[i].state > 0)
		{
			tindex[i] = t++;
			pindex[hull[i].vertices.x] = 1;
			pindex[hull[i].vertices.y] = 1;
			pindex[hull[i].vertices.z] = 1;
		}
		else
			tindex[i] = -1;

	unsigned int p = 0;

	for(unsigned int i = 0; i < cloud_size; ++i)
		if (pindex[i]) pindex[i] = p++;

	glm::vec3* vertices = (glm::vec3*) malloc(p * sizeof(glm::vec3));
	glm::vec3* normals = (glm::vec3*) malloc(p * sizeof(glm::vec3));
	glm::ivec3* indices = (glm::ivec3*) calloc(t, sizeof(glm::ivec3));
	

	vertices[0] = points[0].position;
	glm::dvec3 mass_center = points[0].position;
	
	for(unsigned int i = 1; i < cloud_size; ++i)
		if (pindex[i])
		{
			vertices[pindex[i]] = glm::vec3(points[i].position);
			mass_center += points[i].position;
		};

	mass_center /= double(p);

	for(unsigned int i = 0; i < hull_size; ++i)
		if(hull[i].state != DISCARDED)
		{
			int xi = pindex[hull[i].vertices.x];
			int yi = pindex[hull[i].vertices.y];
			int zi = pindex[hull[i].vertices.z];

			glm::dvec3 nd = glm::cross(points[xi].position - points[yi].position, points[xi].position - points[zi].position);
			if (glm::dot(points[xi].position - mass_center, nd) < 0.0)
			{
				int ti = xi;
				xi = yi;
				yi = ti;
				nd = -nd;
			};

			indices[tindex[i]] = glm::ivec3(xi, yi, zi);
			glm::vec3 nt = glm::vec3(nd);
			normals[xi] += nt;
			normals[yi] += nt;
			normals[zi] += nt;
		};

	free(pindex);
	free(tindex);

	for(unsigned int i = 0; i < p; ++i)
		normals[i] = glm::normalize(normals[i]);

	// ==================================================================================================================================================================================================================
	// fill buffers
	// ==================================================================================================================================================================================================================
	GLuint vao_id, vbo_id, nbo_id, ibo_id;

	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, p * sizeof(glm::vec3), glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);


	glGenBuffers(1, &nbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, nbo_id);
	glBufferData(GL_ARRAY_BUFFER, p * sizeof(glm::vec3), glm::value_ptr(normals[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);


	glGenBuffers(1, &ibo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, t * sizeof(glm::ivec3), glm::value_ptr(indices[0]), GL_STATIC_DRAW);


	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// ==================================================================================================================================================================================================================
	// main loop
	// ==================================================================================================================================================================================================================
	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);																    // clear the screen
		
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(view_matrix));
		glDrawElements(GL_TRIANGLES, 3 * t, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);																							// swap buffers
		glfwPollEvents();
	}; 
	glfwTerminate();																										// close OpenGL window and terminate GLFW
	return 0;
}