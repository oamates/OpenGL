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
#include "texture.hpp"
#include "solid.hpp"
#include "plato.hpp"
#include "shadowmap.hpp"

const float two_pi = 6.283185307179586476925286766559; 
const float radius = 40.0f;

const unsigned int res_x = 1920;
const unsigned int res_y = 1080;
const unsigned int POINT_COUNT = 0x2000;

glm::vec3 torus_func (const glm::vec2& point)
{
	const double R = 1.5;
	const double r = 0.6;
	double cos_2piu = cos(two_pi * point.x);
	double sin_2piu = sin(two_pi * point.x);
	double cos_2piv = cos(two_pi * point.y);
	double sin_2piv = sin(two_pi * point.y);
	return glm::vec3 ((R + r * cos_2piv) * cos_2piu, (R + r * cos_2piv) * sin_2piu, r * sin_2piv);		
};	

struct solid_group
{
	std::vector<glm::vec4> positions;
	GLuint ubo_id;
	unsigned int size;
	unsigned int instances;

    solid_group(unsigned int size, float distance, const glm::vec3& center) : size(size), instances(size * size * size)
	{
		float minimum  = -distance * (size - 1) * 0.5f;

		positions.reserve(instances);
		glm::vec4 position;

		position.w = 1.0f;
		position.x = center.x + minimum;
		for (unsigned int i = 0; i < size; ++i)
		{ 
			position.y = center.y + minimum;
			for (unsigned int j = 0; j < size; ++j)
			{
				position.z = center.z + minimum;
				for (unsigned int k = 0; k < size; ++k)
				{
					positions.push_back(position);
					position.z += distance;
				};
				position.y += distance;
			};
			position.x += distance;
		};
		glGenBuffers(1, &ubo_id);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_id, 0, sizeof(glm::mat4) * instances);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) * instances, glm::value_ptr(positions[0]), GL_STATIC_DRAW);
	};

	~solid_group()
	{
		glDeleteBuffers(1, &ubo_id);
	};

	void uniform_bind()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_id, 0, sizeof(glm::vec4) * instances);
	};
	
	void uniform_update()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_id, 0, sizeof(glm::vec4) * instances);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) * instances, glm::value_ptr(positions[0]), GL_DYNAMIC_DRAW);
	};
};


