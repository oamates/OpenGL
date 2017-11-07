//========================================================================================================================================================================================================================
// DEMO 075 : Post processing effects
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/ext.hpp> 
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "constants.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "torus.hpp"
#include "fbo.hpp"
#include "vertex.hpp"

static const char* post_processor_fs[] = 
{
    "glsl/effects/no_effect.fs",
    "glsl/effects/dream_vision.fs",
    "glsl/effects/frozen_glass.fs",
    "glsl/effects/night_vision.fs",
    "glsl/effects/pixelation.fs",
    "glsl/effects/posterization.fs",
    "glsl/effects/sepia.fs",
    "glsl/effects/swirl.fs",
    "glsl/effects/thermal_vision.fs"
}; 

static const int POST_PROCESSOR_COUNT = sizeof(post_processor_fs) / sizeof(const char*);


static const double TORUS_RADIUS = 3.562;
static const double TORUS_INNER_RADIUS = 1.837;
static const double TORUS_SCALE = 877.0;

static const int TORUS_U_DIV = 400;
static const int TORUS_V_DIV = 144;

static const double delta_u = 1.0 / (2.0 * TORUS_U_DIV);
static const double delta_v = 1.0 / (2.0 * TORUS_V_DIV);

glm::dvec3 disturb_vertex(const glm::dvec3& v, const glm::dvec3& n)
{
    return v + 0.14500 * glm::perlin(2.0 * v) * n +
               0.05989 * glm::dvec3(glm::perlin( 4.0 * v), glm::perlin( 4.4 * v), glm::perlin( 4.7 * v)) +
               0.02712 * glm::dvec3(glm::perlin( 8.0 * v), glm::perlin( 9.0 * v), glm::perlin(11.0 * v)) +
               0.01112 * glm::dvec3(glm::perlin(16.0 * v), glm::perlin(15.0 * v), glm::perlin(18.0 * v));

} 


