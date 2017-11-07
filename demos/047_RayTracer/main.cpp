//========================================================================================================================================================================================================================
// DEMO 047 : Ray Tracer
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    glfw_window_t window("Ray Tracer", 8, 4, 0, 1920, 1080, true /*, true */);
    gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);

    //===================================================================================================================================================================================================================
    // shaders
    //===================================================================================================================================================================================================================

    glsl_program_t initializer(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/init.cs"));
    glsl_program_t ray_tracer(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/trace.cs"));

    glsl_program_t renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/render.vs"),
                            glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/render.fs"));

    //===================================================================================================================================================================================================================
    // ray and atomic counter buffers
    //===================================================================================================================================================================================================================
    GLuint ray_buffer[2], counter_buffer;

    glGenBuffers(2, ray_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ray_buffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 64 * 1024 * 1024, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ray_buffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 64 * 1024 * 1024, 0, GL_DYNAMIC_COPY);

    static const GLuint zeros[] = {0, 0};
    glGenBuffers(1, &counter_buffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counter_buffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(zeros), zeros, GL_DYNAMIC_COPY);

    //===================================================================================================================================================================================================================
    // full-screen quad VAO
    //===================================================================================================================================================================================================================
    GLuint vao_id, vbo_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    static const glm::vec4 vertices[] =
    {
        glm::vec4(-1.0f, -1.0f, 0.5f, 1.0f),
        glm::vec4( 1.0f, -1.0f, 0.5f, 1.0f),
        glm::vec4( 1.0f,  1.0f, 0.5f, 1.0f),
        glm::vec4(-1.0f,  1.0f, 0.5f, 1.0f)
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    //===================================================================================================================================================================================================================
    // output texture
    //===================================================================================================================================================================================================================
    const GLuint LOD = 9;
    const GLuint TEXTURE_SIZE = 1 << LOD;
    GLuint output_image;
    glGenTextures(1, &output_image);
    glBindTexture(GL_TEXTURE_2D, output_image);
    glTexStorage2D(GL_TEXTURE_2D, LOD, GL_RGBA32F, TEXTURE_SIZE, TEXTURE_SIZE);

    //===================================================================================================================================================================================================================
    // Global GL settings :
    // DEPTH not needed, hence disabled
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glDisable(GL_DEPTH_TEST);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        initializer.enable();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ray_buffer[0]);
        glDispatchCompute(TEXTURE_SIZE / 16, TEXTURE_SIZE / 16, 1);
        
        // Reset atomic counters
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, counter_buffer);
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(zeros), zeros, GL_DYNAMIC_COPY);
        
        glBindImageTexture(0, output_image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        
        // Bind ray buffers
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ray_buffer[0]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ray_buffer[1]);
        
        // Bind the trace program
        ray_tracer.enable();
        glDispatchCompute(TEXTURE_SIZE / 16, TEXTURE_SIZE / 16, 1);
        
        glm::ivec2* ptr = (glm::ivec2*) glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
        
        // Now bind the texture for rendering _from_
        glBindTexture(GL_TEXTURE_2D, output_image);
        
        // Clear, select the rendering program and draw a full screen quad
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.enable();
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        
        window.end_frame();
    }


    glDeleteTextures(1, &output_image);
    glDeleteVertexArrays(1, &vao_id);

    glfw::terminate();
    return 0;
}