#include <iostream>
#include <string>
#include <vector>

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
#include "texture.hpp"

#include "model.hpp"
#include "util.hpp"

glsl_program shader1, shadow1, shader2, shadow2;


const unsigned int res_x = 1920;
const unsigned int res_y = 1080;

// Load up the models being drawn in the scene and return them in the vector passed
std::vector<Model*> setupModels(const glm::mat4 &view, const glm::mat4 &proj);

// Setup the depth buffer for the shadow map pass and return the texture and framebuffer in the params passed. The texture will be active in GL_TEXTURE3
void setupShadowMap(GLuint &fbo, GLuint &tex);

// Perform the shadow map rendering pass
void renderShadowMap(GLuint &fbo, const std::vector<Model*> &models);

const float velocity = 0.7f;
const float spheric_velocity = 0.0001f;
const float two_pi = 6.283185307179586476925286766559;

GLFWwindow* window;
glm::mat4 view_matrix = glm::mat4(1.0f);
double time_stamp;
double mouse_x = 0.0, mouse_y = 0.0;
bool position_changed = false;

void onMouseMove (GLFWwindow*, double, double);
void onKeypressed (GLFWwindow*, int, int, int, int);

void init_camera(GLFWwindow* w) 
{
	window = w;
	time_stamp = glfwGetTime();
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, 0, 0);
	glfwSetKeyCallback(window, onKeypressed);
	glfwSetCursorPosCallback(window, onMouseMove);		
};

void onMouseMove (GLFWwindow * window, double x, double y)
{
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
	};
};

void onKeypressed (GLFWwindow*, int key, int scancode, int action, int mods)
{
	glm::mat4 translation = glm::mat4 (1.0);
	if ((key == GLFW_KEY_UP) || (key == GLFW_KEY_W))         { translation[3][2] =  velocity; position_changed = true; }
	else if ((key == GLFW_KEY_DOWN) || (key == GLFW_KEY_S))  { translation[3][2] = -velocity; position_changed = true; }
	else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) { translation[3][0] = -velocity; position_changed = true; }
	else if ((key == GLFW_KEY_LEFT) || (key == GLFW_KEY_A))  { translation[3][0] =  velocity; position_changed = true; }
    if (position_changed) view_matrix = translation * view_matrix;
/*
	switch (key)
	{
		case SDLK_f:
			printFps = !printFps;
			break;
		case SDLK_a:
			models.at(0)->translate(frameTime * glm::vec3(-2.f, 0.f, 0.f));
			break;
		case SDLK_d:
			models.at(0)->translate(frameTime * glm::vec3(2.f, 0.f, 0.f));
			break;
		case SDLK_w:
			models.at(0)->translate(frameTime * glm::vec3(0.f, 2.f, 0.f));
			break;
		case SDLK_s:
			models.at(0)->translate(frameTime * glm::vec3(0.f, -2.f, 0.f));
			break;
		case SDLK_z:
			models.at(0)->translate(frameTime * glm::vec3(0.f, 0.f, -2.f));
			break;
		case SDLK_x:
			models.at(0)->translate(frameTime * glm::vec3(0.f, 0.f, 2.f));
			break;
		case SDLK_q:
			models.at(0)->rotate(glm::rotate<GLfloat>(frameTime * -45.f, 0.f, 1.f, 0.f));
			break;
		case SDLK_e:
			models.at(0)->rotate(glm::rotate<GLfloat>(frameTime * 45.f, 0.f, 1.f, 0.f));
			break;
		default:
			break;
	};
*/
};


