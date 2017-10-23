//========================================================================================================================================================================================================================
// DEMO 032: Bump Generator with GL_COMPUTE_SHADER
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

#include "log.hpp"
#include "imgui_window.hpp"
#include "gl_info.hpp"
#include "shader.hpp"
#include "image.hpp"

struct demo_window_t : public imgui_window_t
{
    int texture = 1;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : imgui_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO | OPENGL_COMPUTE_SHADER_INFO);
    }

    //===================================================================================================================================================================================================================
    // mouse and keyboard handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE))
        {
            texture++;
            if (texture > 5) texture = 1;
        }
    }

    void update_ui()
    {
    }

};

GLuint generate_texture(GLuint unit, GLsizei res_x, GLsizei res_y, GLenum internal_format)
{
    GLuint tex_id;
    glActiveTexture(unit);
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, res_x, res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex_id;
}

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW ImGui window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Bump Generator", 4, 4, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Texture units and that will be used for bump generation
    // Unit 0 : input diffuse texture GL_RGB8UI
    // Unit 1 : luminosity texture GL_R32F
    // Unit 2 : normal texture, GL_RGBA32F
    // Unit 3 : displacement texture, GL_R32F
    // Unit 4 : roughness texture, GL_R32F
    // Unit 5 : shininess texture, GL_R32F
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint diffuse_tex_id = image::png::texture2d("../../../resources/tex2d/rock_wall.png", 0, GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, true);

    GLint texres_x, texres_y;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &texres_x);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texres_y);

    GLuint luminosity_tex_id   = generate_texture(GL_TEXTURE1, texres_x, texres_y, GL_R32F);
    GLuint normal_tex_id       = generate_texture(GL_TEXTURE2, texres_x, texres_y, GL_RGBA32F);
    GLuint displacement_tex_id = generate_texture(GL_TEXTURE3, texres_x, texres_y, GL_R32F);
    GLuint roughness_tex_id    = generate_texture(GL_TEXTURE4, texres_x, texres_y, GL_R32F);
    GLuint shininess_tex_id    = generate_texture(GL_TEXTURE5, texres_x, texres_y, GL_R32F);

    glBindImageTexture(0, diffuse_tex_id,      0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA32F);
    glBindImageTexture(1, luminosity_tex_id,   0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glBindImageTexture(2, normal_tex_id,       0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(3, displacement_tex_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glBindImageTexture(4, roughness_tex_id,    0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glBindImageTexture(5, shininess_tex_id,    0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

    //===================================================================================================================================================================================================================
    // compute shader compilation and subroutine indices querying
    //===================================================================================================================================================================================================================
    glsl_program_t luminosity_filter  (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/luminosity_filter.cs"));
    glsl_program_t normal_filter      (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/normal_filter.cs"));
    glsl_program_t displacement_filter(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/displacement_filter.cs"));
    glsl_program_t roughness_filter   (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/roughness_filter.cs"));
    glsl_program_t shininess_filter   (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/shininess_filter.cs"));

    //===================================================================================================================================================================================================================
    // quad rendering shader
    //===================================================================================================================================================================================================================
    glsl_program_t quad_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/quad.fs"));
    quad_renderer.enable();

    uniform_t uniform_quad = quad_renderer["quad"];
    uniform_t uniform_teximage = quad_renderer["teximage"];

    //===================================================================================================================================================================================================================
    // create fake VAO for rendering quads
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    //===================================================================================================================================================================================================================
    // Global GL settings :
    // DEPTH not needed, hence disabled
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    luminosity_filter.enable();
    glDispatchCompute(texres_x >> 3, texres_y >> 3, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //===============================================================================================================================================================================================================
        // wait for the compute shader to finish its work and show both original and processed image
        //===============================================================================================================================================================================================================
        quad_renderer.enable();
        glBindVertexArray(vao_id);

        const float margin = 0.125;
        float aspect = window.aspect();

        uniform_quad = glm::vec4(-1.0f, -0.5f * aspect, 0.0f, 0.5f * aspect) + margin * glm::vec4(1.0f, aspect, -1.0f, -aspect);
        uniform_teximage = 0;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        uniform_quad = glm::vec4( 0.0f, -0.5f * aspect, 1.0f, 0.5f * aspect) + margin * glm::vec4(1.0f, aspect, -1.0f, -aspect);
        uniform_teximage = window.texture;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // After end_frame call ::
        //  - GL_DEPTH_TEST is disabled
        //  - GL_CULL_FACE is disabled
        //  - GL_SCISSOR_TEST is enabled
        //  - GL_BLEND is enabled -- blending mode GL_SRC_ALPHA/GL_ONE_MINUS_SRC_ALPHA with blending function GL_FUNC_ADD
        //  - VAO binding is destroyed
        //===============================================================================================================================================================================================================
        window.end_frame();
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_BLEND);

    }

    glDeleteTextures(1, &diffuse_tex_id);
    glDeleteTextures(1, &luminosity_tex_id);
    glDeleteTextures(1, &normal_tex_id);
    glDeleteTextures(1, &displacement_tex_id);
    glDeleteTextures(1, &roughness_tex_id);
    glDeleteTextures(1, &shininess_tex_id);
    glDeleteVertexArrays(1, &vao_id);    

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}