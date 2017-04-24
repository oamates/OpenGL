#include <iostream>
#include <random>
#include <map>
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


const unsigned int res_x = 1536;
const unsigned int res_y = 1024;

const float two_pi = 6.283185307179586476925286766559; 


#include <cstdio>
#include <random>

struct Point
{
	double x, y, z;
	Point *prev, *next;
	void act()
	{
		if (prev->next != this)							// insert
			prev->next = next->prev = this;
		else											// delete
		{
			prev->next = next;
			next->prev = prev;
		} 
	}
};

const double INF = 1e20;
static Point nil = {INF, INF, INF, 0, 0};
Point *NIL = &nil;

inline double turn(Point *p, Point *q, Point *r)		// <0 iff cw
{ 
	if (p == NIL || q == NIL || r == NIL) return 1.0;
	return (q->x - p->x) * (r->y - p->y) - (r->x - p->x) * (q->y - p->y);
};

inline double time(Point *p, Point *q, Point *r)		// when turn changes
{ 
	if (p == NIL || q == NIL || r == NIL) return INF;
	return ((q->x - p->x)*(r->z - p->z) - (r->x - p->x)*(q->z - p->z)) / turn(p, q, r);
};

Point* sort(Point P[], int n)							// mergesort
{ 
	Point head;
	if (n == 1)
	{
		P[0].next = NIL; 
		return P;
	};
	Point* a = sort(P, n/2);
	Point* b = sort(P + n/2, n - n/2);
	Point* c = &head;
	do
		if (a->x < b->x)
		{
			c = c->next = a;
			a = a->next;
		}
		else
		{
			c = c->next = b;
			b = b->next;
		}
	while (c != NIL);
	return head.next;
}