int main()
{

	// ==================================================================================================================================================================================================================
	// GLFW library initialization
	// ==================================================================================================================================================================================================================

	if(!glfwInit()) exit_msg("Failed to initialize GLFW.");                                                                 // initialise GLFW

	glfwWindowHint(GLFW_SAMPLES, 8); 																						// 8x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); 																			// we want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 																	
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 															// request core profile
							
	GLFWwindow* window; 																									// open a window and create its OpenGL context 
	window = glfwCreateWindow(res_x, res_y, "Plato solids", glfwGetPrimaryMonitor(), 0); 
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

	// ==================================================================================================================================================================================================================
	// log opengl information for the current implementation
	// ==================================================================================================================================================================================================================
	gl_info();

	init_camera(window);
	glm::mat4 projection_matrix = glm::infinitePerspective(two_pi / 6, float(res_x) / float(res_y), 0.1f); 		        	// projection matrix : 60° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units

	// ==================================================================================================================================================================================================================
	// Shader and uniform initialization
	// ==================================================================================================================================================================================================================

    glsl_program point_lighting(glsl_shader(GL_VERTEX_SHADER,   "glsl/point.vs"),
                                glsl_shader(GL_FRAGMENT_SHADER, "glsl/point.fs"));

    point_lighting.enable();
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	GLuint uniform_projection_view_matrix = point_lighting.uniform_id("projection_view_matrix");						    // projection_view matrix uniform id
	GLuint uniform_shift = point_lighting.uniform_id("shift");								                                // view matrix uniform id
	GLuint point_light_color = point_lighting.uniform_id("light_color");								                        // view matrix uniform id
	GLuint point_lighting_time = point_lighting.uniform_id("global_time");								                    // time uniform id

	// ==================================================================================================================================================================================================================
	// light settings
	// ==================================================================================================================================================================================================================

	glm::vec4 light_color[] =
	{
		glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
		glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
		glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)
	};

	glm::vec4 light_ws[6];

	// ==================================================================================================================================================================================================================
	// Point data initialization 
	// ==================================================================================================================================================================================================================
    
    std::vector<glm::mat4> point_data;
    point_data.reserve(POINT_COUNT);
    for (unsigned int i = 0; i < POINT_COUNT; ++i)
    {
        glm::vec3 axis_z = glm::sphericalRand(1.0f);
        glm::vec3 axis_x = glm::normalize(glm::cross(axis_z, glm::sphericalRand(1.0f)));
        glm::vec3 axis_y = glm::cross(axis_z, axis_x);
        glm::vec3 color  = glm::normalize(glm::vec3(1.0, 1.0, 0.0) + glm::sphericalRand(0.5f));
        point_data.push_back(glm::mat4(glm::vec4(axis_x, 1.0f),
                                       glm::vec4(axis_y, 1.0f),
                                       glm::vec4(axis_z, 1.0f),
                                       glm::vec4(color, glm::gaussRand(0.0f, 1.0f))));
    };
 
    GLuint vao_id, vbo_id;

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, POINT_COUNT * sizeof(glm::mat4), glm::value_ptr(point_data[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 64, (void*)(0));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 64, (void*)(16));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 64, (void*)(32));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 64, (void*)(48));


	// ==================================================================================================================================================================================================================
	// shadow map shader, renders to cube map distance-to-light texture
	// ==================================================================================================================================================================================================================

    glsl_program shadow_map(glsl_shader(GL_VERTEX_SHADER,   "glsl/shadow_map.vs"),
							glsl_shader(GL_GEOMETRY_SHADER, "glsl/shadow_map.gs"),
                            glsl_shader(GL_FRAGMENT_SHADER, "glsl/shadow_map.fs"));

	shadow_map.enable();
    GLint shadow_light_ws			= shadow_map.uniform_id("light_ws");                                                 	// position of the light source 0 in the world space
    GLint shadow_time			 	= shadow_map.uniform_id("global_time");

	// ==================================================================================================================================================================================================================
	// Phong lighting model shader initialization
	// ==================================================================================================================================================================================================================

    glsl_program simple_light(glsl_shader(GL_VERTEX_SHADER,   "glsl/simple_light.vs"),
                              glsl_shader(GL_FRAGMENT_SHADER, "glsl/simple_light.fs"));

	simple_light.enable();

	GLint uniform_projection_matrix = simple_light.uniform_id("projection_matrix");											// projection matrix uniform id
    GLint uniform_light_ws          = simple_light.uniform_id("light_ws");                                                 	// position of the light source 0 in the world space
	GLint uniform_view_matrix       = simple_light.uniform_id("view_matrix");                                               // 
	GLint uniform_time				= simple_light.uniform_id("global_time");                                               // 
	GLint uniform_light_color		= simple_light.uniform_id("light_color");								                // view matrix uniform id

    glUniformMatrix4fv(uniform_projection_matrix, 1, GL_FALSE, glm::value_ptr(projection_matrix));							// projection_matrix
	glUniform4fv(uniform_light_color, 3, glm::value_ptr(light_color[0]));

    const unsigned int size = 6; 
	const float distance = 6.0f;
	const float group_unit = radius;

	// ==================================================================================================================================================================================================================
	// dodecahedron model initialization
	// ==================================================================================================================================================================================================================

    solid dodecahedron(plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::uvs, plato::dodecahedron::mesh_size);
    dodecahedron.texture_id = texture::png("textures/pentagon3.png");
    dodecahedron.normal_texture_id = texture::png("textures/pentagon_normal.png");
    solid_group dodecahedra(size, distance, glm::vec3(-group_unit, 0.0f, 0.0f));

	// ==================================================================================================================================================================================================================
	// tetrahedron model initialization
	// ==================================================================================================================================================================================================================

    solid tetrahedron(plato::tetrahedron::vertices, plato::tetrahedron::normals, plato::tetrahedron::uvs, plato::tetrahedron::mesh_size);
	tetrahedron.texture_id = texture::png("textures/tetrahedron.png");
	tetrahedron.normal_texture_id = texture::png("textures/tetrahedron_normal.png");
    solid_group tetrahedra(size, distance, glm::vec3(group_unit, 0.0f, 0.0f));

	// ==================================================================================================================================================================================================================
	// cube model initialization
	// ==================================================================================================================================================================================================================

    solid cube(plato::cube::vertices, plato::cube::normals, plato::cube::uvs, plato::cube::mesh_size);
    cube.texture_id = texture::png("textures/cube.png");
    cube.normal_texture_id = texture::png("textures/cube_normal.png");
    solid_group cubes(size, distance, glm::vec3(0.0f, -group_unit, 0.0f));

	// ==================================================================================================================================================================================================================
	// icosahedron model initialization
	// ==================================================================================================================================================================================================================

    solid icosahedron(plato::icosahedron::vertices, plato::icosahedron::normals, plato::icosahedron::uvs, plato::icosahedron::mesh_size); 
    icosahedron.texture_id = texture::png("textures/icosahedron.png");
    icosahedron.normal_texture_id = texture::png("textures/icosahedron_normal.png");
    solid_group icosahedra(size, distance, glm::vec3(0.0f, group_unit, 0.0f));

	// ==================================================================================================================================================================================================================
	// octahedron model initialization
	// ==================================================================================================================================================================================================================

    solid octahedron(plato::octahedron::vertices, plato::octahedron::normals, plato::octahedron::uvs, plato::octahedron::mesh_size); 
    octahedron.texture_id = texture::png("textures/octahedron.png");
    octahedron.normal_texture_id = texture::png("textures/octahedron_normal.png");
    solid_group octahedra(size, distance, glm::vec3(0.0f, 0.0f, -group_unit));

	// ==================================================================================================================================================================================================================
	// torus model initialization
	// ==================================================================================================================================================================================================================

	surface torus(torus_func, 64, 32);
    torus.texture_id = texture::png("textures/torus.png");
    torus.normal_texture_id = texture::png("textures/torus_normal.png");
    solid_group tori(size, distance, glm::vec3(0.0f, 0.0f, group_unit));

	// ==================================================================================================================================================================================================================
	// light variables
	// ==================================================================================================================================================================================================================

	shadow_cubemap shadow[3];

	
	while(!glfwWindowShouldClose(window))
	{

		float t = 0.3 * glfwGetTime();
		light_ws[0] = glm::vec4(radius * glm::vec3(cos(t), sin(t),   1.0f), 1.0f);
		light_ws[1] = glm::vec4(radius * glm::vec3(  1.0f, cos(t), sin(t)), 1.0f);
		light_ws[2] = glm::vec4(radius * glm::vec3(sin(t),   1.0f, cos(t)), 1.0f);
		light_ws[3] = glm::vec4(radius * glm::vec3(cos(t), sin(t),  -1.0f), 1.0f);
		light_ws[4] = glm::vec4(radius * glm::vec3( -1.0f, cos(t), sin(t)), 1.0f);
		light_ws[5] = glm::vec4(radius * glm::vec3(sin(t),  -1.0f, cos(t)), 1.0f);


		// ==============================================================================================================================================================================================================
		// light variables
		// ==============================================================================================================================================================================================================

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		shadow_map.enable();
    	glUniform1f(shadow_time, t);

		for (int i = 0; i < 6; ++i)
		{
			shadow[i].bind();

	        glUniform4fv(shadow_light_ws, 1, glm::value_ptr(light_ws[i]));
    	    
			dodecahedra.uniform_bind();
			dodecahedron.depth_texture_render(dodecahedra.instances);
		    
			tetrahedra.uniform_bind();
            tetrahedron.depth_texture_render(tetrahedra.instances);
            
			cubes.uniform_bind();
            cube.depth_texture_render(cubes.instances);
            
			icosahedra.uniform_bind();
            icosahedron.depth_texture_render(icosahedra.instances);
            
			octahedra.uniform_bind();
            octahedron.depth_texture_render(octahedra.instances);
            
			tori.uniform_bind();
            torus.depth_texture_render(tori.instances);
        };

		// ==============================================================================================================================================================================================================
		// final render pass
		// ==============================================================================================================================================================================================================

		simple_light.enable();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);


		shadow[0].bind_texture(GL_TEXTURE2);
		shadow[1].bind_texture(GL_TEXTURE3);
		shadow[2].bind_texture(GL_TEXTURE4);
		shadow[3].bind_texture(GL_TEXTURE5);
		shadow[4].bind_texture(GL_TEXTURE6);
		shadow[5].bind_texture(GL_TEXTURE7);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, res_x, res_y);
		glClearColor(0.01f, 0.0f, 0.04f, 1.0f);																					// dark blue background
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);																	// Clear the screen

	    glUniform4fv(uniform_light_ws, 3, glm::value_ptr(light_ws[0]));
		glUniformMatrix4fv(uniform_view_matrix, 1, GL_FALSE, glm::value_ptr(view_matrix));
    	glUniform1f(uniform_time, t);

		dodecahedra.uniform_bind();
		dodecahedron.instanced_render(dodecahedra.instances);

		tetrahedra.uniform_bind();
        tetrahedron.instanced_render(tetrahedra.instances);

		cubes.uniform_bind();
        cube.instanced_render(cubes.instances);

		icosahedra.uniform_bind();
        icosahedron.instanced_render(icosahedra.instances);

		octahedra.uniform_bind();
        octahedron.instanced_render(octahedra.instances);

		tori.uniform_bind();
        torus.instanced_render(tori.instances);

		// ==============================================================================================================================================================================================================
		// now render light sources
		// ==============================================================================================================================================================================================================


	    point_lighting.enable();


	    glEnable(GL_BLEND);
    	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

    	glBindVertexArray(vao_id);
        glm::mat4 projection_view_matrix = projection_matrix * view_matrix;                                                 // set the projection_view matrix uniform
		glUniformMatrix4fv(uniform_projection_view_matrix, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));		    // 
        glUniform1f(point_lighting_time, t);

        unsigned int index = 0;

		for (int i = 0; i < 6; ++i)
		{
	        glUniform3fv(uniform_shift, 1, glm::value_ptr(light_ws[i]));
	        glUniform4fv(point_light_color, 1, glm::value_ptr(light_color[i]));
            glDrawArrays(GL_POINTS, 0, POINT_COUNT);        
        };

	    glDisable(GL_BLEND);

		glfwSwapBuffers(window);																							// swap buffers
		glfwPollEvents();
	}; 

	glfwTerminate();																										// close OpenGL window and terminate GLFW
	return 0;
}