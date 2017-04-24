//========================================================================================================================================================================================================================
// DEMO 019: Plato Solids
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
#include <glm/gtc/matrix_transform.hpp> 

#include "constants.hpp"
#include "glfw_window.hpp"
#include "log.hpp"
#include "camera3d.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "plato.hpp"
#include "polyhedron.hpp"
#include "fbo.hpp"

//========================================================================================================================================================================================================================
// 3d moving camera : standard initial orientation in space
//========================================================================================================================================================================================================================
const float linear_velocity = 0.7f;
const float angular_rate = 0.0001f;
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
		data[index].rotor = glm::vec4(glm::sphericalRand(1.0f), 2.0f * glm::gaussRand(0.0f, 1.0f));
        index++;
	};
};

struct textured_quad
{
    glm::mat4 projection_matrix;
    GLuint vao_id, vbo_id;

    textured_quad(float left, float right, float bottom, float top, float zNear, float zFar, float z) 
    {
        projection_matrix = glm::ortho(left, right, bottom, top, zNear, zFar);
        debug_msg("textured_quad :: projection_matrix = %s", glm::to_string(projection_matrix).c_str());

        vertex_pt2 zquad[4] = 
        {
            vertex_pt2(glm::vec3(left,  bottom, z), glm::vec2(0.0f, 0.0f)),
            vertex_pt2(glm::vec3(right, bottom, z), glm::vec2(1.0f, 0.0f)),
            vertex_pt2(glm::vec3(right, top,    z), glm::vec2(1.0f, 1.0f)),
            vertex_pt2(glm::vec3(left,  top,    z), glm::vec2(0.0f, 1.0f)),
        };

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        vbo_id = generate_attribute_buffer(zquad, 4);
    };

    void render()
    {
        glBindVertexArray(vao_id);        
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    };

    ~textured_quad()
    {
        glDeleteBuffers(1, &vbo_id);
        glDeleteVertexArrays(1, &vao_id);
    };
};


