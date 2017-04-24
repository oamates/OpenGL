//========================================================================================================================================================================================================================
// DEMO 038: GL_TRANSFORM_FEEDBACK_BUFFER
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
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/matrix_transform.hpp> 

#include "constants.hpp"
#include "glfw_window.hpp"
#include "log.hpp"
#include "camera3d.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "dds.hpp"
#include "vbm.hpp"


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

const int point_count = 50000;
static unsigned int seed = 0x13371337;

static inline float random_float()
{
    float res;
    unsigned int tmp;
    seed *= 16807;
    tmp = seed ^ (seed >> 4) ^ (seed << 15);
    *((unsigned int *) &res) = (tmp >> 9) | 0x3F800000;
    return (res - 1.0f);
};

static glm::vec3 random_vector(float minmag = 0.0f, float maxmag = 1.0f)
{
    glm::vec3 randomvec(random_float() * 2.0f - 1.0f, random_float() * 2.0f - 1.0f, random_float() * 2.0f - 1.0f);
    randomvec = normalize(randomvec);
    randomvec *= (random_float() * (maxmag - minmag) + minmag);
    return randomvec;
};

float aspect;
GLuint update_prog;
GLuint vao[2];
GLuint vbo[2];
GLuint xfb;

GLuint render_prog;
GLuint geometry_vbo;
GLuint render_vao;
GLint render_model_matrix_loc;
GLint render_projection_matrix_loc;

GLuint geometry_tex;

GLuint geometry_xfb;
GLuint particle_xfb;

GLint model_matrix_loc;
GLint projection_matrix_loc;
GLint triangle_count_loc;
GLint time_step_loc;

static inline int min(int a, int b)
{
    return a < b ? a : b;
};


