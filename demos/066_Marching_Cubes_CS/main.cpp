//========================================================================================================================================================================================================================
// DEMO 066 : Marching Cubes Compute Shader
//========================================================================================================================================================================================================================
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
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"
#include "camera.hpp"

const float two_pi = 6.283185307179586476925286766559; 

const unsigned int TEXTURE_SIZE = 32;

const unsigned int res_x = 1920;    
const unsigned int res_y = 1080;

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 4.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Marching cubes via GL_COMPUTE_SHADER", 4, 4, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // compile shaders and load static uniforms
    //===================================================================================================================================================================================================================
    glsl_program_t isosurface(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/isosurface.vs"),
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/isosurface.fs"));

    isosurface.enable();
	uniform_t uni_iso_camera_matrix = isosurface["camera_matrix"];                                         

    //===================================================================================================================================================================================================================
	// Density compute shader
    //===================================================================================================================================================================================================================
    glsl_program_t density_compute(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/density_compute.cs"));
    density_compute.enable();
	uniform_t uni_dc_camera_matrix = density_compute["camera_matrix"];

    //===================================================================================================================================================================================================================
	// Marching cubes compute shader
    //===================================================================================================================================================================================================================
	glsl_program_t marching_cubes(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/marching_cubes.cs"));
    marching_cubes.enable();
	uniform_t uniform_mc_camera_matrix = marching_cubes["camera_matrix"];

    //===================================================================================================================================================================================================================
	// Create image texture to be used for the GL_COMPUTE_SHADER output
    //===================================================================================================================================================================================================================
	GLuint density_texture_id;
	glGenTextures(1, &density_texture_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, density_texture_id);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, TEXTURE_SIZE, TEXTURE_SIZE, TEXTURE_SIZE);
	glBindImageTexture(0, density_texture_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);						// Note : GL_TRUE is necessary as GL_TEXTURE_3D is layered

	GLuint vertices_ssbo_id;
	glGenBuffers(1, &vertices_ssbo_id);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertices_ssbo_id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 400000 * sizeof(glm::vec4), 0, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertices_ssbo_id);

	GLuint triangles_acbo_id;
	glGenBuffers(1, &triangles_acbo_id);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, triangles_acbo_id);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), 0, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, triangles_acbo_id);
    GLuint* data = (GLuint*) glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_WRITE_ONLY);
    *data = 0;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

    //===================================================================================================================================================================================================================
	// Create fake VAO with no attribute buffers, all the input data will come from GL_SHADER_STORAGE_BUFFER
    //===================================================================================================================================================================================================================
	GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
    double startup_ts = window.frame_ts;

	while(!window.should_close())
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);																    // clear the screen
        window.new_frame();

		float time = window.frame_ts;
		glm::mat4 camera_matrix = glm::inverse(window.camera.view_matrix); 

		//debug_msg("Current camera matrix = %s", glm::to_string(camera_matrix).c_str());

		//===============================================================================================================================================================================================================
		// Compute density texture3D
		//===============================================================================================================================================================================================================
		density_compute.enable();
		uni_dc_camera_matrix = camera_matrix;
		glDispatchCompute(TEXTURE_SIZE / 4, TEXTURE_SIZE / 4, TEXTURE_SIZE / 4);

		//===============================================================================================================================================================================================================
		// Marching cubes compute shader
		//===============================================================================================================================================================================================================
		marching_cubes.enable();
		//uniform_inverse_view_matrix_id = camera_matrix;
		glDispatchCompute(TEXTURE_SIZE / 4, TEXTURE_SIZE / 4, TEXTURE_SIZE / 4);

		//===============================================================================================================================================================================================================
		// Read how many triangles were produced, and zero the counter for next iteration
		//===============================================================================================================================================================================================================
        GLuint* data = (GLuint*) glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_WRITE);
        unsigned int triangles_count = *data;
        *data = 0;
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

		debug_msg("Computation #%u done. FPS = %f. time = %f. %u vertices were generated by GL_COMPUTE_SHADER.", window.frame, double(window.frame) / (time - startup_ts), time, triangles_count);

		//===============================================================================================================================================================================================================
		// Finally, render the mesh constructed by marching cubes compute shader
		//===============================================================================================================================================================================================================
		isosurface.enable();
		uni_iso_camera_matrix = camera_matrix;
		glDrawArrays(GL_TRIANGLES, 0, 3 * triangles_count);

		// ==============================================================================================================================================================================================================
		// Done.
		// ==============================================================================================================================================================================================================
        window.end_frame();
	}
    
	glfw::terminate();
	return 0;
}




















