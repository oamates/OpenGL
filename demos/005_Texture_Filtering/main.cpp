//========================================================================================================================================================================================================================
// DEMO 005 : Texture Filtering
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>

#include "log.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"

//=======================================================================================================================================================================================================================
// create a simple 2x2 texture with various filtering and wrap modes
//=======================================================================================================================================================================================================================
GLuint create2x2_texture(GLenum texture_unit, GLint min_filter_mode, GLint mag_filter_mode, GLint wrap_mode)
{
    glActiveTexture(texture_unit);
    glm::u8vec3 pixels[] = 
    {
        glm::u8vec3(255,   0,   0),
        glm::u8vec3(  0, 255,   0),
        glm::u8vec3(  0,   0, 255),
        glm::u8vec3(  0,   0,   0)
    };

    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
    if (min_filter_mode == GL_LINEAR_MIPMAP_LINEAR)
        glGenerateMipmap(GL_TEXTURE_2D);

    return tex_id;
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    glfw_window_t window("Texture Filtering", 4, 3, 3, 1920, 1080, true);
    gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);

    //===================================================================================================================================================================================================================
    // Shader and uniform variables initialization
    //===================================================================================================================================================================================================================
    glsl_program_t quad_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/quad.fs"));

    quad_renderer.enable();
    uniform_t uniform_xy_min = quad_renderer["xy_min"];
    uniform_t uniform_xy_max = quad_renderer["xy_max"];      
    uniform_t uniform_uv_min = quad_renderer["uv_min"];
    uniform_t uniform_uv_max = quad_renderer["uv_max"];      
    uniform_t uniform_texture = quad_renderer["tex2d"];      

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLuint tex0 = create2x2_texture(GL_TEXTURE0, GL_NEAREST, GL_NEAREST, GL_REPEAT);
    GLuint tex1 = create2x2_texture(GL_TEXTURE1, GL_LINEAR,  GL_LINEAR,  GL_REPEAT);
    GLuint tex2 = create2x2_texture(GL_TEXTURE2, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);
    GLuint tex3 = create2x2_texture(GL_TEXTURE3, GL_LINEAR,  GL_LINEAR,  GL_CLAMP_TO_EDGE);

    //===================================================================================================================================================================================================================
    // Simple full-screen quad buffer setup
    //===================================================================================================================================================================================================================
    glm::vec2 uvs[] = 
    {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f)
    };

    GLuint vao_id, vbo_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : background color -- dark blue
    //===================================================================================================================================================================================================================
    glClearColor(0.03f, 0.0f, 0.11f, 1.0f);     
    glDisable(GL_DEPTH_TEST);

    const glm::vec2 quad_size = 0.4f * glm::vec2(1.0f, window.aspect());

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        //===============================================================================================================================================================================================================
        // clear back buffer, process events and update timer
        //===============================================================================================================================================================================================================
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        glm::vec2 basepoint;
        const float upper_row_y = 0.10f;                                                

        basepoint = glm::vec2(-0.92f, upper_row_y);                                                         // filtering : GL_NEAREST, wrap mode : GL_REPEAT    
        uniform_texture = 0;
        uniform_xy_min = basepoint;
        uniform_xy_max = basepoint + quad_size;
        uniform_uv_min = glm::vec2(0.0f);
        uniform_uv_max = glm::vec2(1.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        basepoint = glm::vec2(-0.44f, upper_row_y);                                                         // filtering : GL_NEAREST, wrap mode : GL_REPEAT
        uniform_texture = 0;
        uniform_xy_min = basepoint;
        uniform_xy_max = basepoint + quad_size;
        uniform_uv_min = glm::vec2(-1.0f);
        uniform_uv_max = glm::vec2(3.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        basepoint = glm::vec2(0.04f, upper_row_y);                                                          // filtering : GL_LINEAR,  wrap mode : GL_REPEAT
        uniform_texture = 1;
        uniform_xy_min = basepoint;
        uniform_xy_max = basepoint + quad_size;
        uniform_uv_min = glm::vec2( 0.0f);
        uniform_uv_max = glm::vec2(1.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        basepoint = glm::vec2(0.52f, upper_row_y);                                                          // filtering : GL_LINEAR,  wrap mode : GL_REPEAT
        uniform_texture = 1;
        uniform_xy_min = basepoint;
        uniform_xy_max = basepoint + quad_size;
        uniform_uv_min = glm::vec2(-1.0f);
        uniform_uv_max = glm::vec2(3.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        const float lower_row_y = -0.75f;
        basepoint = glm::vec2(-0.92f, lower_row_y);                                                         // filtering : GL_NEAREST, wrap mode : GL_CLAMP_TO_EDGE
        uniform_texture = 2;
        uniform_xy_min = basepoint;
        uniform_xy_max = basepoint + quad_size;
        uniform_uv_min = glm::vec2(0.0f);
        uniform_uv_max = glm::vec2(1.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        basepoint = glm::vec2(-0.44f, lower_row_y);                                                         // filtering : GL_NEAREST, wrap mode : GL_CLAMP_TO_EDGE
        uniform_texture = 2;
        uniform_xy_min = basepoint;
        uniform_xy_max = basepoint + quad_size;
        uniform_uv_min = glm::vec2(-1.0f);
        uniform_uv_max = glm::vec2(3.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        basepoint = glm::vec2(0.04f, lower_row_y);                                                          // filtering : GL_LINEAR,  wrap mode : GL_CLAMP_TO_EDGE
        uniform_texture = 3;
        uniform_xy_min = basepoint;
        uniform_xy_max = basepoint + quad_size;
        uniform_uv_min = glm::vec2(0.0f);
        uniform_uv_max = glm::vec2(1.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        basepoint = glm::vec2(0.52f, lower_row_y);                                                          // filtering : GL_LINEAR,  wrap mode : GL_CLAMP_TO_EDGE
        uniform_texture = 3;
        uniform_xy_min = basepoint;
        uniform_xy_max = basepoint + quad_size;
        uniform_uv_min = glm::vec2(-1.0f);
        uniform_uv_max = glm::vec2(3.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}