int main()
{
    //===================================================================================================================================================================================================================
    // GLFW window creation + GLEW library initialization
    // 8AA samples, OpenGL 4.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    glfw_window window("GL_TRANSFORM_FEEDBACK_BUFFER", 8, 4, 3, 1920, 1080);
    window.log_info();
    window.mouse_handler = mouse_handler;
    window.keyboard_handler = keyboard_handler;
    camera.infinite_perspective(constants::two_pi / 6.0f, window.aspect_ratio(), 0.1f);

	//===================================================================================================================================================================================================================
	// First transform feedback shader
	//===================================================================================================================================================================================================================

    glsl_program transform_fb_shader;
    transform_fb_shader.attach(glsl_shader(GL_VERTEX_SHADER,   "glsl/transform_fb.vs").id);
    transform_fb_shader.attach(glsl_shader(GL_FRAGMENT_SHADER, "glsl/transform_fb.fs").id);

    static const char * varyings[] =
    {
        "position_out", "velocity_out"
    };
    glTransformFeedbackVaryings(transform_fb_shader.id, 2, varyings, GL_INTERLEAVED_ATTRIBS);
    transform_fb_shader.link();

    model_matrix_loc      = transform_fb_shader.uniform_id("model_matrix");
    projection_matrix_loc = transform_fb_shader.uniform_id("projection_matrix");
    triangle_count_loc    = transform_fb_shader.uniform_id("triangle_count");
    time_step_loc         = transform_fb_shader.uniform_id("time_step");

    //===================================================================================================================================================================================================================
    // First transform feedback shader
    //===================================================================================================================================================================================================================

    glsl_program render_shader;

    render_shader.attach(glsl_shader(GL_VERTEX_SHADER,   "glsl/render.vs").id);
    render_shader.attach(glsl_shader(GL_FRAGMENT_SHADER, "glsl/render.fs").id);

    static const char * varyings2[] =
    {
        "world_space_position"
    };

    glTransformFeedbackVaryings(render_shader.id, 1, varyings2, GL_INTERLEAVED_ATTRIBS);
    render_shader.link();

    render_model_matrix_loc      = render_shader.uniform_id("model_matrix");
    render_projection_matrix_loc = render_shader.uniform_id("projection_matrix");


    //===================================================================================================================================================================================================================
    // Transform feedback buffers initialization
    //===================================================================================================================================================================================================================

    glGenVertexArrays(2, vao);
    glGenBuffers(2, vbo);

    for (int i = 0; i < 2; i++)
    {
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, vbo[i]);
        glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, point_count * (sizeof(glm::vec4) + sizeof(glm::vec3)), 0, GL_DYNAMIC_COPY);
        if (i == 0)
        {
            struct buffer_t 
            {
                glm::vec4 position;
                glm::vec3 velocity;
            } * buffer = (buffer_t *) glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_WRITE_ONLY);

            for (int j = 0; j < point_count; j++)
            {
                buffer[j].velocity = random_vector();
                buffer[j].position = glm::vec4(buffer[j].velocity + glm::vec3(-0.5f, 40.0f, 0.0f), 1.0f);
                buffer[j].velocity = glm::vec3(buffer[j].velocity[0], buffer[j].velocity[1] * 0.3f, buffer[j].velocity[2] * 0.3f);
            }

            glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
        }

        glBindVertexArray(vao[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) + sizeof(glm::vec3), 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) + sizeof(glm::vec3), (GLvoid *)sizeof(glm::vec4));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }

    glGenBuffers(1, &geometry_vbo);
    glGenTextures(1, &geometry_tex);
    glBindBuffer(GL_TEXTURE_BUFFER, geometry_vbo);
    glBufferData(GL_TEXTURE_BUFFER, 1024 * 1024 * sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);
    glBindTexture(GL_TEXTURE_BUFFER, geometry_tex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, geometry_vbo);

    glGenVertexArrays(1, &render_vao);
    glBindVertexArray(render_vao);
    glBindBuffer(GL_ARRAY_BUFFER, geometry_vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);

    VBObject object;
    object.LoadFromVBM("res/armadillo.vbm", 0, 1, 2);

    float inv_aspect = window.inv_aspect_ratio();


	//===================================================================================================================================================================================================================
	// The main loop
	//===================================================================================================================================================================================================================
    while(!window.should_close())
    {

        static int frame_count = 0;
        float t = window.time() * 0.002f;
        static float q = 0.0f;
        static const glm::vec3 X(1.0f, 0.0f, 0.0f);
        static const glm::vec3 Y(0.0f, 1.0f, 0.0f);
        static const glm::vec3 Z(0.0f, 0.0f, 1.0f);

        glm::mat4 projection_matrix = glm::frustum(-1.0f, 1.0f, -inv_aspect, inv_aspect, 1.0f, 5000.0f) * glm::translate(glm::vec3(0.0f, 0.0f, -100.0f));
        glm::mat4 model_matrix =  glm::scale(glm::vec3(0.3f)) *
                                  glm::rotate(t * 360.0f, glm::vec3(0.0f, 1.0f, 0.0f)) *
                                  glm::rotate(t * 360.0f * 3.0f, glm::vec3(0.0f, 0.0f, 1.0f));

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        render_shader.enable();
        glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));
        glUniformMatrix4fv(render_projection_matrix_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        glBindVertexArray(render_vao);

        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, geometry_vbo);

        glBeginTransformFeedback(GL_TRIANGLES);
        object.Render();
        glEndTransformFeedback();

        transform_fb_shader.enable();
        model_matrix = glm::mat4(1.0f);
        glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));
        glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        glUniform1i(triangle_count_loc, object.GetVertexCount() / 3);

        if (t > q)
        {
            glUniform1f(time_step_loc, (t - q) * 2000.0f);
        }

        q = t;

        if ((frame_count & 1) != 0)
        {
            glBindVertexArray(vao[1]);
            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[0]);
        }
        else
        {
            glBindVertexArray(vao[0]);
            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[1]);
        }

        debug_msg("frame_count = %d", frame_count); 


        glBeginTransformFeedback(GL_POINTS);
        glDrawArrays(GL_POINTS, 0, min(point_count, (frame_count >> 3)));
        glEndTransformFeedback();

        glBindVertexArray(0);

        frame_count++;

        window.swap_buffers();
        window.poll_events();
    };
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    return 0;
};