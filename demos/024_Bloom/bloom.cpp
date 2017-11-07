//========================================================================================================================================================================================================================
// DEMO 024: Bloom Effect
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"

GLfloat exposure = 1.0f;
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true /*, true */)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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
		
		if (action != GLFW_RELEASE) return;
		if (key == GLFW_KEY_Q) exposure -= 0.3;
		if (key == GLFW_KEY_E) exposure += 0.3;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

void RenderCube()
{
	// Initialize (if necessary)
	if (cubeVAO == 0)
	{
		GLfloat vertices[] = 
		{
			// Back face
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f, // Bottom-left
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f,  -1.0f,  1.0f,  1.0f, // top-right
			 0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  -1.0f,  1.0f,  0.0f, // bottom-right         
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f,  -1.0f,  1.0f,  1.0f, // top-right
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f, // bottom-left
			-0.5f,  0.5f, -0.5f,  0.0f,  0.0f,  -1.0f,  0.0f,  1.0f, // top-left
			// Front face                                            
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f, // bottom-left
			 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,   1.0f,  1.0f,  0.0f, // bottom-right
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,   1.0f,  1.0f,  1.0f, // top-right
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,   1.0f,  1.0f,  1.0f, // top-right
			-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,   1.0f,  0.0f,  1.0f, // top-left
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f, // bottom-left
			// Left face                                       
			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,   0.0f,  1.0f,  0.0f, // top-right
			-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,   0.0f,  1.0f,  1.0f, // top-left
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,   0.0f,  0.0f,  1.0f, // bottom-left
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,   0.0f,  0.0f,  1.0f, // bottom-left
			-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,   0.0f,  0.0f,  0.0f, // bottom-right
			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,   0.0f,  1.0f,  0.0f, // top-right
			// Right face                                      
			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f, // top-left
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,   0.0f,  0.0f,  1.0f, // bottom-right
			 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,   0.0f,  1.0f,  1.0f, // top-right         
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,   0.0f,  0.0f,  1.0f, // bottom-right
			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f, // top-left
			 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,   0.0f,  0.0f,  0.0f, // bottom-left     
			// Bottom face                                     
			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,   0.0f,  0.0f,  1.0f, // top-right
			 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,   0.0f,  1.0f,  1.0f, // top-left
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f, // bottom-left
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f, // bottom-left
			-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,   0.0f,  0.0f,  0.0f, // bottom-right
			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,   0.0f,  0.0f,  1.0f, // top-right
			// Top face
			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f, // top-left
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,   0.0f,  1.0f,  0.0f, // bottom-right
			 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,   0.0f,  1.0f,  1.0f, // top-right     
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,   0.0f,  1.0f,  0.0f, // bottom-right
			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f, // top-left
			-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,   0.0f,  0.0f,  0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// Fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// Link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// Render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Bloom Effect", 4, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // compile shaders and load static uniforms
    //===================================================================================================================================================================================================================
	glsl_program_t shader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/bloom.vs"), 
						  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/bloom.fs"));
	shader.enable();
	shader["diffuse_texture"] = 0;
	shader["projection_matrix"] = window.camera.projection_matrix;

	glsl_program_t shaderLight(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/bloom.vs"), 
		                       glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/light_box.fs"));
    shaderLight.enable();
	shaderLight["projection_matrix"] = window.camera.projection_matrix;

	glsl_program_t shaderBlur(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/blur.vs"), 
					          glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blur.fs"));
    shaderBlur.enable();
	shaderBlur["image"] = 0;

	glsl_program_t shaderBloomFinal(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/bloom_final.vs"), 
		                            glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/bloom_final.fs"));
	shaderBloomFinal.enable();
	shaderBloomFinal["scene"] = 0;
	shaderBloomFinal["bloomBlur"] = 1;

    //===================================================================================================================================================================================================================
    // light sources worldspace positions and colors
    //===================================================================================================================================================================================================================
	glm::vec3 light_ws[4] = 
    {
	    glm::vec3( 0.0f, 0.5f,  1.5f),  
	    glm::vec3(-4.0f, 0.5f, -3.0f),
	    glm::vec3( 3.0f, 0.5f,  1.0f),  
	    glm::vec3(-0.8f, 2.4f, -1.0f)
    };
 
	glm::vec3 light_colors[4] = 
    {
	    glm::vec3(5.0f, 5.0f,  5.0f), 
	    glm::vec3(5.5f, 0.0f,  0.0f), 
	    glm::vec3(0.0f, 0.0f, 15.0f),
	    glm::vec3(0.0f, 1.5f,  0.0f)
    };

    //===================================================================================================================================================================================================================
    // load textures
    //===================================================================================================================================================================================================================
	GLuint wood_texture_id      = image::png::texture2d("../../../resources/tex2d/wood.png");
	GLuint container_texture_id = image::png::texture2d("../../../resources/tex2d/container.png");

	// Set up floating point framebuffer to render scene to
	GLuint hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// - Create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)
	GLuint colorBuffers[2];
	glGenTextures(2, colorBuffers);
	for (GLuint i = 0; i < 2; i++) 
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F,  2 * window.res_x, 2 * window.res_y);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colorBuffers[i], 0);
	}

	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 2 * window.res_x, 2 * window.res_y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, attachments);
    glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		debug_msg("Framebuffer not complete!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //===================================================================================================================================================================================================================
    // ping-pong framebuffer for blurring
    //===================================================================================================================================================================================================================
	GLuint pingpongFBO[2];
	GLuint pingpongColorbuffers[2];
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (GLuint i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F,  2 * window.res_x, 2 * window.res_y);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // We clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pingpongColorbuffers[i], 0);		
		// Also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			debug_msg("Framebuffer not complete!");
	}

	glm::mat4 m0 = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0)), glm::vec3(25.0f, 1.0f, 25.0f));
    glm::mat4 m1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 0.0));
    glm::mat4 m2 = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 1.0));
    glm::mat4 m3 = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f, 2.0)), 60.0f, glm::normalize(glm::vec3(1.0, 0.0, 1.0))), glm::vec3(2.0));
    glm::mat4 m4 = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.7f, 4.0)), 23.0f, glm::normalize(glm::vec3(1.0, 0.0, 1.0))), glm::vec3(2.5));
    glm::mat4 m5 = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 1.0f, -3.0)), 124.0f, glm::normalize(glm::vec3(1.0, 0.0, 1.0))), glm::vec3(2.0));
    glm::mat4 m6 = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 0.0));

    glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    GLuint quad_vao_id;                                                                         // dummy, but necessary VAO 
    glGenVertexArrays(1, &quad_vao_id);
    //===================================================================================================================================================================================================================
    // the main loop
    //===================================================================================================================================================================================================================
	while (!window.should_close())
	{
        window.new_frame();
        glm::vec3 camera_ws = window.camera.position();
		glm::mat4 view_matrix = window.camera.view_matrix;

        //===============================================================================================================================================================================================================
        // 1. Render scene into floating point framebuffer
        //===============================================================================================================================================================================================================
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, 2 * window.res_x, 2 * window.res_y);

		shader.enable();
		shader["view_matrix"] = view_matrix;
        shader["light_ws"] = light_ws;
        shader["light_color"] = light_colors;
		shader["camera_ws"] = camera_ws;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wood_texture_id);                                          // render wooden floor
		shader["model_matrix"] = m0; RenderCube();

		glBindTexture(GL_TEXTURE_2D, container_texture_id);                                     // render 6 random container boxes
		shader["model_matrix"] = m1; RenderCube();
		shader["model_matrix"] = m2; RenderCube();
		shader["model_matrix"] = m3; RenderCube();
		shader["model_matrix"] = m4; RenderCube();
		shader["model_matrix"] = m5; RenderCube();
		shader["model_matrix"] = m6; RenderCube();

        //===============================================================================================================================================================================================================
        // 2. Show all the light sources as bright cubes
        //===============================================================================================================================================================================================================
		shaderLight.enable();
		shaderLight["view_matrix"] = view_matrix;

		for (GLuint i = 0; i < 4; i++)
		{
			glm::mat4 model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(light_ws[i])), glm::vec3(0.5f));
			shaderLight["model_matrix"] = model_matrix;
			shaderLight["light_color"] = light_colors[i];
			RenderCube();
		}

        //===============================================================================================================================================================================================================
        // 3. Blur bright fragments with two-pass Gaussian Blur 
        //===============================================================================================================================================================================================================
        glBindVertexArray(quad_vao_id);
		GLboolean horizontal = true, first_iteration = true;
		GLuint amount = 4;
		shaderBlur.enable();
		for (GLuint i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]); 
			shaderBlur["horizontal"] = horizontal;
			glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
        	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}

        //===============================================================================================================================================================================================================
        // 4. Render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's clamped color range
        //===============================================================================================================================================================================================================
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window.res_x, window.res_y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderBloomFinal.enable();
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
		shaderBloomFinal["exposure"] = exposure;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}                           