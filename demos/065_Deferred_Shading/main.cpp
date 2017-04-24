//========================================================================================================================================================================================================================
// DEMO 066: Deferred shading
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
#include "vao.hpp"
#include "vertex.hpp"
#include "plato.hpp"
#include "polyhedron.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;
    int draw_mode = 0;
    bool wireframe = false;

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

        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE))
            draw_mode = (draw_mode + 1) % 5;

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
            wireframe = !wireframe;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

const GLuint SCR_WIDTH = 1920, SCR_HEIGHT = 1080;

struct Light
{
    glm::vec3 position;
    glm::vec3 color;    
    float linear;
    float quadratic;
    float radius;
};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Deferred Shading", 4, 3, 3, SCR_WIDTH, SCR_HEIGHT, true, true);

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glEnable(GL_DEPTH_TEST);

    //===================================================================================================================================================================================================================
    // light sources
    //===================================================================================================================================================================================================================
    const GLuint NR_LIGHTS = 32;
    Light lights[NR_LIGHTS];
    srand(13);
    for (GLuint i = 0; i < NR_LIGHTS; i++)
    {
        //===============================================================================================================================================================================================================
        // Calculate slightly random offsets
        //===============================================================================================================================================================================================================
        GLfloat xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
        GLfloat yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
        GLfloat zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
        lights[i].position = glm::vec3(xPos, yPos, zPos);

        //===============================================================================================================================================================================================================
        // Also calculate random color
        //===============================================================================================================================================================================================================
        GLfloat rColor = ((rand() % 100) / 200.0f) + 0.5;
        GLfloat gColor = ((rand() % 100) / 200.0f) + 0.5;
        GLfloat bColor = ((rand() % 100) / 200.0f) + 0.5;
        lights[i].color = glm::vec3(rColor, gColor, bColor);

        lights[i].linear = 0.7;
        lights[i].quadratic = 1.8;
        lights[i].radius = 5.5f;
    }

    //===================================================================================================================================================================================================================
	// Setup and compile our shaders
    //===================================================================================================================================================================================================================
    glsl_program_t geometry_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/geometry_pass.vs"), 
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/geometry_pass.fs"));
    geometry_pass.enable();
    uniform_t uni_gp_model_matrix = geometry_pass["model_matrix"];
    uniform_t uni_gp_pv_matrix = geometry_pass["projection_view_matrix"];
    geometry_pass["diffuse_tex"] = 3;
    geometry_pass["specular_tex"] = 4;

    glsl_program_t lighting_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/lighting_pass.vs"), 
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/lighting_pass.fs"));
    lighting_pass.enable();
    uniform_t uni_lp_camera_ws = lighting_pass["camera_ws"];
    glUniform1fv(lighting_pass["lights"], sizeof(lights) / sizeof(GLfloat), (const GLfloat*) lights);
    uniform_t uni_lp_draw_mode = lighting_pass["draw_mode"];
    lighting_pass["position_tex"] = 0;
    lighting_pass["normal_tex"] = 1;
    lighting_pass["albedo_tex"] = 2;

    glsl_program_t light_box(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/light_box.vs"), 
                             glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/light_box.fs"));
    light_box.enable();
    uniform_t uni_lb_model_matrix = light_box["model_matrix"];
    uniform_t uni_lb_pv_matrix = light_box["projection_view_matrix"];
    uniform_t uni_lb_light_color = light_box["light_color"];

    //===================================================================================================================================================================================================================
	// Models
    //===================================================================================================================================================================================================================
    vao_t cyborg;
	cyborg.init("../../../resources/models/vao/pnt2/cyborg.vao");
    glActiveTexture(GL_TEXTURE3);
    GLuint grass_tex = image::png::texture2d("../../../resources/models/vao/pnt2/cyborg_diffuse.png");
    glActiveTexture(GL_TEXTURE4);
    GLuint dirt_tex  = image::png::texture2d("../../../resources/models/vao/pnt2/cyborg_specular.png");

    const GLuint NR_OBJECTS = 9;
	glm::vec3 objectPositions[NR_OBJECTS];
    objectPositions[0] = glm::vec3(-3.0, -3.0, -3.0);
    objectPositions[1] = glm::vec3( 0.0, -3.0, -3.0);
    objectPositions[2] = glm::vec3( 3.0, -3.0, -3.0);
    objectPositions[3] = glm::vec3(-3.0, -3.0,  0.0);
    objectPositions[4] = glm::vec3( 0.0, -3.0,  0.0);
    objectPositions[5] = glm::vec3( 3.0, -3.0,  0.0);
    objectPositions[6] = glm::vec3(-3.0, -3.0,  3.0);
    objectPositions[7] = glm::vec3( 0.0, -3.0,  3.0);
    objectPositions[8] = glm::vec3( 3.0, -3.0,  3.0);

    //===================================================================================================================================================================================================================
	// Set up G-Buffer
	// 3 textures:
	// 1. Positions (RGB)
	// 2. Color (RGB) + Specular (A)
	// 3. Normals (RGB)
    //===================================================================================================================================================================================================================
	GLuint gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    GLuint gPosition, gNormal, gAlbedoSpec;

    //===================================================================================================================================================================================================================
    // Position color buffer
    //===================================================================================================================================================================================================================
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    //===================================================================================================================================================================================================================
    // Normal color buffer
    //===================================================================================================================================================================================================================
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    //===================================================================================================================================================================================================================
    // Color + Specular color buffer
    //===================================================================================================================================================================================================================
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    //===================================================================================================================================================================================================================
    // Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    //===================================================================================================================================================================================================================
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    //===================================================================================================================================================================================================================
	// Create and attach depth buffer (renderbuffer)
    //===================================================================================================================================================================================================================
	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    //===================================================================================================================================================================================================================
	// Finally check if framebuffer is complete
    //===================================================================================================================================================================================================================
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		exit_msg("Framebuffer not complete!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //===================================================================================================================================================================================================================
    // Create fake vao for full-screen rendering
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    polyhedron cube;
    cube.regular_pnt2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces);    

    //===================================================================================================================================================================================================================
	// main program loop
    //===================================================================================================================================================================================================================
	while (!window.should_close())
	{
        window.new_frame();

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 camera_ws = window.camera.position();

        //===============================================================================================================================================================================================================
		// Step 1: Geometry Pass: render scene's geometry/color data into gbuffer
        //===============================================================================================================================================================================================================
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
            if (window.wireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		    geometry_pass.enable();
            uni_gp_pv_matrix = projection_view_matrix;

		    for (GLuint i = 0; i < NR_OBJECTS; i++)
		    {
			    glm::mat4 model_matrix = glm::mat4(1.0f);
                model_matrix = glm::translate(model_matrix, objectPositions[i]);
                model_matrix = glm::scale(model_matrix, glm::vec3(0.25f));
                uni_gp_model_matrix = model_matrix;
                cyborg.render();
		    }

            if (window.wireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //===============================================================================================================================================================================================================
        // Step 2: Lighting Pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
        //===============================================================================================================================================================================================================
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        lighting_pass.enable();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

        uni_lp_camera_ws = camera_ws;
        uni_lp_draw_mode = window.draw_mode;
        glBindVertexArray(vao_id);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // Step 3: Copy content of geometry's depth buffer to default framebuffer's depth buffer
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //===============================================================================================================================================================================================================
        // Step 4: Render lights on top of scene, by blitting
        //===============================================================================================================================================================================================================
        light_box.enable();
        uni_lb_pv_matrix = projection_view_matrix;

        for (GLuint i = 0; i < NR_LIGHTS; i++)
        {
            glm::mat4 model_matrix = glm::mat4(1.0);
            model_matrix = glm::translate(model_matrix, lights[i].position);
            model_matrix = glm::scale(model_matrix, glm::vec3(0.25f));
            uni_lb_model_matrix = model_matrix;
            uni_lb_light_color = lights[i].color;
            cube.render();
        }

		window.end_frame();
	}

	glfw::terminate();
	return 0;
}