vertex_pn_t toral_func (const glm::vec2& uv)
{
    double up = uv.x + delta_u;
    double um = uv.x - delta_u;

    double vp = uv.y + delta_v;
    double vm = uv.y - delta_v;

    double cos_2piup = glm::cos(constants::two_pi * up);
    double sin_2piup = glm::sin(constants::two_pi * up);
    double cos_2pivp = glm::cos(constants::two_pi * vp);
    double sin_2pivp = glm::sin(constants::two_pi * vp);

    double cos_2pium = glm::cos(constants::two_pi * um);
    double sin_2pium = glm::sin(constants::two_pi * um);
    double cos_2pivm = glm::cos(constants::two_pi * vm);
    double sin_2pivm = glm::sin(constants::two_pi * vm);

    glm::dvec3 v_pp = glm::dvec3((TORUS_RADIUS + TORUS_INNER_RADIUS * cos_2pivp) * cos_2piup, (TORUS_RADIUS + TORUS_INNER_RADIUS * cos_2pivp) * sin_2piup, TORUS_INNER_RADIUS * sin_2pivp);
    glm::dvec3 n_pp = glm::dvec3(cos_2pivp * cos_2piup, cos_2pivp * sin_2piup, sin_2pivp);
    glm::dvec3 vertex_pp = disturb_vertex(v_pp, n_pp);

    glm::dvec3 v_pm = glm::dvec3((TORUS_RADIUS + TORUS_INNER_RADIUS * cos_2pivm) * cos_2piup, (TORUS_RADIUS + TORUS_INNER_RADIUS * cos_2pivm) * sin_2piup, TORUS_INNER_RADIUS * sin_2pivm);
    glm::dvec3 n_pm = glm::dvec3(cos_2pivp * cos_2piup, cos_2pivm * sin_2piup, sin_2pivm);
    glm::dvec3 vertex_pm = disturb_vertex(v_pm, n_pm);

    glm::dvec3 v_mp = glm::dvec3((TORUS_RADIUS + TORUS_INNER_RADIUS * cos_2pivp) * cos_2pium, (TORUS_RADIUS + TORUS_INNER_RADIUS * cos_2pivp) * sin_2pium, TORUS_INNER_RADIUS * sin_2pivp);
    glm::dvec3 n_mp = glm::dvec3(cos_2pivp * cos_2pium, cos_2pivp * sin_2pium, sin_2pivp);
    glm::dvec3 vertex_mp = disturb_vertex(v_mp, n_mp);

    glm::dvec3 v_mm = glm::dvec3((TORUS_RADIUS + TORUS_INNER_RADIUS * cos_2pivm) * cos_2pium, (TORUS_RADIUS + TORUS_INNER_RADIUS * cos_2pivm) * sin_2pium, TORUS_INNER_RADIUS * sin_2pivm);
    glm::dvec3 n_mm = glm::dvec3(cos_2pivm * cos_2pium, cos_2pivm * sin_2pium, sin_2pivm);
    glm::dvec3 vertex_mm = disturb_vertex(v_mm, n_mm);

	glm::dvec3 vertex = 0.25 * TORUS_SCALE * (vertex_pp + vertex_pm + vertex_mp + vertex_mm);
    glm::dvec3 n = glm::cross(vertex_pp - vertex_mm, vertex_pm - vertex_mp);

	return vertex_pn_t(glm::vec3(vertex), glm::vec3(glm::normalize(n)));

}

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    int ppindex = 0;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
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

        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE) && (ppindex != POST_PROCESSOR_COUNT - 1)) ppindex++;
        if ((key == GLFW_KEY_KP_SUBTRACT) && (action == GLFW_RELEASE) && (ppindex != 0)) ppindex--;
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
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Post processing effects", 8, 3, 3, 1920, 1080, true);

	float fov = constants::two_pi / 6.0f;
	float aspect_ratio = window.aspect();
	float znear = 1.0f;
	glm::vec2 scale_xy = -glm::vec2(glm::tan(fov / 2.0f));
	scale_xy.x *= aspect_ratio;
	glm::mat4 projection_matrix = glm::infinitePerspective (fov, aspect_ratio, znear);

	//===================================================================================================================================================================================================================
	// shader program for geometry pass
	//===================================================================================================================================================================================================================
    glsl_program_t geometry(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/geometry_pass.vs"),
                            glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/geometry_pass.fs"));
	geometry.enable();
	uniform_t geometry_projection_view_matrix = geometry["projection_view_matrix"];

	//===================================================================================================================================================================================================================
	// shader program for screen-space ambient occlusion pass
	//===================================================================================================================================================================================================================
    glsl_program_t ssao(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao.vs"),
                        glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao.fs"));
	ssao.enable();
	ssao["scale_xy"] = scale_xy;
	ssao["projection_matrix"] = window.camera.projection_matrix;
	ssao["depth_map"] = 0;

	const unsigned int MAX_CLOUD_SIZE = 256;
	glm::vec3 rnd[MAX_CLOUD_SIZE];
	for (unsigned int i = 0; i < MAX_CLOUD_SIZE; ++i)
		rnd[i] = glm::sphericalRand(1.0f);

    ssao["cloud_size"] = 64;
	glUniform3fv(ssao["spherical_rand"], MAX_CLOUD_SIZE, glm::value_ptr(rnd[0]));

	//===================================================================================================================================================================================================================
	// shader program for ssao image blur 
	//===================================================================================================================================================================================================================
    glsl_program_t blur(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/blur.vs"),
                        glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blur.fs"));
    blur.enable();
    blur["ssao_image"] = 0;
    debug_msg("Cave generated. window.res_x = %d, window.res_y = %d", window.res_x, window.res_y);
	//===================================================================================================================================================================================================================
	// lighting shader program 
	//===================================================================================================================================================================================================================
    glsl_program_t simple_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/simple_light.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/simple_light.fs"));

    simple_light.enable();


    const int MAX_LIGHT_SOURCES = 8;
    glm::vec4 light_color[MAX_LIGHT_SOURCES] = 
        {
            glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),    
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
            glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),    
            glm::vec4(0.707f, 0.707f, 0.000f, 1.0f),
            glm::vec4(0.000f, 0.707f, 0.707f, 1.0f),    
            glm::vec4(0.707f, 0.000f, 0.707f, 1.0f),
            glm::vec4(0.577f, 0.577f, 0.577f, 1.0f),    
            glm::vec4(0.577f, 0.577f, 0.577f, 1.0f)
        };
    glm::vec4 light_motion[MAX_LIGHT_SOURCES];


    for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; ++i)
    {
        light_motion[i] = glm::vec4(glm::gaussRand(0.0f, 1.0f), glm::gaussRand(0.0f, 1.0f), glm::circularRand(5.0f));
    }

    glUniform4fv(simple_light["light_color"], MAX_LIGHT_SOURCES, glm::value_ptr(light_color[0]));

    uniform_t uniform_light_position  = simple_light["light_position"];
    uniform_t uniform_light_pvmatrix  = simple_light["projection_view_matrix"];
    uniform_t uniform_light_camera_ws = simple_light["camera_ws"];
    simple_light["ssao_image"] = 0;

	//===================================================================================================================================================================================================================
	// compile post-processing shader programs      
	//===================================================================================================================================================================================================================
    glsl_shader_t post_processor_vs(GL_VERTEX_SHADER, "glsl/post_processing.vs");
    glsl_program_t* post_processor[POST_PROCESSOR_COUNT];

    for (int i = 0; i < POST_PROCESSOR_COUNT; ++i)
    {
        post_processor[i] = new glsl_program_t(post_processor_vs, glsl_shader_t(GL_FRAGMENT_SHADER, post_processor_fs[i]));
        glsl_program_t& processor = *(post_processor[i]);
        processor.enable();
        processor["scene_texture"] = 0;
    } 

	//===================================================================================================================================================================================================================
	// create toral cave mesh
	//===================================================================================================================================================================================================================


	torus_t cave;
    cave.generate_vao_mt<vertex_pn_t>(toral_func, TORUS_U_DIV, TORUS_V_DIV);

	fbo_depth_t zbuffer(window.res_x, window.res_y, GL_TEXTURE0, GL_DEPTH_COMPONENT32, GL_LINEAR, GL_CLAMP_TO_EDGE);
	fbo_color_t<GL_TEXTURE_2D, 1> ssao_buffer(window.res_x, window.res_y);
	fbo_color_t<GL_TEXTURE_2D, 1> blur_buffer(window.res_x, window.res_y);
	color_rbo_t scene_buffer(window.res_x, window.res_y);

	//===================================================================================================================================================================================================================
	// generate quad VAO
	//===================================================================================================================================================================================================================
	GLuint quad_vao_id, quad_vbo_id;
	
	glm::vec2 quad_data [] =
	{
		glm::vec2(-1.0f, -1.0f),
		glm::vec2( 1.0f, -1.0f),
		glm::vec2( 1.0f,  1.0f),
		glm::vec2(-1.0f,  1.0f)
	};

	glGenVertexArrays(1, &quad_vao_id);
	glBindVertexArray(quad_vao_id);

	glGenBuffers(1, &quad_vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), glm::value_ptr(quad_data[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //===================================================================================================================================================================================================================
    // global OpenGL state
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(-1);

    //===================================================================================================================================================================================================================
    // main loop
    //===================================================================================================================================================================================================================

    while(!window.should_close())
    {
		float time = glfw::time();
        glm::mat4 projection_matrix = window.camera.projection_matrix;

        glm::vec3 camera_ws = window.camera.position();
        glm::mat4 view_matrix = window.camera.view_matrix;

		glm::mat4 projection_view_matrix = projection_matrix * view_matrix;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

        //===============================================================================================================================================================================================================
        // geometry rendering pass, just to get the z-texture filled
        //===============================================================================================================================================================================================================
		geometry.enable();
		zbuffer.bind(GL_FRAMEBUFFER);	
		glClear(GL_DEPTH_BUFFER_BIT);	
		geometry_projection_view_matrix = projection_view_matrix;
		cave.render();

		glBindVertexArray(quad_vao_id);

        //===============================================================================================================================================================================================================
        // screen space ambient occlusion calculation from z-texture
        //===============================================================================================================================================================================================================
    	ssao.enable();
		ssao_buffer.bind();
		glClear(GL_COLOR_BUFFER_BIT);
//		zbuffer.bind_texture(GL_TEXTURE0);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


    	blur.enable();
		blur_buffer.bind();
		glClear(GL_COLOR_BUFFER_BIT);
        ssao_buffer.bind_texture(GL_TEXTURE0);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        //===============================================================================================================================================================================================================
        // lighting pass
        //===============================================================================================================================================================================================================		
		scene_buffer.bind();
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    	simple_light.enable();
        blur_buffer.bind_texture(GL_TEXTURE0);

        glm::vec3 light_position[MAX_LIGHT_SOURCES];

        for (int i = 0; i < MAX_LIGHT_SOURCES; ++i)
        {
            glm::vec4 motion_param = light_motion[i];
            double u = motion_param.x + 0.025f * time * motion_param.z;
            double v = motion_param.y + 0.025f * time * motion_param.w;
            double cos_2piu = glm::cos(constants::two_pi_d * u);
            double sin_2piu = glm::sin(constants::two_pi_d * u);
            double cos_2piv = glm::cos(constants::two_pi_d * v);
            double sin_2piv = glm::sin(constants::two_pi_d * v);

            static const double R = TORUS_RADIUS;
            static const double r = 0.85 * TORUS_INNER_RADIUS;

            glm::dvec3 l = TORUS_SCALE * glm::dvec3((R + r * cos_2piv) * cos_2piu, (R + r * cos_2piv) * sin_2piu, r * sin_2piv);        

            light_position[i] = glm::vec3(l);

        }
        glUniform3fv(uniform_light_position, MAX_LIGHT_SOURCES, glm::value_ptr(light_position[0]));

        uniform_light_pvmatrix  = projection_view_matrix;
        uniform_light_camera_ws = camera_ws;
		cave.render();

        //===============================================================================================================================================================================================================
        // post processing pass
        //===============================================================================================================================================================================================================
	    glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene_buffer.bind_texture(GL_TEXTURE0);

        glsl_program_t& processor = *(post_processor[window.ppindex]);
		processor.enable();
		glBindVertexArray(quad_vao_id);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        window.swap_buffers();
        glfw::poll_events();
    }
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}