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

const char* internal_format_name(GLint format)
{
    switch (format)
    {
        /* Base Internal Formats */
        case GL_DEPTH_COMPONENT : return "GL_DEPTH_COMPONENT";
        case GL_DEPTH_STENCIL :   return "GL_DEPTH_STENCIL";
        case GL_RED :             return "GL_RED";
        case GL_RG :              return "GL_RG";
        case GL_RGB :             return "GL_RGB";
        case GL_RGBA :            return "GL_RGBA";

        /* Sized Internal Formats */
        case GL_R8 :              return "GL_R8";
        case GL_R8_SNORM :        return "GL_R8_SNORM";
        case GL_R16 :             return "GL_R16";
        case GL_R16_SNORM :       return "GL_R16_SNORM";
        case GL_RG8 :             return "GL_RG8";
        case GL_RG8_SNORM :       return "GL_RG8_SNORM";
        case GL_RG16 :            return "GL_RG16";
        case GL_RG16_SNORM :      return "GL_RG16_SNORM";
        case GL_R3_G3_B2 :        return "GL_R3_G3_B2";
        case GL_RGB4 :            return "GL_RGB4";
        case GL_RGB5 :            return "GL_RGB5";
        case GL_RGB8 :            return "GL_RGB8";
        case GL_RGB8_SNORM :      return "GL_RGB8_SNORM";
        case GL_RGB10 :           return "GL_RGB10";
        case GL_RGB12 :           return "GL_RGB12";
        case GL_RGB16_SNORM :     return "GL_RGB16_SNORM";
        case GL_RGBA2 :           return "GL_RGBA2";
        case GL_RGBA4 :           return "GL_RGBA4";
        case GL_RGB5_A1 :         return "GL_RGB5_A1";
        case GL_RGBA8 :           return "GL_RGBA8";
        case GL_RGBA8_SNORM :     return "GL_RGBA8_SNORM";
        case GL_RGB10_A2 :        return "GL_RGB10_A2";
        case GL_RGB10_A2UI :      return "GL_RGB10_A2UI";
        case GL_RGBA12 :          return "GL_RGBA12";
        case GL_RGBA16 :          return "GL_RGBA16";
        case GL_SRGB8 :           return "GL_SRGB8";
        case GL_SRGB8_ALPHA8 :    return "GL_SRGB8_ALPHA8";
        case GL_R16F :            return "GL_R16F";
        case GL_RG16F :           return "GL_RG16F";
        case GL_RGB16F :          return "GL_RGB16F";
        case GL_RGBA16F :         return "GL_RGBA16F";
        case GL_R32F :            return "GL_R32F";
        case GL_RG32F :           return "GL_RG32F";
        case GL_RGB32F :          return "GL_RGB32F";
        case GL_RGBA32F :         return "GL_RGBA32F";
        case GL_R11F_G11F_B10F :  return "GL_R11F_G11F_B10F";
        case GL_RGB9_E5 :         return "GL_RGB9_E5";
        case GL_R8I :             return "GL_R8I";
        case GL_R8UI :            return "GL_R8UI";
        case GL_R16I :            return "GL_R16I";
        case GL_R16UI :           return "GL_R16UI";
        case GL_R32I :            return "GL_R32I";
        case GL_R32UI :           return "GL_R32UI";
        case GL_RG8I :            return "GL_RG8I";
        case GL_RG8UI :           return "GL_RG8UI";
        case GL_RG16I :           return "GL_RG16I";
        case GL_RG16UI :          return "GL_RG16UI";
        case GL_RG32I :           return "GL_RG32I";
        case GL_RG32UI :          return "GL_RG32UI";
        case GL_RGB8I :           return "GL_RGB8I";
        case GL_RGB8UI :          return "GL_RGB8UI";
        case GL_RGB16I :          return "GL_RGB16I";
        case GL_RGB16UI :         return "GL_RGB16UI";
        case GL_RGB32I :          return "GL_RGB32I";
        case GL_RGB32UI :         return "GL_RGB32UI";
        case GL_RGBA8I :          return "GL_RGBA8I";
        case GL_RGBA8UI :         return "GL_RGBA8UI";
        case GL_RGBA16I :         return "GL_RGBA16I";
        case GL_RGBA16UI :        return "GL_RGBA16UI";
        case GL_RGBA32I :         return "GL_RGBA32I";
        case GL_RGBA32UI :        return "GL_RGBA32UI";
    }
    return "Unknown";
}

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW ImGui window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Bump Generator", 4, 4, 4, 1920, 1080, true);

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
    GLuint diffuse_tex_id = image::png::texture2d("../../../resources/tex2d/rock_wall.png", 0, GL_LINEAR, GL_LINEAR, GL_REPEAT, true);

    GLint texres_x, texres_y;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &texres_x);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texres_y);

    GLint internal_format;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);
    debug_msg("Texture internal format is %u. Format name = %s", internal_format, internal_format_name(internal_format));


    GLuint luminosity_tex_id   = generate_texture(GL_TEXTURE1, texres_x, texres_y, GL_R32F);
    GLuint normal_tex_id       = generate_texture(GL_TEXTURE2, texres_x, texres_y, GL_RGBA32F);
    GLuint displacement_tex_id = generate_texture(GL_TEXTURE3, texres_x, texres_y, GL_R32F);
    GLuint roughness_tex_id    = generate_texture(GL_TEXTURE4, texres_x, texres_y, GL_R32F);
    GLuint shininess_tex_id    = generate_texture(GL_TEXTURE5, texres_x, texres_y, GL_R32F);
    GLuint aux_tex_id          = generate_texture(GL_TEXTURE6, texres_x, texres_y, GL_R32F);

    glBindImageTexture(0, diffuse_tex_id,      0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA32F);
    glBindImageTexture(1, luminosity_tex_id,   0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glBindImageTexture(2, normal_tex_id,       0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(3, displacement_tex_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glBindImageTexture(4, roughness_tex_id,    0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glBindImageTexture(5, shininess_tex_id,    0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glBindImageTexture(6, aux_tex_id,          0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

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

    /* compute luminosity from rgb */
    luminosity_filter.enable();
    glDispatchCompute(texres_x >> 3, texres_y >> 3, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* compute normal map from luminosity by using Sobel/Scharr derivative filters */
    normal_filter.enable();
    glDispatchCompute(texres_x >> 3, texres_y >> 3, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* compute displacement map from normal map */
    displacement_filter.enable();
    uniform_t uni_df_n = displacement_filter["n"];

    const float zero = 0.0f;
    glClearTexImage(displacement_tex_id, 0, GL_RED, GL_FLOAT, &zero);
    int n = 32;

    for(int i = 0; i < 6; ++i)
    {
        uni_df_n = n;
        for(int j = 0; j < 15; ++j)
        {
            glBindImageTexture(3, displacement_tex_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
            glBindImageTexture(6, aux_tex_id,          0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
            glDispatchCompute(texres_x >> 3, texres_y >> 3, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            glBindImageTexture(3, aux_tex_id,          0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
            glBindImageTexture(6, displacement_tex_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
            glDispatchCompute(texres_x >> 3, texres_y >> 3, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
        n >>= 1;
    }



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