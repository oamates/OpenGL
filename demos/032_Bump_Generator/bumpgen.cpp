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
    int texture = 0;

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
            if (texture > 7) texture = 0;
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,  GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R,  GL_MIRRORED_REPEAT);
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

struct gauss_kernel_t
{
    const double sigma = 4.0;
    const double gamma = 0.5 / (sigma * sigma);

    double operator () (int l)
        { return exp(-gamma * l * l); }
};

struct separable_filter_t
{
    static const int MAX_KERNEL_SIZE = 16;
    int kernel_size;
    glsl_program_t conv1d;

    uniform_t uni_kernel_size;
    uniform_t uni_radius;
    uniform_t uni_weight;
    uniform_t uni_texel_size;
    uniform_t uni_conv_axis;

    uniform_t uni_input_tex;
    uniform_t uni_conv_image;

    separable_filter_t(const char* cs_file_name)
        : conv1d(glsl_shader_t(GL_COMPUTE_SHADER, cs_file_name))
    {
        uni_kernel_size = conv1d["kernel_size"];
        uni_radius      = conv1d["radius"];
        uni_weight      = conv1d["weight"];
        uni_texel_size  = conv1d["texel_size"];
        uni_conv_axis   = conv1d["axis"];
        uni_input_tex   = conv1d["input_tex"];
        uni_conv_image  = conv1d["conv_image"];
    }

    void enable()
        { conv1d.enable(); };

    template<typename kernel_t> void set_kernel(int kernel_size)
    {
        separable_filter_t::kernel_size = kernel_size;

        kernel_t kernel;
        double weight_d[MAX_KERNEL_SIZE];
        double total_weight = 0.0;
        float weight_f[MAX_KERNEL_SIZE];
        float radius_f[MAX_KERNEL_SIZE];

        for(int p = 0; p < kernel_size; ++p)
        {
            int p2 = p + p;
            double w0 = kernel(p2);
            if (p == 0) w0 *= 0.5;
            double w1 = kernel(p2 + 1);
            double w = w0 + w1;
            total_weight += w;
            weight_d[p] = w;
            radius_f[p] = p2 + w0 / w; 
        }

        double inv_factor = 0.5 / total_weight;

        for(int p = 0; p < kernel_size; ++p)
            weight_f[p] = inv_factor * weight_d[p];

        uni_kernel_size = kernel_size;
        glUniform1fv(uni_radius.location, kernel_size, radius_f);
        glUniform1fv(uni_weight.location, kernel_size, weight_f);
    }

