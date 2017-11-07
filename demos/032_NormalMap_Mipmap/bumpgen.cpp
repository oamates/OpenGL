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
#include "camera.hpp"
#include "polyhedron.hpp"
#include "plato.hpp"

const int MAX_LOD = 7;

struct demo_window_t : public imgui_window_t
{
    int texture = 0;
    int level = 0;
    bool pause = false;
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : imgui_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
          camera(32.0, 0.125, glm::mat4(1.0f))        
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO | OPENGL_COMPUTE_SHADER_INFO);
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

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
            pause = !pause;

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE))
        {
            texture++;
            if (texture > 4) texture = 0;
        }

        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE))
        {
            level++;
            if (level == MAX_LOD) level--;
        }

        if ((key == GLFW_KEY_KP_SUBTRACT) && (action == GLFW_RELEASE))
        {
            level--;
            if (level < 0) level = 0;
        }


    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }

    void update_ui()
    {
    }
};

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

GLuint generate_texture(GLuint unit, GLsizei res_x, GLsizei res_y, GLenum internal_format)
{
    GLuint tex_id;
    glActiveTexture(unit);
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, res_x, res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,  GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R,  GL_MIRRORED_REPEAT);
    return tex_id;
}

GLuint generate_mipmap_texture(GLuint unit, GLsizei res_x, GLsizei res_y, GLenum internal_format, GLsizei levels)
{
    GLuint tex_id;
    glActiveTexture(unit);
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexStorage2D(GL_TEXTURE_2D, levels, internal_format, res_x, res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,  GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R,  GL_MIRRORED_REPEAT);
    return tex_id;
}

