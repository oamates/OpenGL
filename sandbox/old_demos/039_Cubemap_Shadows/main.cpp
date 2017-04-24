//========================================================================================================================================================================================================================
// DEMO 024: Shadows
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
#include "plato.hpp"
#include "polyhedron.hpp"
#include "torus.hpp"

//========================================================================================================================================================================================================================
// 3d moving camera : standard initial orientation in space
//========================================================================================================================================================================================================================
const double linear_velocity = 0.01f;
const double angular_rate = 0.0001f;
static camera3d camera;

//========================================================================================================================================================================================================================
// keyboard and mouse handlers
//========================================================================================================================================================================================================================

void keyboard_handler(int key, int scancode, int action, int mods)
{
    if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(linear_velocity);  
    else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(linear_velocity); 
    else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(linear_velocity);
    else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(linear_velocity); 
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

//=======================================================================================================================================================================================================================
// Standard parameterization of the torus of rotation
//=======================================================================================================================================================================================================================
vertex_pnt2 torus3d_func (const glm::vec2& uv)
{
	vertex_pnt2 vertex;

	const double R = 1.5;
	const double r = 0.6;
	double cos_2piu = glm::cos(constants::two_pi_d * uv.x);
	double sin_2piu = glm::sin(constants::two_pi_d * uv.x);
	double cos_2piv = glm::cos(constants::two_pi_d * uv.y);
	double sin_2piv = glm::sin(constants::two_pi_d * uv.y);

	vertex.position = glm::vec3((R + r * cos_2piv) * cos_2piu, (R + r * cos_2piv) * sin_2piu, r * sin_2piv);
	vertex.normal = glm::vec3(cos_2piv * cos_2piu, cos_2piv * sin_2piu, sin_2piv);
	vertex.uv = uv;
	return vertex;
};	

struct motion3d_t
{
	glm::vec4 shift;
	glm::vec4 rotor;
};

//=======================================================================================================================================================================================================================
// function that initializes initial model matrices and object rotation axes
//=======================================================================================================================================================================================================================
void fill_shift_rotor_data(motion3d_t* data, const glm::vec3& group_shift, float cell_size, int N)
{
	float middle = 0.5f * float(N) - 0.5f;
    int index = 0;
	for (int i = 0; i < N; ++i) for (int j = 0; j < N; ++j) for (int k = 0; k < N; ++k)
	{
		data[index].shift = glm::vec4(group_shift + cell_size * glm::vec3(float(i) - middle, float(j) - middle, float(k) - middle), 0.0f);
		data[index].rotor = glm::vec4(glm::sphericalRand(1.0f), 4.0f * glm::gaussRand(0.0f, 1.0f));
        index++;
	};
};

int main()
{
    //===================================================================================================================================================================================================================
    // GLFW window creation + GLEW library initialization
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    glfw_window window("Shadows", 8, 3, 3, 1920, 1080);
    window.log_info();
    window.mouse_handler = mouse_handler;
    window.keyboard_handler = keyboard_handler;
    camera.infinite_perspective(constants::two_pi / 6.0f, window.aspect_ratio(), 0.1f);

	//===================================================================================================================================================================================================================
	// shadow map shader, renders to cube map distance-to-light texture
	//===================================================================================================================================================================================================================

    glsl_program shadow_map(glsl_shader(GL_VERTEX_SHADER,   "glsl/shadow_map.vs"),
							glsl_shader(GL_GEOMETRY_SHADER, "glsl/shadow_map.gs"),
                            glsl_shader(GL_FRAGMENT_SHADER, "glsl/shadow_map.fs"));

	shadow_map.enable();
    GLint shadow_light_ws			 = shadow_map.uniform_id("light_ws");                                                 	// position of the light source 0 in the world space
    GLint shadow_buffer_base         = shadow_map.uniform_id("buffer_base");
    GLint shadow_time				 = shadow_map.uniform_id("time");

	//===================================================================================================================================================================================================================
	// Phong lighting model shader initialization
	//===================================================================================================================================================================================================================

    glsl_program simple_light(glsl_shader(GL_VERTEX_SHADER,   "glsl/simple_light.vs"),
                              glsl_shader(GL_FRAGMENT_SHADER, "glsl/simple_light.fs"));

	simple_light.enable();

	GLint uniform_projection_matrix = simple_light.uniform_id("projection_matrix");											// projection matrix uniform id
    GLint uniform_light_ws          = simple_light.uniform_id("light_ws");                                                 	// position of the light source 0 in the world space
	GLint uniform_view_matrix       = simple_light.uniform_id("view_matrix");                                               // position of the light source 0 in the world space
    GLint uniform_base              = simple_light.uniform_id("buffer_base");
    GLint uniform_diffuse_texture   = simple_light.uniform_id("diffuse_texture");
    GLint uniform_normal_texture    = simple_light.uniform_id("normal_texture");
    GLint uniform_time				= simple_light.uniform_id("time");

    glUniformMatrix4fv(uniform_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera.projection_matrix));							// projection_matrix

    const unsigned int size = 5; 
	const float distance = 5.5f;
	const float angular_rate = 0.01f;
	const float group_unit = 60.0f;

	//===================================================================================================================================================================================================================
	// polyhedra + torus models initialization
	//===================================================================================================================================================================================================================
    polyhedron tetrahedron, cube, octahedron, dodecahedron, icosahedron;

    tetrahedron.regular_pft2_vao(4, 4, plato::tetrahedron::vertices, plato::tetrahedron::normals, plato::tetrahedron::faces);
    cube.regular_pft2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces);
    octahedron.regular_pft2_vao(6, 8, plato::octahedron::vertices, plato::octahedron::normals, plato::octahedron::faces);
    dodecahedron.regular_pft2_vao(20, 12, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);
    icosahedron.regular_pft2_vao(12, 20, plato::icosahedron::vertices, plato::icosahedron::normals, plato::icosahedron::faces);

    glActiveTexture(GL_TEXTURE0);
	GLuint tetrahedron_diffuse_texture_id = texture::texture2d_png("res/tetrahedron.png");
    glActiveTexture(GL_TEXTURE1);
	GLuint tetrahedron_normal_texture_id = texture::texture2d_png("res/tetrahedron_bump.png");
    glActiveTexture(GL_TEXTURE2);
    GLuint cube_diffuse_texture_id = texture::texture2d_png("res/cube.png");
    glActiveTexture(GL_TEXTURE3);
    GLuint cube_normal_texture_id = texture::texture2d_png("res/cube_bump.png");
    glActiveTexture(GL_TEXTURE4);
    GLuint octahedron_diffuse_texture_id = texture::texture2d_png("res/octahedron.png");
    glActiveTexture(GL_TEXTURE5);
    GLuint octahedron_normal_texture_id = texture::texture2d_png("res/octahedron_bump.png");
    glActiveTexture(GL_TEXTURE6);
    GLuint dodecahedron_diffuse_texture_id = texture::texture2d_png("res/pentagon.png");
    glActiveTexture(GL_TEXTURE7);
    GLuint dodecahedron_normal_texture_id = texture::texture2d_png("res/pentagon_bump.png");
    glActiveTexture(GL_TEXTURE8);
    GLuint icosahedron_diffuse_texture_id = texture::texture2d_png("res/icosahedron.png");
    glActiveTexture(GL_TEXTURE9);
    GLuint icosahedron_normal_texture_id = texture::texture2d_png("res/icosahedron_bump.png");


    const int N = 16;
    const int group_size = N * N * N;
	const float cell_size = 6.0f;
	const float origin_distance = 1.25f * cell_size * N;
	const GLsizeiptr chunk_size = group_size * sizeof(motion3d_t);	

	motion3d_t data[5 * group_size];

	fill_shift_rotor_data(&data[0 * group_size], glm::vec3(            0.0f,             0.0f,  origin_distance), cell_size, N);
	fill_shift_rotor_data(&data[1 * group_size], glm::vec3(            0.0f,             0.0f, -origin_distance), cell_size, N);
	fill_shift_rotor_data(&data[2 * group_size], glm::vec3(            0.0f,  origin_distance,             0.0f), cell_size, N);
	fill_shift_rotor_data(&data[3 * group_size], glm::vec3(            0.0f, -origin_distance,             0.0f), cell_size, N);
	fill_shift_rotor_data(&data[4 * group_size], glm::vec3(            0.0f,             0.0f,             0.0f), cell_size, N);

	fbo_depth_cubemap shadow_cubemap(2048);

	glClearColor(0.01f, 0.00f, 0.06f, 1.00f);																				// dark blue background
	glm::vec4 light_ws = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
    while(!window.should_close())
    {
    	float time = window.time();
		//===============================================================================================================================================================================================================
		// light variables
		//===============================================================================================================================================================================================================
/*
		glEnable(GL_CULL_FACE);
		shadow_cubemap.bind();
		shadow_map.enable();

	    glUniform4fv(shadow_light_ws, 1, glm::value_ptr(light_ws));
	    glUniform1f(shadow_time, time);

		//===============================================================================================================================================================================================================
		// render pass to get cubemap z-buffer filled w.r.t current light position
		//===============================================================================================================================================================================================================
        glUniform1i(shadow_buffer_base, 0 * group_size);
		tetrahedron.instanced_render(group_size);
        glUniform1i(shadow_buffer_base, 1 * group_size);
		cube.instanced_render(group_size);
        glUniform1i(shadow_buffer_base, 2 * group_size);
		octahedron.instanced_render(group_size);
        glUniform1i(shadow_buffer_base, 3 * group_size);
		dodecahedron.instanced_render(group_size);
        glUniform1i(shadow_buffer_base, 4 * group_size);
		icosahedron.instanced_render(group_size);

		//===============================================================================================================================================================================================================
		// on-screen render pass using created depth texture
		//===============================================================================================================================================================================================================
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_DEPTH_TEST);
		shadow_cubemap.bind_texture(GL_TEXTURE10);
*/
		glClearColor(0.00f, 0.00f, 0.00f, 1.00f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		simple_light.enable();

	    glUniform4fv(uniform_light_ws, 1, glm::value_ptr(light_ws));
	    glUniform1f(uniform_time, time);
		glUniformMatrix4fv(uniform_view_matrix, 1, GL_FALSE, glm::value_ptr(camera.view_matrix));

        glUniform1i(uniform_base, 0 * group_size);
        glUniform1i(uniform_diffuse_texture, 0);
        glUniform1i(uniform_normal_texture, 1);
		tetrahedron.instanced_render(group_size);

        glUniform1i(uniform_base, 1 * group_size);
        glUniform1i(uniform_diffuse_texture, 2);
        glUniform1i(uniform_normal_texture, 3);
		cube.instanced_render(group_size);

        glUniform1i(uniform_base, 2 * group_size);
        glUniform1i(uniform_diffuse_texture, 4);
        glUniform1i(uniform_normal_texture, 5);
		octahedron.instanced_render(group_size);

        glUniform1i(uniform_base, 3 * group_size);
        glUniform1i(uniform_diffuse_texture, 6);
        glUniform1i(uniform_normal_texture, 7);
		dodecahedron.instanced_render(group_size);

        glUniform1i(uniform_base, 4 * group_size);
        glUniform1i(uniform_diffuse_texture, 8);
        glUniform1i(uniform_normal_texture, 9);
		icosahedron.instanced_render(group_size);

        window.swap_buffers();
        window.poll_events();
    };
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    return 0;
};