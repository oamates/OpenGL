//========================================================================================================================================================================================================================
// DEMO 023: Rendering to multisample texture
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>

#include "constants.hpp"
#include "glfw_window.hpp"
#include "log.hpp"
#include "camera3d.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "fbo.hpp"

//========================================================================================================================================================================================================================
// 3d moving camera : standard initial orientation in space
//========================================================================================================================================================================================================================
bool position_changed = false;
const double linear_velocity = 0.01f;
const double angular_rate = 0.0001f;
static camera3d camera;

//========================================================================================================================================================================================================================
// keyboard and mouse handlers
//========================================================================================================================================================================================================================

void keyboard_handler(int key, int scancode, int action, int mods)
{
    if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) { camera.move_forward(linear_velocity);   position_changed = true; }
    else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) { camera.move_backward(linear_velocity);  position_changed = true; }
    else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) { camera.straight_right(linear_velocity); position_changed = true; }
    else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) { camera.straight_left(linear_velocity);  position_changed = true; }
};

void mouse_handler(double dx, double dy, double duration)
{
    duration = glm::max(duration, 0.01);    
    double norm = sqrt(dx * dx + dy * dy);
    if (norm > 0.01f)
    {
        dx /= norm; dy /= norm;
        double angle = angular_rate * sqrt(norm) / (duration + 0.01);
        camera.rotateXY(dx, dy, angle);
    };
};

const int CUBE_SIZE = 0x08;
const int HOLE_SIZE = 0x04;
const int POINT_COUNT = (2 * CUBE_SIZE + 1) * (2 * CUBE_SIZE + 1) * (2 * CUBE_SIZE + 1) - (2 * HOLE_SIZE + 1) * (2 * HOLE_SIZE + 1) * (2 * HOLE_SIZE + 1);

