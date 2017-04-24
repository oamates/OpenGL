#include <iostream>
#include <random>
#include <cstdlib>
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

//=======================================================================================================================================================================================================================
// figure struct for tetris game
//=======================================================================================================================================================================================================================
struct figure
{
	//===================================================================================================================================================================================================================
	// variables
	//===================================================================================================================================================================================================================
	static const unsigned int MAX_SECTION_SIZE = 64;
	int size, size_z, size_x, size_y;
	int* data;
    
	//===================================================================================================================================================================================================================
	// member functions
	//===================================================================================================================================================================================================================
	figure(int size_z, int size_x, int size_y, int* figure_data) : size_z(size_z), size_x(size_x), size_y(size_y)
	{
		size = size_z * size_y * size_x;
		if (figure_data != 0)
		{
			data = new int [size];
			for (int i = 0; i < size; ++i) data[i] = figure_data[i];
		}
		else
			data = 0;
	};

	~figure()
	{
		delete[] data;
	};

	void rotate_z()
	{
		unsigned int q[MAX_SECTION_SIZE];
		for (int z = 0; z < size_z; ++z)
		{
			for(int y = 0; y < size_y; ++y)
				for(int x = 0; x < size_x; ++x)
					q[size_x * y + x] = data[size_y * size_x * z + size_x * y + x];

			for(int y = 0; y < size_y; ++y)
				for(int x = 0; x < size_x; ++x)
					data[size_y * size_x * z + (size_x - x - 1) * size_y + y] = q[size_x * y + x];
		};
		int s = size_x;
		size_x = size_y;
		size_y = s;
	};                                                                                                                                                                                                                                      

	void rotate_x()
	{
		unsigned int q[MAX_SECTION_SIZE];
		for (int x = 0; x < size_x; ++x)
		{
			for(int z = 0; z < size_z; ++z)
				for(int y = 0; y < size_y; ++y)
					q[size_y * z + y] = data[size_y * size_x * z + size_x * y + x];

			for(int z = 0; z < size_z; ++z)
				for(int y = 0; y < size_y; ++y)
					data[size_z * size_x * (size_y - y - 1) + z * size_x + x] = q[size_y * z + y];
		};
		int s = size_y;
		size_y = size_z;
		size_z = s;
	};                                                                                                                                                                                                                                      

	void rotate_y()
	{
		unsigned int q[MAX_SECTION_SIZE];
		for (int y = 0; y < size_y; ++y)
		{
			for(int x = 0; x < size_x; ++x)
				for(int z = 0; z < size_z; ++z)
					q[size_z * x + z] = data[size_y * size_x * z + size_x * y + x];

			for(int x = 0; x < size_x; ++x)
				for(int z = 0; z < size_z; ++z)
					data[size_y * size_z * x + size_z * y + (size_z - z - 1)] = q[size_z * x + z];
		};
		int s = size_x;
		size_x = size_z;
		size_z = s;
	};                                                                                                                                                                                                                                      
};


//=======================================================================================================================================================================================================================
// tetris game
//=======================================================================================================================================================================================================================
struct tetris
{	
	//===================================================================================================================================================================================================================
	// variables
	//===================================================================================================================================================================================================================
	static const int WIDTHX = 8;
	static const int WIDTHY = 8;
	static const int HEIGHT = 14;

	std::vector<figure> figures;
	figure current_figure;
	int _z, _y, _x;						// current figure position

	int	field[HEIGHT][WIDTHY][WIDTHX];	// 0 = the cell is empty
										// 1 = the cell is occupied
	int row_count[HEIGHT];
	int score;

    std::mt19937 generator;

	double time_stamp;

	//===================================================================================================================================================================================================================
	// member functions
	//===================================================================================================================================================================================================================
	tetris() : current_figure(0, 0, 0, 0) 
	{
		
		generator.seed(0);
	};

	void add_figure(const figure& f)
	{
		figures.push_back(f);
	};

	void move_figure()
	{
		
	};

	GLuint set_shift(GLint shift_id, float cube_size)
	{
	    std::vector<glm::vec3> shift_vec;
		for(int z = 0; z < HEIGHT; ++z)
			for(int y = 0; y < WIDTHY; ++y)  
				for(int x = 0; x < WIDTHX; ++x)
					if (field[z][y][x] != 0)
					{
						glm::vec3 center = (cube_size + 0.05f) * glm::vec3(float(x) - 0.5f * (WIDTHX - 1), float(y) - 0.5f * (WIDTHY - 1), float(z));
						shift_vec.push_back(center);
					};
		int size = shift_vec.size();
		glUniform3fv(shift_id, size, glm::value_ptr(shift_vec[0]));
		return size;									
	};

	