//=======================================================================================================================================================================================================================
// Program entry point
//=======================================================================================================================================================================================================================

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

	//Note: If we see an invalid enumerant error that's a result of glewExperimental
	//and it sounds like it can be safely ignored

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);

	// ==================================================================================================================================================================================================================
	// log opengl information for the current implementation
	// ==================================================================================================================================================================================================================
	util::gl_info();

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(util::glDebugCallback, 0);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
	
	glm::mat4 projection = glm::perspective(75.f, res_x / float (res_y), 1.f, 100.f);
	glm::vec4 viewPos(0.f, 0.f, 5.f, 1.f);
	glm::mat4 view = glm::lookAt(glm::vec3(viewPos), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	std::vector<Model*> models = setupModels(view, projection);

	
	glm::vec4 lightDir = glm::normalize(glm::vec4(1.f, 0.f, 1.f, 0.f));														// The light direction and half vector
	glm::mat4 lightView = glm::lookAt(glm::vec3(lightDir) * 8.f, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));       // Setup the light's view & projection matrix for the light
	glm::mat4 lightVP = glm::ortho(-4.f, 4.f, -4.f, 4.f, 1.f, 100.f) * lightView;											// For a directional light orthographic projection (point use perspective)

	
	GLuint fbo;																												// Setup our render targets
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	debug_msg("FBO created ... ");

	
	GLuint texBuffers[3];																									// 0 : diffuse, 1 : normals, 2 : depth
	glGenTextures(3, texBuffers);
	for (int i = 0; i < 2; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texBuffers[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB, res_x, res_y);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texBuffers[i], 0);
	};

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texBuffers[2]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, res_x, res_y);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texBuffers[2], 0);

	GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffers);

	debug_msg("Created and attached render targets ... ");

	//Need another shader program for the second pass
	glsl_program vsecondpass(glsl_shader(GL_VERTEX_SHADER,   "glsl/vsecondpass.vs"),
                             glsl_shader(GL_FRAGMENT_SHADER, "glsl/vsecondpass.fs"));

    vsecondpass.enable();

	GLuint diffuseUnif = vsecondpass.uniform_id("diffuse");
	GLuint normalUnif = vsecondpass.uniform_id("normal");
	GLuint depthUnif = vsecondpass.uniform_id("depth");
	glUniform1i(diffuseUnif, 0);
	glUniform1i(normalUnif, 1);
	glUniform1i(depthUnif, 2);

	GLuint invProjUnif = vsecondpass.uniform_id("inv_proj");
	GLuint invViewUnif = vsecondpass.uniform_id("inv_view");
	glm::mat4 invProj = glm::inverse(projection);
	glm::mat4 invView = glm::inverse(view);
	glUniformMatrix4fv(invProjUnif, 1, GL_FALSE, glm::value_ptr(invProj));
	glUniformMatrix4fv(invViewUnif, 1, GL_FALSE, glm::value_ptr(invView));

	//Pass them to the first pass shader for a forward lighting test
	GLuint lightDirUnif = vsecondpass.uniform_id("light_dir");
	GLuint viewPosUnif = vsecondpass.uniform_id("view_pos");

	glUniform4fv(lightDirUnif, 1, glm::value_ptr(lightDir));
	glUniform4fv(viewPosUnif, 1, glm::value_ptr(viewPos));

	//Shadow map is bound to texture unit 3
	GLuint shadowMapUnif = vsecondpass.uniform_id("shadow_map");
	glUniform1i(shadowMapUnif, 3);
	GLuint lightVPUnif = vsecondpass.uniform_id("light_vp");
	glUniformMatrix4fv(lightVPUnif, 1, GL_FALSE, glm::value_ptr(lightVP));

	//We render the second pass onto a quad drawn to the NDC
	Model quad("res/quad.obj", vsecondpass.id);

	//Setup the shadow map
	GLuint shadowTex, shadowFbo;
	setupShadowMap(shadowFbo, shadowTex);

	for (Model *m : models)
	{
		m->setShadowVP(lightVP);
	};
	
	// Setup a debug output quad to be drawn to NDC after all other rendering
	glsl_program dbgProgram(glsl_shader(GL_VERTEX_SHADER,   "glsl/vforward.vs"),
                            glsl_shader(GL_FRAGMENT_SHADER, "glsl/fforward_lum.fs"));

	Model dbgOut("res/quad.obj", dbgProgram.id);

	dbgOut.scale(glm::vec3(0.3f, 0.3f, 1.f));
	dbgOut.translate(glm::vec3(-0.7f, 0.7f, 0.f));
	dbgProgram.enable();

	GLuint dbgTex = dbgProgram.uniform_id("tex");
	glUniform1i(dbgTex, 3);

	debug_msg("Beginning the main loop");
	

	while(!glfwWindowShouldClose(window))
	{
		// ==============================================================================================================================================================================================================
		// GLEW library initialization
		// ==============================================================================================================================================================================================================
		renderShadowMap(shadowFbo, models);

		//First pass
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (Model *m : models)
		{
			m->bind();
			glDrawElements(GL_TRIANGLES, m->elems(), GL_UNSIGNED_SHORT, 0);
		};

		debug_msg("Shadow rendering done ... ");

		//Second pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		quad.bind();
		glDrawElements(GL_TRIANGLES, quad.elems(), GL_UNSIGNED_SHORT, 0);

		debug_msg("Second pass made ...");

		//Draw debug texture
		glDisable(GL_DEPTH_TEST);
		//Unset the compare mode so that we can draw it properly
		glActiveTexture(GL_TEXTURE3);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		dbgOut.bind();
		glDrawElements(GL_TRIANGLES, dbgOut.elems(), GL_UNSIGNED_SHORT, 0);
		//Set it back to the shadow map compare mode
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glEnable(GL_DEPTH_TEST);
	};


	glDeleteFramebuffers(1, &shadowFbo);
	glDeleteTextures(1, &shadowTex);

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(3, texBuffers);
	
	glfwTerminate();																										// close OpenGL window and terminate GLFW
	return 0;
}