int main()
{
    //===================================================================================================================================================================================================================
    // GLFW window creation + GLEW library initialization
    // 8AA samples, OpenGL 4.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    glfw_window window("Plato Solids", 8, 4, 3, 1920, 1080);
    window.log_info();
    window.mouse_handler = mouse_handler;
    window.keyboard_handler = keyboard_handler;
    camera.infinite_perspective(constants::two_pi / 6.0f, window.aspect_ratio(), 0.1f);

	//===================================================================================================================================================================================================================
	// phong lighting model shader initialization
	//===================================================================================================================================================================================================================
    glsl_program simple_light(glsl_shader(GL_VERTEX_SHADER,   "glsl/phong_light.vs"),
                              glsl_shader(GL_FRAGMENT_SHADER, "glsl/phong_light.fs"));

	simple_light.enable();

	GLuint uniform_projection_view_matrix = simple_light.uniform_id("projection_view_matrix");									// projection matrix uniform id
    GLuint uniform_time                   = simple_light.uniform_id("time");													// world time
    GLuint uniform_light_ws               = simple_light.uniform_id("light_ws");                                                // position of the light source
    GLuint uniform_camera_ws              = simple_light.uniform_id("camera_ws");                                               // position of the camera
    GLuint uniform_base                   = simple_light.uniform_id("buffer_base");
    GLuint uniform_solid_scale            = simple_light.uniform_id("solid_scale");
    GLuint uniform_diffuse_texture        = simple_light.uniform_id("diffuse_texture");
    GLuint uniform_normal_texture         = simple_light.uniform_id("normal_texture");
    glUniform1f(uniform_solid_scale, 15.0f);

    //===================================================================================================================================================================================================================
    // Initialize buffers : position + tangent frame + texture coordinates 
    //===================================================================================================================================================================================================================
    polyhedron tetrahedron, cube, octahedron, dodecahedron, icosahedron;

    tetrahedron.regular_pft2_vao(4, 4, plato::tetrahedron::vertices, plato::tetrahedron::normals, plato::tetrahedron::faces);
    cube.regular_pft2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces);
    octahedron.regular_pft2_vao(6, 8, plato::octahedron::vertices, plato::octahedron::normals, plato::octahedron::faces);
    dodecahedron.regular_pft2_vao(20, 12, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);
    icosahedron.regular_pft2_vao(12, 20, plato::icosahedron::vertices, plato::icosahedron::normals, plato::icosahedron::faces);

    //===================================================================================================================================================================================================================
    // Load textures : diffuse + bump for each polyhedron
    //===================================================================================================================================================================================================================
    const int TEXTURE_COUNT = 10;
    GLuint texture_id[TEXTURE_COUNT];

    const char* texture_filenames [TEXTURE_COUNT] = 
    {
        "res/tetrahedron.png",  "res/tetrahedron_bump.png",
        "res/cube.png",         "res/cube_bump.png",
        "res/octahedron.png",   "res/octahedron_bump.png",
        "res/pentagon.png",     "res/pentagon_bump.png",
        "res/icosahedron.png",  "res/icosahedron_bump.png"
    };

    for (int i = 0; i < TEXTURE_COUNT; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        texture_id[i] = texture::texture2d_png(texture_filenames[i]);
    };

    glActiveTexture(GL_TEXTURE0 + TEXTURE_COUNT + 1);
    GLuint glass_texture_id = texture::texture2d_png("res/marble.png");

    //===================================================================================================================================================================================================================
    // Mirror shader to render the reflected image
    //===================================================================================================================================================================================================================
    glsl_program mirror_shader(glsl_shader(GL_VERTEX_SHADER,   "glsl/mirror.vs"),
                               glsl_shader(GL_FRAGMENT_SHADER, "glsl/mirror.fs"));

    mirror_shader.enable();
    GLuint mirror_projection_view_matrix = mirror_shader.uniform_id("projection_view_matrix");
    GLuint mirror_texture                = mirror_shader.uniform_id("diffuse_texture");
    GLuint mirror_glass_texture          = mirror_shader.uniform_id("glass_texture");
    GLuint mirror_light_ws               = mirror_shader.uniform_id("light_ws");
    GLuint mirror_camera_ws              = mirror_shader.uniform_id("camera_ws");

    glUniform1i(mirror_texture, TEXTURE_COUNT);
    glUniform1i(mirror_glass_texture, TEXTURE_COUNT + 1);

    //===================================================================================================================================================================================================================
    // Initialize objects displacement vectors and rotation axes, and write the data to GL_SHADER_STORAGE_BUFFER
    // The buffer will be read according to gl_InstanceID variable and buffer_base uniform
    //===================================================================================================================================================================================================================
    const int N = 4;
    const int group_size = N * N * N;
	const float cell_size = 30.0f;
	const float origin_distance = 1.25f * cell_size * N;
    const float mirror_size = origin_distance + 0.6f * cell_size * N;
    const float height = 0.5f * cell_size * N;
	const GLsizeiptr chunk_size = group_size * sizeof(motion3d_t);	

	motion3d_t data[5 * group_size];

	fill_shift_rotor_data(&data[0 * group_size], glm::vec3(            0.0f,             0.0f, 0.0f), cell_size, N);
	fill_shift_rotor_data(&data[1 * group_size], glm::vec3(            0.0f,  origin_distance, 0.0f), cell_size, N);
	fill_shift_rotor_data(&data[2 * group_size], glm::vec3(            0.0f, -origin_distance, 0.0f), cell_size, N);
	fill_shift_rotor_data(&data[3 * group_size], glm::vec3( origin_distance,             0.0f, 0.0f), cell_size, N);
	fill_shift_rotor_data(&data[4 * group_size], glm::vec3(-origin_distance,             0.0f, 0.0f), cell_size, N);

    GLuint ssbo_id;
    glGenBuffers(1, &ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ssbo_id, 0, sizeof(data));

	//===================================================================================================================================================================================================================
	// light variables
	//===================================================================================================================================================================================================================

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    const float light_radius = cell_size * N; 

    const int MIRROR_TEXTURE_SIZE = 8192;
    fbo_color mirror_fbo(MIRROR_TEXTURE_SIZE, MIRROR_TEXTURE_SIZE, GL_TEXTURE0 + TEXTURE_COUNT);
    textured_quad mirror(-mirror_size, mirror_size, -mirror_size, mirror_size, height, -height, -1.5f * height);    

	//===================================================================================================================================================================================================================
	// the main loop
	//===================================================================================================================================================================================================================
    while(!window.should_close())
    {

        float time = window.time();
        glm::vec4 light_ws = glm::vec4(light_radius * cos(0.5f * time), light_radius * sin(0.5f * time), -0.66f * light_radius, 1.0f);
        glm::vec4 camera_ws = glm::vec4(camera.position(), 1.0f);
        glm::mat4 projection_view_matrix = camera.projection_view_matrix();

        simple_light.enable();
        glUniform1f(uniform_time, time);
        glUniform4fv(uniform_light_ws, 1, glm::value_ptr(light_ws));
        glUniform4fv(uniform_camera_ws, 1, glm::value_ptr(camera_ws));

        //===============================================================================================================================================================================================================
        // Bind off-screen framebuffer and render scene to 2d-texture (note : bind does clear both depth renderbuffer and color attachment)
        //===============================================================================================================================================================================================================
        mirror_fbo.bind();

        glUniformMatrix4fv(uniform_projection_view_matrix, 1, GL_FALSE, glm::value_ptr(mirror.projection_matrix));
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

        //===============================================================================================================================================================================================================
        // Bind default framebuffer back and render everything again with correct projection_view_matrix
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window.res_x, window.res_y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniformMatrix4fv(uniform_projection_view_matrix, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));

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

        mirror_shader.enable(); 
        glUniformMatrix4fv(mirror_projection_view_matrix, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));
        glUniform4fv(mirror_light_ws, 1, glm::value_ptr(light_ws));
        glUniform4fv(mirror_camera_ws, 1, glm::value_ptr(camera_ws));

        mirror.render();

        window.swap_buffers();
        window.poll_events();
    };

    glDeleteTextures(TEXTURE_COUNT, texture_id);
    glDeleteTextures(1, &glass_texture_id);

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    return 0;
};