//==============================================================================================================================================================================================
// DEMO 029: SSAO Effect Shader
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <random>

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
#include "vao.hpp"


GLuint draw_mode = 1;

GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
{
    return a + f * (b - a);
}

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true /*, true */)
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
		
		if (action != GLFW_RELEASE) return;

        if (key = GLFW_KEY_1) draw_mode = 1;
        if (key = GLFW_KEY_2) draw_mode = 2;
        if (key = GLFW_KEY_3) draw_mode = 3;
        if (key = GLFW_KEY_4) draw_mode = 4;

    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

GLuint cubeVAO = 0;
GLuint cubeVBO = 0;

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

    demo_window_t window("SSAO Effect Shader", 4, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // generate SSAO sample kernel points
    //===================================================================================================================================================================================================================
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    glm::vec3 ssaoKernel[64];
    for (GLuint i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        GLfloat scale = GLfloat(i) / 64.0;

		// Scale samples s.t. they're more aligned to center of kernel
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel[i] = sample;
    }

    //===================================================================================================================================================================================================================
    // compile shaders and load static uniforms
    //===================================================================================================================================================================================================================
    glsl_program_t shaderGeometryPass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao_geometry.vs"), 
                                      glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_geometry.fs"));
    shaderGeometryPass.enable();
    shaderGeometryPass["projection_matrix"] = window.camera.projection_matrix;

    glsl_program_t shaderLightingPass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao.vs"), 
                                      glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_lighting.fs"));
    shaderLightingPass.enable();
    shaderLightingPass["gPosition"] = 0;
    shaderLightingPass["gNormal"] = 1;
    shaderLightingPass["gAlbedo"] = 2; 
    shaderLightingPass["ssao"] = 3; 
    shaderLightingPass["light_color"] = glm::vec3(0.2, 0.2, 0.7);

    glsl_program_t shaderSSAO(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao.vs"), 
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao.fs"));
    shaderSSAO.enable();
    shaderSSAO["gPosition"] = 0;
    shaderSSAO["gNormal"] = 1;
    shaderSSAO["texNoise"] = 2;
    shaderSSAO["projection_matrix"] = window.camera.projection_matrix;
    shaderSSAO["samples"] = ssaoKernel;

    glsl_program_t shaderSSAOBlur(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao.vs"), 
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_blur.fs"));

    // "../../../resources/models/vao/ashtray.vao";
    // "../../../resources/models/vao/azog.vao",
    // "../../../resources/models/vao/bust.vao",
    // "../../../resources/models/vao/chubby_girl.vao",
    // "../../../resources/models/vao/demon.vao",    
    // "../../../resources/models/vao/dragon.vao",   
    // "../../../resources/models/vao/female_01.vao",
    // "../../../resources/models/vao/female_02.vao",
    // "../../../resources/models/vao/female_03.vao",
    // "../../../resources/models/vao/king_kong.vao",
    // "../../../resources/models/vao/predator.vao", 
    // "../../../resources/models/vao/skull.vao",    
    // "../../../resources/models/vao/trefoil.vao"     */


    vao_t model;
    model.init("../../../resources/models/vao/demon.vao");
    debug_msg("VAO Loaded :: \n\tvertex_count = %d. \n\tvertex_layout = %d. \n\tindex_type = %d. \n\tprimitive_mode = %d. \n\tindex_count = %d\n\n\n", 
              model.vbo.size, model.vbo.layout, model.ibo.type, model.ibo.mode, model.ibo.size);
    debug_msg("Done ... \nGL_UNSIGNED_INT = %d.\nGL_TRIANGLES = %d", GL_UNSIGNED_INT, GL_TRIANGLES);

    GLuint gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    GLuint gPosition, gNormal, gAlbedo;

    //===================================================================================================================================================================================================================
    // position buffer
    //===================================================================================================================================================================================================================
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gPosition, 0);

    //===================================================================================================================================================================================================================
    // normal color buffer
    //===================================================================================================================================================================================================================
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, gNormal, 0);

    //===================================================================================================================================================================================================================
    // albedo color buffer
    //===================================================================================================================================================================================================================
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, gAlbedo, 0);

    GLuint attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window.res_x, window.res_y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        exit_msg("GBuffer Framebuffer not complete!");
    //
    //===================================================================================================================================================================================================================
    // framebuffer to hold SSAO processing stage 
    //===================================================================================================================================================================================================================
    GLuint ssaoFBO, ssaoBlurFBO;
    glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    GLuint ssaoColorBuffer, ssaoColorBufferBlur;

    //===================================================================================================================================================================================================================
    // SSAO color buffer
    //===================================================================================================================================================================================================================
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        exit_msg("SSAO Framebuffer not complete!");

    //===================================================================================================================================================================================================================
    // and blur stage
    //===================================================================================================================================================================================================================
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        exit_msg("SSAO Blur Framebuffer not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //===================================================================================================================================================================================================================
    // noise texture
    //===================================================================================================================================================================================================================
    glm::vec3 ssaoNoise[16];
    for (GLuint i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise[i] = noise;
    }

    GLuint noiseTexture; 
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glm::mat4 m0 = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3( 10.0f,  0.0f,   0.0f)), glm::vec3( 1.0f, 20.0f, 20.0f));
    glm::mat4 m1 = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f,  0.0f,   0.0f)), glm::vec3( 1.0f, 20.0f, 20.0f));
    glm::mat4 m2 = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(  0.0f,  0.0f,  10.0f)), glm::vec3(20.0f, 20.0f,  1.0f));
    glm::mat4 m3 = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(  0.0f,  0.0f, -10.0f)), glm::vec3(20.0f, 20.0f,  1.0f));
    glm::mat4 m4 = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(  0.0f, 10.0f,   0.0f)), glm::vec3(20.0f,  1.0f, 20.0f));
    glm::mat4 m5 = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(  0.0f,-10.0f,   0.0f)), glm::vec3(20.0f,  1.0f, 20.0f));

    glm::vec3 light_ws = glm::vec3(2.0, 4.0, -2.0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    GLuint quad_vao_id;
	glGenVertexArrays(1, &quad_vao_id);
    glViewport(0, 0, window.res_x, window.res_y);
    glEnable(GL_DEPTH_TEST);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while (!window.should_close())
    {
        window.new_frame();
        //===============================================================================================================================================================================================================
        // 1. Geometry Pass: render scene's geometry/color data into gbuffer
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 view_matrix = window.camera.view_matrix;
        shaderGeometryPass.enable();
        shaderGeometryPass["view_matrix"] = view_matrix;

        shaderGeometryPass["model_matrix"] = m0; RenderCube();
        shaderGeometryPass["model_matrix"] = m1; RenderCube();
        shaderGeometryPass["model_matrix"] = m2; RenderCube();
        shaderGeometryPass["model_matrix"] = m3; RenderCube();
        shaderGeometryPass["model_matrix"] = m4; RenderCube();
        shaderGeometryPass["model_matrix"] = m5; RenderCube();

        glm::mat4 model_matrix(1.0f);
        model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0));
        model_matrix = glm::rotate(model_matrix, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
        model_matrix = glm::scale(model_matrix, glm::vec3(0.5f));
        shaderGeometryPass["model_matrix"] = model_matrix;
        model.render();

        //===============================================================================================================================================================================================================
        // 2. Create SSAO texture
        //===============================================================================================================================================================================================================
	    glBindVertexArray(quad_vao_id);
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        shaderSSAO.enable();
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, noiseTexture);
	    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // 3. Blur SSAO texture to remove noise
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        shaderSSAOBlur.enable();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // 4. Lighting Pass: traditional deferred Blinn-Phong lighting now with added screen-space ambient occlusion
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderLightingPass.enable();
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, gAlbedo);
        glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);

        glm::vec3 light_cs = window.camera.view_matrix * glm::vec4(light_ws, 1.0);
        shaderLightingPass["light_cs"] = light_cs;
        shaderLightingPass["draw_mode"] = (GLint) draw_mode;
	    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}