    void convolve(int input_tex, int output_image, int res_x, int res_y)
    {
        uni_input_tex = input_tex;
        uni_conv_image = output_image;
        glDispatchCompute(res_x >> 3, res_y >> 3, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
};


void generate_heightmap(unsigned char* rgb_data, int w, int h)
{
    unsigned int num_pixels = w * h;

    float* s = (float*)malloc(num_pixels * 3 * sizeof(float));
    float* r = (float*)malloc(num_pixels * 4 * sizeof(float));

    const float contrast = 0.05f;
    const float inv_factor = 1.0f / 127.5f;

    for(unsigned int i = 0; i < num_pixels; ++i)
    {
        s[3 * i + 0] = inv_factor * float(rgb_data[3 * i + 0]) - 1.0f;
        s[3 * i + 1] = inv_factor * float(rgb_data[3 * i + 1]) - 1.0f;
        s[3 * i + 2] = inv_factor * float(rgb_data[3 * i + 2]) - 1.0f;
    }

    memset(r, 0, num_pixels * 4 * sizeof(float));

#define S(x, y, n) s[(y) * (w * 3) + ((x) * 3) + (n)]
#define R(x, y, n) r[(y) * (w * 4) + ((x) * 4) + (n)]

    int x, y;

    /* top-left to bottom-right */
    for(x = 1; x < w; ++x)
        R(x, 0, 0) = R(x - 1, 0, 0) + S(x - 1, 0, 0);
    for(y = 1; y < h; ++y)
        R(0, y, 0) = R(0, y - 1, 0) + S(0, y - 1, 1);
    for(y = 1; y < h; ++y)
    {
        for(x = 1; x < w; ++x)
        {
            R(x, y, 0) = (R(x, y - 1, 0) + R(x - 1, y, 0) + S(x - 1, y, 0) + S(x, y - 1, 1)) * 0.5f;
        }
    }

    /* top-right to bottom-left */
    for(x = w - 2; x >= 0; --x)
        R(x, 0, 1) = R(x + 1, 0, 1) - S(x + 1, 0, 0);
    for(y = 1; y < h; ++y)
        R(0, y, 1) = R(0, y - 1, 1) + S(0, y - 1, 1);
    for(y = 1; y < h; ++y)
    {
        for(x = w - 2; x >= 0; --x)
        {
            R(x, y, 1) = (R(x, y - 1, 1) + R(x + 1, y, 1) - S(x + 1, y, 0) + S(x, y - 1, 1)) * 0.5f;
        }
    }

    /* bottom-left to top-right */
    for(x = 1; x < w; ++x)
        R(x, 0, 2) = R(x - 1, 0, 2) + S(x - 1, 0, 0);
    for(y = h - 2; y >= 0; --y)
        R(0, y, 2) = R(0, y + 1, 2) - S(0, y + 1, 1);
    for(y = h - 2; y >= 0; --y)
    {
        for(x = 1; x < w; ++x)
        {
            R(x, y, 2) = (R(x, y + 1, 2) + R(x - 1, y, 2) + S(x - 1, y, 0) - S(x, y + 1, 1)) * 0.5f;
        }
    }

    /* bottom-right to top-left */
    for(x = w - 2; x >= 0; --x)
        R(x, 0, 3) = R(x + 1, 0, 3) - S(x + 1, 0, 0);
    for(y = h - 2; y >= 0; --y)
        R(0, y, 3) = R(0, y + 1, 3) - S(0, y + 1, 1);
    for(y = h - 2; y >= 0; --y)
    {
        for(x = w - 2; x >= 0; --x)
        {
            R(x, y, 3) = (R(x, y + 1, 3) + R(x + 1, y, 3) - S(x + 1, y, 0) - S(x, y + 1, 1)) * 0.5f;
        }
    }

#undef S
#undef R

   /* accumulate, find min/max */
    float hmin =  1e10f;
    float hmax = -1e10f;
    for(unsigned int i = 0; i < num_pixels; ++i)
    {
        r[4 * i] += r[4 * i + 1] + r[4 * i + 2] + r[4 * i + 3];
        if(r[4 * i] < hmin) hmin = r[4 * i];
        if(r[4 * i] > hmax) hmax = r[4 * i];
    }

    /* scale into 0 - 1 range */
    for(unsigned int i = 0; i < num_pixels; ++i)
    {
        float v = (r[4 * i] - hmin) / (hmax - hmin);
        /* adjust contrast */
        v = (v - 0.5f) * contrast + v;
        if(v < 0.0f) v = 0.0f;
        if(v > 1.0f) v = 1.0f;
        r[4 * i] = v;
    }

    /* write out results */
    for(unsigned int i = 0; i < num_pixels; ++i)
    {
        float v = 255.0f * r[4 * i];
        rgb_data[3 * i + 0] = (unsigned char) v;
        rgb_data[3 * i + 1] = (unsigned char) v;
        rgb_data[3 * i + 2] = (unsigned char) v;
    }

    free(s);
    free(r);
}


int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW ImGui window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Bump Generator", 4, 4, 0, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // Texture units and that will be used for bump generation
    // Unit 0 : input diffuse texture, GL_RGBA32F
    // Unit 1 : blurred diffuse texture, GL_RGBA32F
    // Unit 2 : luminosity texture, GL_R32F
    // Unit 3 : blurred luminosity texture, GL_R32F
    // Unit 4 : normal texture, GL_RGBA32F
    // Unit 5 : displacement texture, GL_R32F
    // Unit 6 : roughness texture, GL_R32F
    // Unit 7 : shininess texture, GL_R32F
    // Unit 8 : auxiliary texture, GL_RGBA32F
    // Unit 9 : auxiliary texture, GL_R32F
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint diffuse_tex_id = image::png::texture2d("../../../resources/tex2d/rock.png", 0, GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT);

    GLint tex_res_x, tex_res_y;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &tex_res_x);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tex_res_y);

    GLint internal_format;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);
    debug_msg("Texture internal format is %u. Format name = %s", internal_format, internal_format_name(internal_format));

    GLuint blurred_tex_id      = generate_texture(GL_TEXTURE1, tex_res_x, tex_res_y, GL_RGBA32F);
    GLuint luma_tex_id         = generate_texture(GL_TEXTURE2, tex_res_x, tex_res_y, GL_R32F);
    GLuint luma_blurred_tex_id = generate_texture(GL_TEXTURE3, tex_res_x, tex_res_y, GL_R32F);
    GLuint normal_tex_id       = generate_texture(GL_TEXTURE4, tex_res_x, tex_res_y, GL_RGBA32F);
    GLuint displacement_tex_id = generate_texture(GL_TEXTURE5, tex_res_x, tex_res_y, GL_R32F);
    GLuint roughness_tex_id    = generate_texture(GL_TEXTURE6, tex_res_x, tex_res_y, GL_R32F);
    GLuint shininess_tex_id    = generate_texture(GL_TEXTURE7, tex_res_x, tex_res_y, GL_R32F);
    GLuint aux_rgba_tex_id     = generate_texture(GL_TEXTURE8, tex_res_x, tex_res_y, GL_RGBA32F);
    GLuint aux_r_tex_id        = generate_texture(GL_TEXTURE9, tex_res_x, tex_res_y, GL_R32F);

    glBindImageTexture(0, diffuse_tex_id,      0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA32F);
    glBindImageTexture(1, blurred_tex_id,      0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(2, luma_tex_id,         0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glBindImageTexture(3, luma_blurred_tex_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glBindImageTexture(4, normal_tex_id,       0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(5, displacement_tex_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glBindImageTexture(6, roughness_tex_id,    0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
//    glBindImageTexture(7, shininess_tex_id,    0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
//    glBindImageTexture(8, aux_rgba_tex_id,     0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
//    glBindImageTexture(9, aux_r_tex_id,        0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);




    //===================================================================================================================================================================================================================
    // compute shader compilation and subroutine indices querying
    //===================================================================================================================================================================================================================
    glsl_program_t luminosity_filter  (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/luminosity_filter.cs"));
    glsl_program_t normal_filter      (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/normal_filter.cs"));
    glsl_program_t displacement_filter(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/displacement_filter.cs"));
    glsl_program_t roughness_filter   (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/roughness_filter.cs"));
    glsl_program_t shininess_filter   (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/shininess_filter.cs"));
    glsl_program_t laplace_filter     (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/laplace_filter.cs"));

    /* compute blurred diffuse map */
    float texel_size_x = 1.0f / tex_res_x;
    float texel_size_y = 1.0f / tex_res_y;
    glm::vec2 texel_size = glm::vec2(texel_size_x, texel_size_y);

    separable_filter_t gauss_filter("glsl/conv1d_filter.cs");
    gauss_filter.enable();
    gauss_filter.set_kernel<gauss_kernel_t>(2);
    gauss_filter.uni_texel_size = texel_size;
    
    gauss_filter.uni_conv_axis = glm::vec2(texel_size_x, 0.0f);
    glBindImageTexture(7, aux_rgba_tex_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    gauss_filter.convolve(0, 7, tex_res_x, tex_res_y);

    gauss_filter.uni_conv_axis = glm::vec2(0.0f, texel_size_y);
    gauss_filter.convolve(8, 1, tex_res_x, tex_res_y);

    /* compute luminosity from diffuse map */
    luminosity_filter.enable();
    uniform_t uni_lf_diffuse_tex = luminosity_filter["diffuse_tex"];
    uniform_t uni_lf_luma_image = luminosity_filter["luma_image"];
    uniform_t uni_lf_texel_size = luminosity_filter["texel_size"];

    uni_lf_diffuse_tex = 0;
    uni_lf_luma_image = 2;
    uni_lf_texel_size = texel_size;
    glDispatchCompute(tex_res_x >> 3, tex_res_y >> 3, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* compute blurred luminosity map from blurred diffuse map */
    uni_lf_diffuse_tex = 1;
    uni_lf_luma_image = 3;
    glDispatchCompute(tex_res_x >> 3, tex_res_y >> 3, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* compute normal map from luminosity by using Sobel/Scharr derivative filters */
    normal_filter.enable();
    uniform_t uni_nf_luma_tex = normal_filter["luma_tex"];
    uniform_t uni_nf_amplitude = normal_filter["amplitude"];
    uniform_t uni_nf_texel_size = normal_filter["texel_size"];
    uniform_t uni_nf_normal_image = normal_filter["normal_image"];
    uni_nf_luma_tex = 3;
    uni_nf_normal_image = 4;
    uni_nf_amplitude = 4.0f;
    uni_nf_texel_size = texel_size;
    glDispatchCompute(tex_res_x >> 3, tex_res_y >> 3, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    glActiveTexture(GL_TEXTURE4);
    uint8_t* rgb_data = (uint8_t*) malloc(3 * tex_res_x * tex_res_y);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_data);
    generate_heightmap(rgb_data, tex_res_x, tex_res_y);
    image::png::write("heightmap.png", tex_res_x, tex_res_y, rgb_data, PNG_COLOR_TYPE_RGB);
    free(rgb_data);



    /* compute displacement map from normal map */
    displacement_filter.enable();
    uniform_t uni_df_normal_tex = displacement_filter["normal_tex"];
    uniform_t uni_df_disp_tex = displacement_filter["disp_tex"];
    uniform_t uni_df_output_image = displacement_filter["output_image"];

    uniform_t uni_df_texel_size = displacement_filter["texel_size"];
    uniform_t uni_df_delta = displacement_filter["delta"];

    uni_df_normal_tex = 4;
    uni_df_texel_size = texel_size;
    const float zero = 0.0f;
    glClearTexImage(displacement_tex_id, 0, GL_RED, GL_FLOAT, &zero);
    float dx = 32.0f;

    glBindImageTexture(7, aux_r_tex_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

    for(int i = 0; i < 6; ++i)
    {
        uni_df_delta = glm::vec2(dx / tex_res_x, dx / tex_res_y);
        for(int j = 0; j < 2; ++j)
        {
            uni_df_disp_tex = 5;
            uni_df_output_image = 7;
            glDispatchCompute(tex_res_x >> 3, tex_res_y >> 3, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            uni_df_disp_tex = 9;
            uni_df_output_image = 5;
            glDispatchCompute(tex_res_x >> 3, tex_res_y >> 3, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
        dx *= 0.5;
    }

    /* compute roughness map from diffuse map */
    roughness_filter.enable();
    uniform_t uni_rf_diffuse_tex = roughness_filter["diffuse_tex"];
    uniform_t uni_rf_roughness_image = roughness_filter["roughness_image"];
    uniform_t uni_rf_texel_size = roughness_filter["texel_size"];

    uni_rf_diffuse_tex = 0;
    uni_rf_roughness_image = 6;
    uni_rf_texel_size = texel_size;

    glDispatchCompute(tex_res_x >> 3, tex_res_y >> 3, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* compute shininess map from diffuse map */
    shininess_filter.enable();
    uniform_t uni_sf_diffuse_tex = shininess_filter["diffuse_tex"];
    uniform_t uni_sf_shininess_image = shininess_filter["shininess_image"];
    uniform_t uni_sf_texel_size = shininess_filter["texel_size"];

    uni_sf_diffuse_tex = 1;
    uni_sf_shininess_image = 7;
    uni_sf_texel_size = texel_size;

    glBindImageTexture(7, shininess_tex_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glDispatchCompute(tex_res_x >> 3, tex_res_y >> 3, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    //===================================================================================================================================================================================================================
    // quad rendering shader and fake VAO for rendering quads
    //===================================================================================================================================================================================================================
    glsl_program_t quad_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/quad.fs"));
    quad_renderer.enable();

    uniform_t uniform_quad = quad_renderer["quad"];
    uniform_t uniform_teximage = quad_renderer["teximage"];

    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    //===================================================================================================================================================================================================================
    // Global GL settings :
    // DEPTH not needed, hence disabled
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

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

    GLuint textures[] = {diffuse_tex_id, blurred_tex_id, luma_tex_id, luma_blurred_tex_id, normal_tex_id, 
                         displacement_tex_id, roughness_tex_id, shininess_tex_id, aux_rgba_tex_id, aux_r_tex_id};

    glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}