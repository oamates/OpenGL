#ifndef _model_included_2357861205786232406352783465872435681273516434235167554 
#define _model_included_2357861205786232406352783465872435681273516434235167554

#include <string>
#include <glm/glm.hpp>

//=======================================================================================================================================================================================================================
// A simple abstraction of a 3d model to load a wavefront obj model file and setup a
// vertex array object with 3 attributes : position, normals, uv
//=======================================================================================================================================================================================================================

struct model 
{
	GLuint vao_id;																									// vertex array object name
	GLuint vbo_id, ibo_id;																							// vertex buffer object and index buffer object names
	GLuint elements;																								// number of indices in the element array buffer

	glm::vec3 shift;
	glm::vec3 scale;
	glm::mat3 rotation;	
	
	model(const char * file_name, GLuint program, GLuint shadowProgram = 0);										// Load the model from a file and give it a shader program to use. The shader program should take position, normal and uv as inputs 0, 1, 2            
	                                                                                                                // and have a uniform mat4 input for the model matrix if not doing instanced rendering                                                                 
	
	~model();																										// free the model's buffers and program

	
	void bind();																									// bind the model's buffers for rendering

	void bindShadow();																								// Bind the model and its program for shadow map pass
	
	void setShadowVP(const glm::mat4 &vp);																			// set the shadow pass view / projection matrix this is sorta hacked in
	
	void translate(const glm::vec3 &vec);																			// translate the model
	
	void rotate(const glm::mat4 &rot);																				// rotate the model

	void scale(const glm::vec3 &scale);																				// scale the model
	
	void load(const char * file_name);																				// Load the model from the file and setup the VAO
	
	void updateMatrix();																							// Update the uniform model matrix being sent to the shader
};

#endif //_model_included_2357861205786232406352783465872435681273516434235167554