int main()
{
    //===================================================================================================================================================================================================================
    // GLFW window creation + GLEW library initialization
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    glfw_window window("Rendering to multisample texture", 8, 3, 3, 1920, 1080);
    window.log_info();
    window.mouse_handler = mouse_handler;
    window.keyboard_handler = keyboard_handler;
    camera.infinite_perspective(constants::two_pi / 6.0f, window.aspect_ratio(), 0.1f);
	glClearColor(0.01f, 0.00f, 0.06f, 1.00f);																				// dark blue background

	// ==================================================================================================================================================================================================================
	// Creating shaders and uniforms
	// ==================================================================================================================================================================================================================

    glsl_program cubes_program(glsl_shader(GL_VERTEX_SHADER,   "glsl/cubes.vs"),
                               glsl_shader(GL_GEOMETRY_SHADER, "glsl/cubes.gs"),
                               glsl_shader(GL_FRAGMENT_SHADER, "glsl/cubes.fs"));

    cubes_program.enable();

	GLuint uniform_projection_matrix = cubes_program.uniform_id("projection_matrix");						                // projection matrix uniform id
	GLuint uniform_view_matrix = cubes_program.uniform_id("view_matrix");						                            // view matrix uniform id
	GLuint uniform_texture_sampler = cubes_program.uniform_id("texture_sampler");				                            // texture_sampler uniform
	GLuint uniform_global_time = cubes_program.uniform_id("global_time");				                                    // time uniform

	glUniformMatrix4fv(uniform_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera.projection_matrix));		                	// set up projection matrix, it is not going to change
    glUniform1i(uniform_texture_sampler, 0);																				// set our "texture_sampler" to use texture unit 0

	// ==================================================================================================================================================================================================================
	// Point data initialization 
	// ==================================================================================================================================================================================================================

    GLuint vao_id, vbo_id, ibo_id;
    
    std::vector<glm::mat4> points;
    std::vector<GLushort> indices;
    points.reserve(POINT_COUNT);
    indices.reserve(POINT_COUNT);

    GLushort index = 0;
    for (int i = -CUBE_SIZE; i <= CUBE_SIZE; ++i)
    for (int j = -CUBE_SIZE; j <= CUBE_SIZE; ++j)
    for (int k = -CUBE_SIZE; k <= CUBE_SIZE; ++k)
    {
        if ((abs(i) > HOLE_SIZE) || (abs(j) > HOLE_SIZE) || (abs(k) > HOLE_SIZE))
		{
        	glm::vec3 axis_z = glm::sphericalRand(1.0f);
	        glm::vec3 axis_x = glm::normalize(glm::cross(axis_z, glm::sphericalRand(1.0f)));
    	    glm::vec3 axis_y = glm::cross(axis_z, axis_x);
        	points.push_back(glm::mat4(glm::vec4(axis_x, 0.0f),
            	                       glm::vec4(axis_y, 0.0f),
                	                   glm::vec4(axis_z, 0.0f),
                    	               glm::vec4(6.0f * glm::vec3(i, j, k), 1.0f)));
	        indices.push_back(index++);
		};
    };
 

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, POINT_COUNT * sizeof(glm::mat4), glm::value_ptr(points[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 64, (void*)(0));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 64, (void*)(16));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 64, (void*)(32));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 64, (void*)(48));


    glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, POINT_COUNT * sizeof(GLushort), &indices[0], GL_DYNAMIC_DRAW);

	// ==================================================================================================================================================================================================================
	// Load texture for cube faces
	// ==================================================================================================================================================================================================================

    glActiveTexture(GL_TEXTURE0);
	GLuint cube_texture_id = texture::texture2d_png("res/cube.png");
    glBindTexture(GL_TEXTURE_2D, cube_texture_id);
    glUniform1i(uniform_texture_sampler, 0);																						// set our "texture_sampler" to use texture unit 0



	// ==================================================================================================================================================================================================================
	// Simple cube VAO generated by hands, no geometry shader
	// ==================================================================================================================================================================================================================

    const unsigned int mesh_size = 36;
    GLuint cube_vao_id;
    GLuint cube_vbo_id, cube_nbo_id, cube_tbo_id;

    glGenVertexArrays(1, &cube_vao_id);
    glBindVertexArray(cube_vao_id);
    const float cube_size = 10.0f;
	const glm::vec3 vertex[] = 
	{
		glm::vec3(-cube_size, -cube_size, -cube_size),
		glm::vec3( cube_size, -cube_size, -cube_size),
		glm::vec3(-cube_size,  cube_size, -cube_size),
		glm::vec3( cube_size,  cube_size, -cube_size),
		glm::vec3(-cube_size, -cube_size,  cube_size),
		glm::vec3( cube_size, -cube_size,  cube_size),
		glm::vec3(-cube_size,  cube_size,  cube_size),
		glm::vec3( cube_size,  cube_size,  cube_size)
	};

	const glm::vec3 triangulation[] = 
	{
		vertex[0], vertex[2], vertex[3], vertex[0], vertex[3], vertex[1],													// faces parallel to xy plane : the face [0231] and ...
		vertex[4], vertex[5], vertex[7], vertex[4], vertex[7], vertex[6],													// ... the face [4576]
		vertex[0], vertex[4], vertex[6], vertex[0], vertex[6], vertex[2],													// faces parallel to yz plane : the face [0462] and ...
		vertex[1], vertex[3], vertex[7], vertex[1], vertex[7], vertex[5],													// ... the face [1375]
		vertex[0], vertex[1], vertex[5], vertex[0], vertex[5], vertex[4],									            	// faces parallel to zx plane : the face [0154] and ...
		vertex[2], vertex[6], vertex[7], vertex[2], vertex[7], vertex[3]													// ... the face [2673]
	};   

    glEnableVertexAttribArray(0);
    glGenBuffers(1, &cube_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangulation), glm::value_ptr(triangulation[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	const glm::vec3 normal[] = 
    { 
		glm::vec3 ( 1.0f,  0.0f,  0.0f),
		glm::vec3 (-1.0f,  0.0f,  0.0f),
		glm::vec3 ( 0.0f,  1.0f,  0.0f),
		glm::vec3 ( 0.0f, -1.0f,  0.0f),
		glm::vec3 ( 0.0f,  0.0f,  1.0f),
		glm::vec3 ( 0.0f,  0.0f, -1.0f)
	};

	const glm::vec3 normal_data[] = 
	{                                                                                                                   	
		normal[5], normal[5], normal[5], normal[5], normal[5], normal[5],													// faces parallel to xy plane : the face [0231] and ...            
		normal[4], normal[4], normal[4], normal[4], normal[4], normal[4],                                                   // ... the face [4576]                                             
		normal[1], normal[1], normal[1], normal[1], normal[1], normal[1],                                                   // faces parallel to yz plane : the face [0462] and ...            
		normal[0], normal[0], normal[0], normal[0], normal[0], normal[0],                                                   // ... the face [1375]                                             
		normal[3], normal[3], normal[3], normal[3], normal[3], normal[3],                                                   // faces parallel to zx plane : the face [0154] and ...            
		normal[2], normal[2], normal[2], normal[2], normal[2], normal[2]                                                    // ... the face [2673]                                             
	};   

    glEnableVertexAttribArray(1);
    glGenBuffers(1, &cube_nbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, cube_nbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normal_data), glm::value_ptr(normal_data[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	const glm::vec2 tc[] = 
    { 
		glm::vec2 (0.0f, 0.0f),
		glm::vec2 (1.0f, 0.0f),
		glm::vec2 (1.0f, 1.0f),
		glm::vec2 (0.0f, 1.0f) 
	};

	const glm::vec2 texture_coords[] = 
	{
		tc[0], tc[1], tc[2], tc[0], tc[2], tc[3],
		tc[0], tc[1], tc[2], tc[0], tc[2], tc[3],
		tc[0], tc[1], tc[2], tc[0], tc[2], tc[3],
		tc[0], tc[1], tc[2], tc[0], tc[2], tc[3],
		tc[0], tc[1], tc[2], tc[0], tc[2], tc[3],
		tc[0], tc[1], tc[2], tc[0], tc[2], tc[3]
	};   

    glEnableVertexAttribArray(2);
    glGenBuffers(1, &cube_tbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, cube_tbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coords), glm::value_ptr(texture_coords[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glsl_program simple_light(glsl_shader(GL_VERTEX_SHADER,   "glsl/simple_light.vs"),
                              glsl_shader(GL_FRAGMENT_SHADER, "glsl/simple_light.fs"));

    simple_light.enable();

	GLuint simple_light_projection_matrix = simple_light.uniform_id("projection_matrix");						            // projection_view matrix uniform id
	GLuint simple_light_view_matrix = simple_light.uniform_id("view_matrix");						                        // projection_view matrix uniform id
	GLuint simple_light_texture_sampler = simple_light.uniform_id("texture_sampler");				                        // "texture_sampler" uniform

	glUniformMatrix4fv(simple_light_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera.projection_matrix));		                // set up projection matrix, it is not going to change
    glUniform1i(simple_light_texture_sampler, 0);																			// set our "texture_sampler" to use texture unit 0


	// ==================================================================================================================================================================================================================
	// Creating additional framebuffer for rendering to texture
	// ==================================================================================================================================================================================================================

    glEnable(GL_MULTISAMPLE);

	msfbo_color mirror_msfbo(window.res_x, window.res_y);

	// ==================================================================================================================================================================================================================
	// Camera, view_matrix and projection_matrix initialization                                                                                                                        `
	// ==================================================================================================================================================================================================================


	//===================================================================================================================================================================================================================
	// The main loop
	//===================================================================================================================================================================================================================

	while(!window.should_close())
    {

   	    //===============================================================================================================================================================================================================
	    // rendering the scene to cubemap texture frame buffer
	    //===============================================================================================================================================================================================================
    	mirror_msfbo.bind();
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);


        cubes_program.enable();
        glBindVertexArray(vao_id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cube_texture_id);
        glUniform1f(uniform_global_time, (float) glfwGetTime());
		glUniformMatrix4fv(uniform_view_matrix, 1, GL_FALSE, glm::value_ptr(camera.view_matrix));		                             
        glDrawElements(GL_POINTS, POINT_COUNT, GL_UNSIGNED_SHORT, 0);        

   	    //===============================================================================================================================================================================================================
	    // rendering central cube using created texture
	    //===============================================================================================================================================================================================================
  
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
		simple_light.enable();
		mirror_msfbo.bind_texture(GL_TEXTURE0);
        glBindVertexArray(cube_vao_id);
		glUniformMatrix4fv(simple_light_view_matrix, 1, GL_FALSE, glm::value_ptr(camera.view_matrix));		                             
        glDrawArrays(GL_TRIANGLES, 0, mesh_size);

	   	// ==============================================================================================================================================================================================================
	    // Rendering the main scene to the screen
	    // ==============================================================================================================================================================================================================

        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

        cubes_program.enable();
        glBindVertexArray(vao_id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cube_texture_id);
        glUniform1f(uniform_global_time, (float) glfwGetTime());
		glUniformMatrix4fv(uniform_view_matrix, 1, GL_FALSE, glm::value_ptr(camera.view_matrix));		                             
        glDrawElements(GL_POINTS, POINT_COUNT, GL_UNSIGNED_SHORT, 0);        


        if (position_changed)
        {
     		glm::vec4 position = camera.position();
            bool index_order_changed = false;
            bool done = false;
			unsigned int iteration = 0;
            while (!done)
            {
                done = true;
                float norm1 = glm::length2(camera.view_matrix * points[indices[0]][3]);

            	for(unsigned int i = 1; i < POINT_COUNT - iteration; ++i)
                {
                    float norm2 = glm::length2(camera.view_matrix * points[indices[i]][3]);
					if ((norm2 - norm1) > 0.01)
					{
						GLushort q = indices[i - 1];
						indices[i - 1] = indices[i];
						indices[i] = q;
                        done = false;
					}
                    else
                        norm1 = norm2;
                    
				};
			    index_order_changed |= (!done);
                ++iteration;
            };
			debug_msg("Sorted after %u iterations", iteration);
			if (index_order_changed)
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, POINT_COUNT * sizeof(GLushort), &indices[0], GL_DYNAMIC_DRAW);
            position_changed = false;
		};
        window.swap_buffers();
        window.poll_events();
    }; 
    
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    return 0;
};