//==============================================================================================================================================================================================
// DEMO 025: HDR Effect Shader
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"

GLuint quadVAO = 0;
GLuint quadVBO = 0;
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;


void RenderQuad()
{
	if (quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void RenderCube()
{
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
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
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
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("HDR Effect Shader", 4, 3, 3, 1920, 1080, true);

    glViewport(0, 0, window.res_x, window.res_y);

    //===================================================================================================================================================================================================================
    // Setup some OpenGL options
    //===================================================================================================================================================================================================================
    glEnable(GL_DEPTH_TEST);


	glsl_program_t shader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/lighting.vs"), 
						  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/lighting.fs"));

	glsl_program_t hdrShader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/hdr.vs"), 
		                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/hdr.fs"));

    //===================================================================================================================================================================================================================
    // Light sources positions and colors
    //===================================================================================================================================================================================================================
    glm::vec3 lightPositions[4] = 
    {
        glm::vec3( 0.0f,  0.0f, 49.5f), 
        glm::vec3(-1.4f, -1.9f,  9.0f),
        glm::vec3( 0.0f, -1.8f,  4.0f), 
        glm::vec3( 0.8f, -1.7f,  6.0f)
    }; 

    glm::vec3 lightColors[4] = 
    {
        glm::vec3(200.0f, 200.0f, 200.0f),
        glm::vec3(  0.1f,   0.0f,   0.0f),
        glm::vec3(  0.0f,   0.0f,   0.2f),
        glm::vec3(  0.0f,   0.1f,   0.0f)
    };

    //===================================================================================================================================================================================================================
    // Load textures
    //===================================================================================================================================================================================================================
    GLuint woodTexture = image::png::texture2d("../../../resources/tex2d/wood.png");

    //===================================================================================================================================================================================================================
    // Set up floating point framebuffer to render scene to
    //===================================================================================================================================================================================================================
    GLuint hdrFBO;
    glGenFramebuffers(1, &hdrFBO);

    GLuint colorBuffer;
    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window.res_x, window.res_y);

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	GLuint attachment = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &attachment);
	glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        exit_msg("Framebuffer not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while (!window.should_close())
    {
        window.new_frame();
        glm::vec3 camera_ws = window.camera.position();

        // 1. Render scene into floating point framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glm::mat4 projection = window.camera.projection_matrix;
            glm::mat4 view       = window.camera.view_matrix;
            glm::mat4 model;
            shader.enable();
            shader["projection_matrix"] = projection;
            shader["view_matrix"] = view;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, woodTexture);
            shader["diffuse_texture"] = 0;

			glUniform3fv(shader["light_ws"], 4, glm::value_ptr(lightPositions[0]));
			glUniform3fv(shader["light_color"],    4, glm::value_ptr(lightColors[0]));
            shader["camera_ws"] = camera_ws;

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 25.0));
            model = glm::scale(model, glm::vec3(5.0f, 5.0f, 55.0f));
            shader["model_matrix"] = model;
            shader["inverse_normals"] = 1;
            RenderCube();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);      

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        
        hdrShader.enable();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffer);

        hdrShader["hdrBuffer"] = 0;
        RenderQuad();       

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}