	void put_figure(const figure& f, int z, int x, int y)
	{

		for (int z = 0; z < f.size_z; ++z)
		{
			for(int y = 0; y < f.size_y; ++y)
			{
				for(int x = 0; x < f.size_x; ++x)
				{
					if (f.data[f.size_y * f.size_x * z + y * f.size_x + x] == 1)
					{
						field[z][y][x] = 1;							
						++row_count[z];
					};			
				};                   
			};
		};
	};

	void eliminate_row(int z)
	{
		for(int i = z + 1; i < HEIGHT; ++i)
		{
			row_count[i - 1] = row_count[i];
			for(int y = 0; y < WIDTHY; ++y)
			{
				for(int x = 0; x < WIDTHX; ++x)
				{
					field[i - 1][y][x] = field[i][y][x];
				};
			};
		};
		row_count[HEIGHT - 1] = 0;
		for(int y = 0; y < WIDTHY; ++y)
		{
			for(int x = 0; x < WIDTHX; ++x)
			{
				field[HEIGHT - 1][y][x] = 0;
			};
		};
	};

	void eliminate_rows()
	{
		int rows = 0;
		int z = 0;
		while(z < HEIGHT)
		{
			if (row_count[z] == WIDTHX * WIDTHY) 
			{
				eliminate_row(z);
				++rows;
			}
		    else ++z;
		};
		score = score + 64 * rows * rows;
	};

	bool can_rotate_z()
	{
		
		
	};	


    void rotate_x() 
	{
		unsigned int q[MAX_SECTION_SIZE];
		for (int x = 0; x < size_x; ++x)
		{
			for(int z = 0; z < size_z; ++z)
				for(int y = 0; y < size_y; ++y)
					q[size_y * z + y] = data[size_y * size_x * z + size_x * y + x];
		
			for(int z = 0; z < size_z; ++z)
				for(int y = 0; y < size_y; ++y)
					data[size_z * size_x * (size_y - y - 1) + z * size_x + x] = q[size_y * z + y];
		};


		unsigned int q[MAX_SECTION_SIZE];
		for (int x = 0; x < size_x; ++x)
		{
			for(int z = 0; z < size_z; ++z)
				for(int y = 0; y < size_y; ++y)
					q[size_y * z + y] = data[size_y * size_x * z + size_x * y + x];
		
			for(int z = 0; z < size_z; ++z)
				for(int y = 0; y < size_y; ++y)
					data[size_z * size_x * (size_y - y - 1) + z * size_x + x] = q[size_y * z + y];
		};
		int s = size_y;
		size_y = size_z;
		size_z = s;
	};



    void rotate_x_inv() {};
	void rotate_y() {};
    void rotate_y_inv() {};
	void rotate_z() {};
    void rotate_z_inv() {};

    void drop() 
	{
		current_figure.size_z
		
		bool can_move = true;

		while ((_z > current_figure.size_z - 1) && )
		{

			can_move = true;

			for (int z = 0; z < current_figure.size_z; ++z)
				for(int y = 0; y < current_figure.size_y; ++y)
					for(int x = 0; x < current_figure.size_x; ++x)
						if ((current_figure.data[current_figure.size_y * current_figure.size_x * z + y * current_figure.size_x + x] == 1) && (field[z][y][x] == 1)) can_move = false;										
			
			if () --_z;
		}

	};


    void shift(int dz, int dy, int dx) 
	{
		int _z, _y, _x;						// current figure position

		_z + dz, _y + dy, _x + dx	
		if ((_x + dx + current_figure.size_x > WIDTHX) || (_y  + dy + current_figure.size_y > WIDTHY) || (_z + dz - current_figure.size_z + 1 < 0)) return;

		for (int z = 0; z < current_figure.size_z; ++z)
			for(int y = 0; y < current_figure.size_y; ++y)
				for(int x = 0; x < current_figure.size_x; ++x)
					if ((current_figure.data[current_figure.size_y * current_figure.size_x * z + y * current_figure.size_x + x] == 1) && (field[z][y][x] == 1)) return;										

		_z = _z + dz; 
		_y = _y + dy;
		_x = _x + dx;
	};	

	void restart(double t) 
	{
		time_stamp = t;
		score = 0;
		for(int z = 0; z < HEIGHT; ++z)
		{
			row_count[z] = 0;
			for(int y = 0; y < WIDTHY; ++y)
				for(int x = 0; x < WIDTHX; ++x) field[z][y][x] = 1;
		};

		current_figure = 

	};
	
};


