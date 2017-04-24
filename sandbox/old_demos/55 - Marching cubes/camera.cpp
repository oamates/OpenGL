#include "camera.hpp"
#include "log.hpp"

const float velocity = 0.7f;
const float spheric_velocity = 0.0001f;
const float two_pi = 6.283185307179586476925286766559;

GLFWwindow* window;
//glm::mat4 view_matrix = glm::mat4(1.0f);
glm::mat4 view_matrix = glm::translate(glm::vec3(0.0f, 0.0f, 20.0f)) * glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));

double time_stamp;
double mouse_x = 0.0, mouse_y = 0.0;

void onMouseMove (GLFWwindow*, double, double);
void onKeypressed (GLFWwindow*, int, int, int, int);

void bindInputs()
{
	time_stamp = glfwGetTime();
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, 0, 0);
	glfwSetKeyCallback(window, onKeypressed);
	glfwSetCursorPosCallback(window, onMouseMove);		
};

void init_camera(GLFWwindow* w) 
{
	window = w;
	bindInputs();
};

void init_camera(GLFWwindow* w, const glm::vec3& position, const glm::vec3 unit_x, const glm::vec3 unit_y) 
{
	window = w;
	glm::vec3 unit_z = glm::cross(unit_x, unit_y);
	view_matrix = glm::lookAt(position, unit_z, unit_y);
	bindInputs();
};

void onMouseMove (GLFWwindow * window, double x, double y)
{
	double dx = mouse_x - x; mouse_x = x;
	double dy = y - mouse_y; mouse_y = y;
	double new_stamp = glfwGetTime();
	double duration = new_stamp - time_stamp;
	time_stamp = new_stamp;  
	double norm = sqrt(dx * dx + dy * dy);
	if ((norm > 0.01f) && (norm < 20.0f))
	{
		dx /= norm;
		dy /= norm;  
		double angle = (sqrt(norm) / duration) * spheric_velocity;
		double cs = cos(angle);
		double sn = sin(angle);
		double _1mcs = 1 - cs;
		glm::mat4 rotor = glm::mat4 (1.0 - dx * dx * _1mcs,     - dx * dy * _1mcs,  sn * dx, 0.0f, 
									     - dx * dy * _1mcs, 1.0 - dy * dy * _1mcs,  sn * dy, 0.0f,
									             - sn * dx,             - sn * dy,       cs, 0.0f,
									                  0.0f,                  0.0f,     0.0f, 1.0f);
		view_matrix = rotor * view_matrix;
	};
};

void onKeypressed (GLFWwindow*, int key, int scancode, int action, int mods)
{
	glm::mat4 translation = glm::mat4 (1.0);
	if ((key == GLFW_KEY_UP) || (key == GLFW_KEY_W))         translation[3][2] =  velocity;
	else if ((key == GLFW_KEY_DOWN) || (key == GLFW_KEY_S))  translation[3][2] = -velocity;
	else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) translation[3][0] = -velocity;
	else if ((key == GLFW_KEY_LEFT) || (key == GLFW_KEY_A))  translation[3][0] =  velocity;
    view_matrix = translation * view_matrix;

    static float psize = 1.0f;
    if (key == GLFW_KEY_KP_ADD)
    {
        psize += 0.2;
        glPointSize(psize);
    }
    else if (key == GLFW_KEY_KP_SUBTRACT)
    {
        psize -= 0.2;
        glPointSize(psize);
    }
};