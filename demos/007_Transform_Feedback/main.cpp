//========================================================================================================================================================================================================================
// DEMO 007 : Transform Feedback
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "plato.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "image.hpp"
#include "polyhedron.hpp"

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
    // mouse handlers
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

#define MAX_PARTICLES 1000
#define PARTICLE_LIFETIME 10.0f
#define PARTICLE_TYPE_LAUNCHER 0.0f
#define PARTICLE_TYPE_SHELL 1.0f
#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Transform Feedback", 4, 4, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // blinn-phong lighting model with framed bump texturing
    //===================================================================================================================================================================================================================
    glsl_program_t basic_lighting(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/basic_lighting.vs"),
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/basic_lighting.fs"));

    uniform_t uni_bl_projection_view_matrix = basic_lighting["projection_view_matrix"];
    uniform_t uni_bl_camera_ws              = basic_lighting["camera_ws"];
    uniform_t uni_bl_light_ws               = basic_lighting["light_ws"]; 
    basic_lighting["diffuse_texture"] = 0;
    basic_lighting["bump_texture"] = 1; 

    //===================================================================================================================================================================================================================
    // billboard generator shader
    //===================================================================================================================================================================================================================
    glsl_program_t billboard_generator(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/billboard.vs"),
                                       glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/billboard.gs"),
                                       glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/billboard.fs"));

    billboard_generator.enable();
    uniform_t uni_bg_projection_view_matrix = billboard_generator["projection_view_matrix"];
    uniform_t uni_bg_camera_ws              = billboard_generator["camera_ws"];
    billboard_generator["particle_texture"] = 2;


    //===================================================================================================================================================================================================================
    // particle system update shader
    //===================================================================================================================================================================================================================
    glsl_program_t ps_update(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ps_update.vs"),
                             glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/ps_update.gs"),
                             glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ps_update.fs"));

    const GLchar* Varyings[4] = {"Type1", "Position1", "Velocity1", "Age1"};
    glTransformFeedbackVaryings(ps_update.id, 4, Varyings, GL_INTERLEAVED_ATTRIBS);

    ps_update.link();
    ps_update.enable();
    ps_update.dump_info();
    
    uniform_t uni_ps_time = ps_update["time"];
    uniform_t uni_ps_dt   = ps_update["dt"];

    ps_update["random_texture"] = 3;

    //===================================================================================================================================================================================================================
    // common variables
    //===================================================================================================================================================================================================================    
    double last_ts = glfw::time();

    polyhedron cube;
    cube.regular_pft2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces);

    glActiveTexture(GL_TEXTURE0);
    GLuint diffuse_texture_id = image::png::texture2d("../../../resources/tex2d/bricks.png");
    glActiveTexture(GL_TEXTURE1);
    GLuint bump_texture_id  = image::png::texture2d("../../../resources/tex2d/bricks_bump.png");
    
    //===================================================================================================================================================================================================================
    // particle variables
    //===================================================================================================================================================================================================================

    struct Particle
    {
        float Type;    
        glm::vec3 Pos;
        glm::vec3 Vel;    
        float LifetimeMillis;    
    };


    bool m_isFirst = true;
    unsigned int m_currVB = 0;
    unsigned int m_currTFB = 1;
    GLuint m_particleBuffer[2] = {0, 0};
    GLuint m_transformFeedback[2] = {0, 0};

    glActiveTexture(GL_TEXTURE2);
    GLuint particle_texture_id = image::png::texture2d("../../../resources/tex2d/fireworks_red.png");

    double time = 0.0;

    Particle Particles[MAX_PARTICLES];
    memset(Particles, 0, sizeof(Particles));

    Particles[0].Type = PARTICLE_TYPE_LAUNCHER;
    Particles[0].Pos = glm::vec3(0.0f, 0.0f, 1.0f);
    Particles[0].Vel = glm::vec3(0.0f, 0.0001f, 0.0f);
    Particles[0].LifetimeMillis = 0.0f;

    glGenTransformFeedbacks(2, m_transformFeedback);    
    glGenBuffers(2, m_particleBuffer);
    
    for (unsigned int i = 0; i < 2 ; i++)
    {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
        glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), Particles, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);        
    }

    //===================================================================================================================================================================================================================
    // random texture creation
    //===================================================================================================================================================================================================================
    GLuint random_texture_id;
    const unsigned int Size = 1000;
    glm::vec3 pRandomData[Size];

    std::mt19937 engine;
    std::normal_distribution<float> dist;

    for (unsigned int i = 0 ; i < Size; i++)
        pRandomData[i] = glm::vec3(dist(engine), dist(engine), dist(engine));
    
    glActiveTexture(GL_TEXTURE3);
    glGenTextures(1, &random_texture_id);
    glBindTexture(GL_TEXTURE_1D, random_texture_id);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, Size, 0, GL_RGB, GL_FLOAT, pRandomData);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);    

    glEnable(GL_DEPTH_TEST);

    while(!window.should_close())
    {
        window.new_frame();

        float time = 0.01 * window.frame_ts;
        float dt = 0.01 * window.frame_dt;

        glm::mat4 projection_matrix = window.camera.projection_matrix;
        glm::mat4 view_matrix = window.camera.view_matrix;
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 camera_ws = window.camera.position();
        glm::vec3 light_ws = glm::vec3(3.0f * glm::cos(0.35f * time), 3.0f * glm::sin(0.35f * time), 2.0f);

        //===============================================================================================================================================================================================================
        // render the scene
        //===============================================================================================================================================================================================================
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        basic_lighting.enable();
        uni_bl_projection_view_matrix = projection_view_matrix;
        uni_bl_camera_ws              = camera_ws;
        uni_bl_light_ws               = light_ws; 
        cube.render();

        //===============================================================================================================================================================================================================
        // update the particles
        //===============================================================================================================================================================================================================
        ps_update.enable();
        uni_ps_time = time;
        uni_ps_dt = dt;

        glEnable(GL_RASTERIZER_DISCARD);
        
        glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);    
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[m_currTFB]);
        
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        
        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*) offsetof(Particle, Type));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*) offsetof(Particle, Pos));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*) offsetof(Particle, Vel));
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*) offsetof(Particle, LifetimeMillis));   
        
        glBeginTransformFeedback(GL_POINTS);
        
        if (m_isFirst)
        {
            glDrawArrays(GL_POINTS, 0, 1);
            m_isFirst = false;
        }
        else
        {
            glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currVB]);
        }            
        
        glEndTransformFeedback();
        
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(3);

        //===============================================================================================================================================================================================================
        // render the particles
        //===============================================================================================================================================================================================================
        billboard_generator.enable();
        uni_bg_projection_view_matrix = projection_view_matrix;
        uni_bg_camera_ws              = camera_ws;

        glDisable(GL_RASTERIZER_DISCARD);
        glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currTFB]);    
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*) 4);  // position
        glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currTFB]);
        glDisableVertexAttribArray(0);

        m_currVB = m_currTFB;
        m_currTFB = (m_currTFB + 1) & 0x1;

        window.end_frame();
    }

    glDeleteTransformFeedbacks(2, m_transformFeedback);
    glDeleteBuffers(2, m_particleBuffer);

    glfw::terminate();
    return 0;
}