int main(int argc, char *argv[])
{
    const int res_x = 1920;
    const int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW ImGui window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Normal map mipmap generator", 4, 4, 0, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // Texture units and that will be used for bump generation
    // Unit 0 : input diffuse texture, GL_RGBA32F
    // Unit 1 : blurred diffuse texture, GL_RGBA32F
    // Unit 2 : luminosity texture, GL_R32F
    // Unit 3 : blurred luminosity texture, GL_R32F
    // Unit 4 : normal texture, GL_RGBA32F
    // Unit 6 : auxiliary texture, GL_RGBA32F
    // Unit 7 : auxiliary texture, GL_R32F
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint diffuse_tex_id = image::png::texture2d("../../../resources/tex2d/rock_wall.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT);

    GLint internal_format;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);
    debug_msg("Texture internal format is %u. Format name = %s", internal_format, internal_format_name(internal_format));

    GLsizei tex_res_x[MAX_LOD], tex_res_y[MAX_LOD];
    for (int l = 0; l < MAX_LOD; ++l)
    {
        glGetTexLevelParameteriv(GL_TEXTURE_2D, l, GL_TEXTURE_WIDTH,  &tex_res_x[l]);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, l, GL_TEXTURE_HEIGHT, &tex_res_y[l]);
        debug_msg("\tLevel %u size = %u x %u", l, tex_res_x[l], tex_res_y[l]);
    }

    GLuint luma_tex_id            = generate_mipmap_texture(GL_TEXTURE1, tex_res_x[0], tex_res_y[0], GL_R32F,    MAX_LOD);
    GLuint normal_tex_id          = generate_mipmap_texture(GL_TEXTURE2, tex_res_x[0], tex_res_y[0], GL_RGBA32F, MAX_LOD);
    GLuint normal_ext_tex_id      = generate_mipmap_texture(GL_TEXTURE3, tex_res_x[0], tex_res_y[0], GL_RGBA32F, MAX_LOD);
    GLuint normal_combined_tex_id = generate_mipmap_texture(GL_TEXTURE4, tex_res_x[0], tex_res_y[0], GL_RGBA32F, MAX_LOD);

    //===================================================================================================================================================================================================================
    // compute shader compilation
    //===================================================================================================================================================================================================================
    glsl_program_t luminosity_filter    (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/luminosity_filter.cs"));
    glsl_program_t normal_filter        (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/normal_filter.cs"));
    glsl_program_t extension_filter     (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/extension_filter.cs"));
    glsl_program_t level_combine_filter (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/level_combine_filter.cs"));


    //===================================================================================================================================================================================================================
    // compute luminosity texture for each mip level
    //===================================================================================================================================================================================================================
    luminosity_filter.enable();
    uniform_t uni_lf_diffuse_tex = luminosity_filter["diffuse_tex"];
    uniform_t uni_lf_luma_image  = luminosity_filter["luma_image"];
    uniform_t uni_lf_tex_level   = luminosity_filter["tex_level"];

    uni_lf_diffuse_tex = 0;
    uni_lf_luma_image = 1;

    for (int l = 0; l < MAX_LOD; ++l)
    {
        uni_lf_tex_level = l;
        glBindImageTexture(1, luma_tex_id, l, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        glDispatchCompute(tex_res_x[l] >> 3, tex_res_y[l] >> 3, 1);
    }

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    //===================================================================================================================================================================================================================
    // compute normal texture for each mip level
    //===================================================================================================================================================================================================================
    normal_filter.enable();
    uniform_t uni_nf_luma_tex      = normal_filter["luma_tex"];
    uniform_t uni_nf_inv_amplitude = normal_filter["inv_amplitude"];
    uniform_t uni_nf_normal_image  = normal_filter["normal_image"];
    uniform_t uni_nf_tex_level     = normal_filter["tex_level"];

    uni_nf_luma_tex = 1;
    uni_nf_normal_image = 2;

    for (int l = 0; l < MAX_LOD; ++l)
    {
        uni_nf_tex_level = l;
        uni_nf_inv_amplitude = 1.0f;
        glBindImageTexture(2, normal_tex_id, l, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glDispatchCompute(tex_res_x[l] >> 3, tex_res_y[l] >> 3, 1);
    }

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    //===================================================================================================================================================================================================================
    // extend normals with normal extension filter
    //===================================================================================================================================================================================================================
    extension_filter.enable();
    uniform_t uni_ef_normal_tex   = extension_filter["normal_tex"];
    uniform_t uni_ef_normal_image = extension_filter["normal_ext_image"];
    uniform_t uni_ef_tex_level    = extension_filter["tex_level"];

    uni_ef_normal_tex = 2;
    uni_ef_normal_image = 3;

    for (int l = 0; l < MAX_LOD; ++l)
    {
        uni_ef_tex_level = l;
        glBindImageTexture(3, normal_ext_tex_id, l, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glDispatchCompute(tex_res_x[l] >> 3, tex_res_y[l] >> 3, 1);
    }

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    //===================================================================================================================================================================================================================
    // compute normal map from luminosity by using Sobel/Scharr derivative filters
    //===================================================================================================================================================================================================================
    level_combine_filter.enable();
    uniform_t uni_lc_normal_ext_tex = normal_filter["normal_ext_tex"];
    uniform_t uni_lc_normal_combined_image = normal_filter["normal_combined_image"];

    uni_lc_normal_ext_tex = 3;
    uni_lc_normal_combined_image = 4;

    glDispatchCompute(tex_res_x[0] >> 3, tex_res_y[0] >> 3, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glActiveTexture(GL_TEXTURE4);
    glGenerateMipmap(GL_TEXTURE_2D);

    //===================================================================================================================================================================================================================
    // quad rendering shader and fake VAO for rendering quads
    //===================================================================================================================================================================================================================
    glsl_program_t quad_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/quad.fs"));
    quad_renderer.enable();
    uniform_t uni_qr_teximage = quad_renderer["teximage"];
    uniform_t uni_qr_texlevel = quad_renderer["texlevel"];

    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    //===================================================================================================================================================================================================================
    // phong lighting model shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t simple_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_light.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_light.fs"));

    simple_light.enable();

    uniform_t uni_sl_projection_matrix = simple_light["projection_matrix"];
    uniform_t uni_sl_view_matrix       = simple_light["view_matrix"];      
    uniform_t uni_sl_time              = simple_light["time"];             
    uniform_t uni_sl_light_ws          = simple_light["light_ws"];         
    uniform_t uni_sl_camera_ws         = simple_light["camera_ws"];         

    simple_light["solid_scale"] = 1.0f;
    simple_light["diffuse_tex"] = 0;
    simple_light["normal_tex"] = 4;

    glm::vec4 shift_rotor[128];

    float middle = 1.5f;
    float cell_size = 2.0f;
    int index = 0;
    for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) for (int k = 0; k < 4; ++k)
    {
        shift_rotor[index++] = glm::vec4(cell_size * glm::vec3(float(i) - middle, float(j) - middle, float(k) - middle), 0.0f);
        shift_rotor[index++] = glm::vec4(glm::sphericalRand(1.0f), glm::gaussRand(0.0f, 1.0f));
    }

    simple_light["shift_rotor"] = shift_rotor;

    //===================================================================================================================================================================================================================
    // initialize buffers : vertex + tangent frame + texture coordinates 
    // for five regular plato solids and load the diffuse + bump textures for each
    //===================================================================================================================================================================================================================
    polyhedron cube;
    cube.regular_pft2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces);

    //===================================================================================================================================================================================================================
    // global GL settings :
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    float t = 0.0f;

    const int half_res_x = res_x / 2;
    const int half_res_y = res_y / 2;
    const int margin_x = 16;
    const int margin_y = half_res_y - half_res_x / 2 + margin_x;

    const glm::ivec4 left_vp  = glm::ivec4(margin_x, margin_y, half_res_x - 2 * margin_x, res_y - 2 * margin_y);
    const glm::ivec4 right_vp = glm::ivec4(half_res_x + margin_x, margin_y, half_res_x - 2 * margin_x, res_y - 2 * margin_y);

    window.camera.infinite_perspective(constants::two_pi / 6.0f, 1.0, 0.5f);
    uni_sl_projection_matrix = window.camera.projection_matrix;

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
        glViewport(left_vp.x, left_vp.y, left_vp.z, left_vp.w);
        quad_renderer.enable();
        glBindVertexArray(vao_id);

        uni_qr_teximage = window.texture;
        uni_qr_texlevel = window.level;

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glViewport(right_vp.x, right_vp.y, right_vp.z, right_vp.w);
        simple_light.enable();

        if (!window.pause)
            t += window.frame_dt;

        uni_sl_time = t;
        uni_sl_view_matrix = window.camera.view_matrix;
    
        glm::vec3 light_ws = cell_size * glm::vec3(glm::cos(0.25f * t), glm::sin(0.25f * t), 2.0f);
        uni_sl_light_ws = light_ws;

        glm::vec3 camera_ws = window.camera.position();
        uni_sl_camera_ws = camera_ws;

        cube.instanced_render(64);

        window.end_frame();

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

    GLuint textures[] = {diffuse_tex_id, luma_tex_id, normal_tex_id, normal_ext_tex_id, normal_combined_tex_id};
    glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}