std::vector<Model*> setupModels(const glm::mat4 &view, const glm::mat4 &proj)
{
	std::vector<Model*> models;


	shader1.init(glsl_shader(GL_VERTEX_SHADER, "glsl/vshader.vs"),
                 glsl_shader(GL_FRAGMENT_SHADER, "glsl/fshader.fs"));

	shader1.enable();

	//Load a texture for the polyhedron
	glActiveTexture(GL_TEXTURE4);
	GLuint texture = texture::bmp("res/texture.bmp");
	glBindTexture(GL_TEXTURE_2D, texture);
	GLuint texUnif = shader1.uniform_id("tex_diffuse");
	glUniform1i(texUnif, 4);

	//Pass the view/projection matrices
	GLint projUnif = shader1.uniform_id("proj");
	GLint viewUnif = shader1.uniform_id("view");

	glUniformMatrix4fv(projUnif, 1, GL_FALSE, glm::value_ptr(proj));
	glUniformMatrix4fv(viewUnif, 1, GL_FALSE, glm::value_ptr(view));

	shadow1.init(glsl_shader(GL_VERTEX_SHADER, "glsl/vshadow.vs"),   
	             glsl_shader(GL_FRAGMENT_SHADER, "glsl/fshadow.fs"));

	//With suzanne the self-shadowing is much easier to see
	Model *polyhedron = new Model("res/suzanne.obj", shader1.id, shadow1.id);
	polyhedron->translate(glm::vec3(1.0f, 0.0f, 1.0f));
	models.push_back(polyhedron);

	//TODO: Perhaps in the future a way to share programs across models and optimize drawing
	//order to reduce calls to glUseProgram? could also share proj/view matrices with UBOs
	shader2.init(glsl_shader(GL_VERTEX_SHADER, "glsl/vshader.vs"),   
	             glsl_shader(GL_FRAGMENT_SHADER, "glsl/fshader.fs"));

	shader2.enable();

	//Load a texture for the floor
	glActiveTexture(GL_TEXTURE5);
	texture = texture::bmp("res/texture2.bmp");
	glBindTexture(GL_TEXTURE_2D, texture);
	texUnif = shader2.uniform_id("tex_diffuse");
	glUniform1i(texUnif, 5);
	
	projUnif = shader2.uniform_id("proj");
	viewUnif = shader2.uniform_id("view");
	glUniformMatrix4fv(projUnif, 1, GL_FALSE, glm::value_ptr(proj));
	glUniformMatrix4fv(viewUnif, 1, GL_FALSE, glm::value_ptr(view));

	shadow2.init(glsl_shader(GL_VERTEX_SHADER, "glsl/vshadow.vs"),   
	             glsl_shader(GL_FRAGMENT_SHADER, "glsl/fshadow.fs"));

	shadow2.enable();

	Model *floor = new Model("res/quad.obj", shader2.id, shadow2.id);
	
	floor->scale(glm::vec3(3.0f, 3.0f, 1.0f));																			//Get it laying perpindicularish to the light direction and behind the camera some
	floor->rotate(glm::rotate(-35.f, glm::vec3(1.0f, 0.0f, 0.0f)));
	floor->rotate(glm::rotate(20.f, glm::vec3(0.0f, 1.0f, 0.0f)));
	models.push_back(floor);

	return models;
};

void setupShadowMap(GLuint &fbo, GLuint &tex)
{
	glActiveTexture(GL_TEXTURE3);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	//Will just use a shadow map equal to the window dimensions, must use specific formats for depth_stencil attachment
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, res_x, res_y);
	//No mip maps
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//Setup depth comparison mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,	GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//Don't wrap edges
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
	glDrawBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (GL_FRAMEBUFFER_COMPLETE == status)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	};
	const char * msg;	
	switch (status)
	{
		case GL_FRAMEBUFFER_UNDEFINED: msg = "The specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: msg = "Some of the framebuffer attachment points are framebuffer incomplete."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: msg = "Framebuffer does not have at least one image attached to it."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: msg = "The value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for some color attachment point(s) named by GL_DRAW_BUFFERi."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: msg = "GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER."; break;
		case GL_FRAMEBUFFER_UNSUPPORTED: msg = "The combination of internal formats of the attached images violates an implementation-dependent set of restrictions."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: msg = "The value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES;\
		                                                   or the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: msg = "Some framebuffer attachment is layered, and some populated attachment is not layered, or all populated color attachments are not from textures of the same target."; break;
	  default:
		msg = "Unknown Framebuffer error.";
	};
	debug_msg("Shadow FBO incomplete : %s", msg);
};

void renderShadowMap(GLuint &fbo, const std::vector<Model*> &models)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_POLYGON_OFFSET_FILL);																						// Polygon offset fill helps resolve depth-fighting
	glPolygonOffset(2.f, 4.f);
	for (Model *m : models)
	{
		m->bindShadow();
		glDrawElements(GL_TRIANGLES, m->elems(), GL_UNSIGNED_SHORT, 0);
	};
	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

};

