#ifndef _camera_included_109473687243345458978054261543657840978841435479995145
#define _camera_included_109473687243345458978054261543657840978841435479995145

#include <GLFW\glfw3.h>																										// Windows and event management library

#define GLM_FORCE_RADIANS 
#include <glm\glm.hpp>																										// OpenGL mathematics
#include <glm\gtx\transform.hpp> 
#include <glm\gtc\matrix_transform.hpp>																						// for transformation matrices to work

void init_camera(GLFWwindow* window);
void init_camera(GLFWwindow* window, const glm::vec3& position, const glm::vec3 unit_x, const glm::vec3 unit_y);
extern glm::mat4 view_matrix;

#endif	// _camera_included_109473687243345458978054261543657840978841435479995145