//=======================================================================================================================================================================================================================
// Window related variables
//=======================================================================================================================================================================================================================
const unsigned int res_x = 1920;
const unsigned int res_y = 1080;
GLFWwindow* window;
tetris game; 

//=======================================================================================================================================================================================================================
// View matrix
//=======================================================================================================================================================================================================================
const float velocity = 0.7f;
const float spheric_velocity = 0.0001f;
const float two_pi = 6.283185307179586476925286766559;
glm::mat4 view_matrix = glm::mat4(1.0f);
bool view_changed;

//=======================================================================================================================================================================================================================
// Mouse move callback function
//=======================================================================================================================================================================================================================
void onMouseMove (GLFWwindow * window, double x, double y)
{
	static double time_stamp = 0.0;
	static double mouse_x = 0.0, mouse_y = 0.0;
	double dx = mouse_x - x; mouse_x = x;
	double dy = y - mouse_y; mouse_y = y;
	double new_stamp = glfwGetTime();
	double duration = new_stamp - time_stamp;
	time_stamp = new_stamp;  
	double norm = sqrt(dx * dx + dy * dy);

	if (norm > 0.01f)
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
		view_changed = true;
	};
};

//=======================================================================================================================================================================================================================
// Keyboard event callback function
//=======================================================================================================================================================================================================================
void onKeypressed (GLFWwindow*, int key, int scancode, int action, int mods)
{
	//===================================================================================================================================================================================================================
	// A, S, D, W control view matrix
	//===================================================================================================================================================================================================================
	if ((key == GLFW_KEY_W) || (key == GLFW_KEY_S) || (key == GLFW_KEY_D) || (key == GLFW_KEY_A))
	{
		view_changed = true;
		glm::mat4 translation = glm::mat4 (1.0);
		if (key == GLFW_KEY_W)
			translation[3][2] =  velocity;
		else if (key == GLFW_KEY_S)
			translation[3][2] = -velocity;
		else if (key == GLFW_KEY_D)
			translation[3][0] = -velocity;
		else // key == GLFW_KEY_A
			translation[3][0] =  velocity;
	    view_matrix = translation * view_matrix;
		return;
	};

	//===================================================================================================================================================================================================================
	// KEY_UP, KEY_UP, KEY_UP, KEY_UP move tetris figure in x, y - plane
	//===================================================================================================================================================================================================================
	if ((key == GLFW_KEY_UP) || (key == GLFW_KEY_DOWN) || (key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_LEFT))
	{
		if (key == GLFW_KEY_UP)
			game.rotate_x();
		else if (key == GLFW_KEY_DOWN)
			game.rotate_x_inv();
		else if (key == GLFW_KEY_RIGHT)
			game.rotate_y();
		else // key == GLFW_KEY_LEFT
			game.rotate_y_inv();
		return;	
	};

	//===================================================================================================================================================================================================================
	// keypad 2,4,6,8 keys rotate figure about x and y axis by 90 or -90 degrees
	//===================================================================================================================================================================================================================
	if ((key == GLFW_KEY_KP_2) || (key == GLFW_KEY_KP_4) || (key == GLFW_KEY_KP_6) || (key == GLFW_KEY_KP_8))
	{
		if (key == GLFW_KEY_KP_2)
			game.shift(0, -1, 0);
		else if (key == GLFW_KEY_KP_4)
			game.shift(0, 0, -1);
		else if (key == GLFW_KEY_KP_6)
			game.shift(0, 1, 0);
		else // key == GLFW_KEY_KP_8
			game.shift(0, 0, 1);
		return;	
	};
	
	//===================================================================================================================================================================================================================
	// keypad / and * rotate the figure by 90 and -90 degrees about z axis respectively
	//===================================================================================================================================================================================================================
    if ((key == GLFW_KEY_KP_DIVIDE) || (key == GLFW_KEY_KP_MULTIPLY))
	{
		if (key == GLFW_KEY_KP_DIVIDE)
			game.rotate_z();
		else
			game.rotate_z_inv();
		return;	
	};
	
	//===================================================================================================================================================================================================================
	// ENTER (keypad ENTER) key drops the figure down
	//===================================================================================================================================================================================================================
	if ((key == GLFW_KEY_ENTER) || (key == GLFW_KEY_KP_ENTER)) 
	{
		game.drop();
		return;
	}

	//===================================================================================================================================================================================================================
	// N key starts the new game
	//===================================================================================================================================================================================================================
	if (key == GLFW_KEY_N) game.restart(glfwGetTime());

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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, 0, 0);
	glfwSetKeyCallback(window, onKeypressed);
	glfwSetCursorPosCallback(window, onMouseMove);		

	const float two_pi = 6.283185307179586476925286766559; 
	glm::mat4 projection_matrix = glm::infinitePerspective (two_pi / 6.0f, float(res_x) / float(res_y), 0.1f); 		        						// projection matrix : 60° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units

	// ==================================================================================================================================================================================================================
	// simple shader for tetris figures
	// ==================================================================================================================================================================================================================

    glsl_program tetris_shader(glsl_shader(GL_VERTEX_SHADER,   "glsl/tetris.vs"),
                       glsl_shader(GL_FRAGMENT_SHADER, "glsl/tetris.fs"));
	tetris_shader.enable();

	GLint shift_id = tetris_shader.uniform_id("shift");
	GLint view_matrix_id = tetris_shader.uniform_id("view_matrix");
	GLint projection_matrix_id = tetris_shader.uniform_id("projection_matrix");
	glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

	// ==================================================================================================================================================================================================================
	// cube
	// ==================================================================================================================================================================================================================
	
	float half_cube_size = 0.5f;

	glm::vec3 cube_vertices [] =
	{
		half_cube_size * glm::vec3(-1.0f, -1.0f, -1.0f),	
		half_cube_size * glm::vec3(-1.0f,  1.0f, -1.0f),
		half_cube_size * glm::vec3(-1.0f, -1.0f,  1.0f),
		half_cube_size * glm::vec3(-1.0f,  1.0f,  1.0f),	
		half_cube_size * glm::vec3( 1.0f, -1.0f, -1.0f),
		half_cube_size * glm::vec3( 1.0f,  1.0f, -1.0f),
		half_cube_size * glm::vec3( 1.0f, -1.0f,  1.0f),	
		half_cube_size * glm::vec3( 1.0f,  1.0f,  1.0f)
	};

	glm::vec3 cube_texcoords [] =
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

	GLubyte cube_indices[] = {  	0,1,3, 0,3,2,
									2,3,6, 3,7,6,
		                            4,1,0, 4,5,1,
        		                    2,4,0, 2,6,4,
                		            1,5,3, 5,7,3,
                        		    7,5,6, 6,5,4
								};

	GLuint vao_id, vbo_id, tbo_id, ibo_id;

	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), glm::value_ptr(cube_vertices[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &tbo_id);                                                                                 
	glBindBuffer(GL_ARRAY_BUFFER, tbo_id);                                                                    
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_texcoords), glm::value_ptr(cube_texcoords[0]), GL_STATIC_DRAW);     
	glEnableVertexAttribArray(1);                                                                             
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);                                                    

	glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), &cube_indices[0], GL_STATIC_DRAW);     

	glEnable(GL_DEPTH_TEST);

	// ==================================================================================================================================================================================================================
	// Game logic initialization
	// ==================================================================================================================================================================================================================
		
	int cube1x1x1_data[] = {1};
	int plane_square2x2_data[] = {1, 1, 1, 1};
	int cube2x2x2_data[] = {1, 1, 1, 1, 1, 1, 1, 1};
	int stick3x1x1_data[] = {1, 1, 1};
	int plane_cross_data[] = {0, 1, 0, 1, 1, 1, 0, 1, 0};
	int space_cross_data[] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0};
	int cube3x3x3_data[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

	game.add_figure(figure(1, 1, 1, cube1x1x1_data));
	game.add_figure(figure(1, 2, 2, plane_square2x2_data));
	game.add_figure(figure(2, 2, 2, cube2x2x2_data));
	game.add_figure(figure(2, 2, 2, stick3x1x1_data));
	game.add_figure(figure(1, 3, 3, plane_cross_data));
	game.add_figure(figure(3, 3, 3, space_cross_data));
	game.add_figure(figure(3, 3, 3, cube3x3x3_data));


	game.restart(glfwGetTime());

	while(!glfwWindowShouldClose(window))
	{
		view_changed = false;
		double t = glfwGetTime();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);																    // clear the screen
		
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(view_matrix));

		GLuint size = game.set_shift(shift_id, 2.0f * half_cube_size);
        glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0, size);

		glfwSwapBuffers(window);																							// swap buffers
		glfwPollEvents();
	}; 

	glfwTerminate();																										// close OpenGL window and terminate GLFW
	return 0;
}