void hull(Point *list, int n, Point **A, Point **B)				// the algorithm
{ 
	Point *u, *v, *mid;
	double t[6], oldt, newt;
	int i, j, k, l, minl;


	if (n == 1)
	{
		A[0] = list->prev = list->next = NIL; 
		return;
	};

	for (u = list, i = 0; i < n/2 - 1; u = u->next, i++); mid = v = u->next;
	hull(list, n/2, B, A);																// recurse on left and right sides
	hull(mid, n - n/2, B + (n / 2) * 2, A + (n / 2) * 2);

	while(1)													// find initial bridge
		if (turn(u, v, v->next) < 0) v = v->next;
		else if (turn(u->prev, u, v) < 0) u = u->prev;
		else break;


	for (i = k = 0, j = (n / 2) * 2, oldt = -INF; ; oldt = newt) 						// merge by tracking bridge uv over time
	{
		t[0] = time(B[i]->prev, B[i], B[i]->next);
		t[1] = time(B[j]->prev, B[j], B[j]->next);
		t[2] = time(u, u->next, v);
		t[3] = time(u->prev, u, v);
		t[4] = time(u, v->prev, v);
		t[5] = time(u, v, v->next);
		for (newt = INF, l = 0; l < 6; l++)
			if (t[l] > oldt && t[l] < newt) { minl = l; newt = t[l]; }
		if (newt == INF) break;
		switch (minl)
		{
			case 0: if (B[i]->x < u->x) A[k++] = B[i]; B[i++]->act(); break;
			case 1: if (B[j]->x > v->x) A[k++] = B[j]; B[j++]->act(); break;
			case 2: A[k++] = u = u->next; break;
			case 3: A[k++] = u; u = u->prev; break;
			case 4: A[k++] = v = v->prev; break;
			case 5: A[k++] = v; v = v->next; break;
		};
	};
	A[k] = NIL;

	u->next = v; v->prev = u; // now go back in time to update pointers
	for (k--; k >= 0; k--)
	if (A[k]->x <= u->x || A[k]->x >= v->x)
	{
		A[k]->act();
		if (A[k] == u) u = u->prev; else if (A[k] == v) v = v->next;
 	}
	else
	{
		u->next = A[k]; A[k]->prev = u; v->prev = A[k]; A[k]->next = v;
		if (A[k]->x < mid->x) u = A[k]; else v = A[k];
	};
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
	window = glfwCreateWindow(res_x, res_y, "Empty glfw application", 0, 0); 
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
	// Creating shaders and uniforms
	// ==================================================================================================================================================================================================================

    glsl_program chull(glsl_shader(GL_VERTEX_SHADER,   "glsl/chull.vs"),
                       glsl_shader(GL_FRAGMENT_SHADER, "glsl/chull.fs"));

    chull.enable();

	GLuint uniform_projection_matrix = chull.uniform_id("projection_matrix");						                // projection_view matrix uniform id
	GLuint uniform_view_matrix = chull.uniform_id("view_matrix");						                            // projection_view matrix uniform id



	// ==================================================================================================================================================================================================================
	// Camera, view_matrix and projection_matrix initialization
	// ==================================================================================================================================================================================================================

	init_camera(window);
	glm::mat4 projection_matrix = glm::infinitePerspective	(two_pi / 6, float(res_x) / float(res_y), 0.1f); 		        // projection matrix : 60° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);																					// dark blue background
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); 																									// accept fragment if it closer to the camera than the former one
	glEnable(GL_CULL_FACE);

    unsigned int frame = 0;
    double startup_time = glfwGetTime();                                                                                    // set the time uniform

	// ==================================================================================================================================================================================================================
	// The main loop
	// ==================================================================================================================================================================================================================
    debug_msg("frame#%u : \nprojection_matrix = %s\nview_matrix = %s\ntime = %f\nfps = %f", frame, glm::to_string(projection_matrix).c_str(), glm::to_string(view_matrix).c_str(), startup_time, 0.0);


	GLuint vao_id, vbo_id, ibo_id;

	std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

	const int n = 16;
	Point P[n];

	for (int i = 0; i < n; i++)
	{
		P[i].x = dist(mt); 
		P[i].y = dist(mt); 
		P[i].z = dist(mt);

	};

	Point *list = sort(P, n);
	Point **A = new Point *[2 * n], **B = new Point *[2 * n];
	hull(list, n, A, B);

	unsigned int triangles = 0;

	std::vector<glm::vec3> vertices;
	std::vector<glm::ivec3> indices;

	std::map<GLuint, GLuint> index_map;
	GLuint index = 0;

	for (int i = 0; A[i] != NIL; A[i++]->act())
	{
		glm::ivec3 triangle;
	    GLuint indexA = A[i]->prev - P;
		GLuint indexB = A[i] - P;
		GLuint indexC = A[i]->next - P;

		std::map<GLuint, GLuint>::iterator it = index_map.find(indexA);
		if (it != index_map.end()) triangle.x = it->second;
		else
		{
			triangle.x = index++;
			index_map[indexA] = triangle.x;
			vertices.push_back(glm::vec3(P[indexA].x, P[indexA].y, P[indexA].z));
			
		};

        it = index_map.find(indexB);
		if (it != index_map.end()) triangle.y = it->second;
		else
		{
			triangle.y = index++;
			index_map[indexB] = triangle.y;
			vertices.push_back(glm::vec3(P[indexB].x, P[indexB].y, P[indexB].z));
			
		};

        it = index_map.find(indexC);
		if (it != index_map.end()) triangle.z = it->second;
		else
		{
			triangle.z = index++;
			index_map[indexC] = triangle.z;
			vertices.push_back(glm::vec3(P[indexC].x, P[indexC].y, P[indexC].z));
			
		};

		indices.push_back(triangle);		
	};

	delete A; 
	delete B; 

	//===============================================================================================================================================================================================================
	// create VAO
	//===============================================================================================================================================================================================================
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &ibo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3) * indices.size(), glm::value_ptr(indices[0]), GL_STATIC_DRAW);

	debug_msg("Hull constructed : triangles = %u", indices.size());
	for (unsigned int i = 0; i < indices.size(); ++i)
	{
		debug_msg("Triangle[%u] = %s", i, glm::to_string(indices[i]).c_str());
	};

	glDisable(GL_CULL_FACE);

	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);																    

		glUniformMatrix4fv(uniform_projection_matrix, 1, GL_FALSE, glm::value_ptr(projection_matrix));		                
		glUniformMatrix4fv(uniform_view_matrix, 1, GL_FALSE, glm::value_ptr(view_matrix));		                            


        glDrawElements(GL_TRIANGLES, 3 * indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);																							
		glfwPollEvents();
	};                                  
    
	glfwTerminate();																										
	return 0;
}