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
#include "gl_aux.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "polyhedron.hpp"
#include "plato.hpp"

const int MAX_LOD = 5;

struct normalmap_params_t
{
    int luma_subroutine;                                /* luminosity computation subroutine index */
    float gamma;                                        /* gamma parameter used in luminosity conversion */
    float brightness;                                   /* brightness factor used in luminosity conversion */

    int derivative_subroutine;                          /* derivative subroutine index */
    float amplitude;                                    /* amplitude for initial normal calculation */
    float radius;                                       /* normal extension radius, in pixels */
    float sharpness;                                    /* normal extension sharpness */
    float lod_intensity[MAX_LOD];                       /* intensities of the LOD input into final blended normal */

    normalmap_params_t()
    {
        luma_subroutine = 0;
        gamma = 2.2f;
        brightness = 1.0f;

        derivative_subroutine = 0;
        amplitude = 1.0f;

        radius = 3.0f;
        sharpness = 1.0f;
        for(int i = 0; i < MAX_LOD; ++i)
            lod_intensity[i] = 0.0f;
    }
};

struct subroutine_t
{
    const char* name;
    const char* description;
    GLint index;
};

subroutine_t luma_subroutines[] = 
{
    {"luma_bt709",      "BT.709 luminosity function"},
    {"luma_bt709_gc",   "BT.709 luminosity function with gamma-correction"},
    {"luma_max",        "L = max(r, g, b) luminosity function"},
    {"luma_max_gc",     "L = max(r, g, b) luminosity function with gamma-correction"},
    {"luma_product",    "L = 1 - (1 - r)*(1 - g)*(1 - b) luminosity function"},
    {"luma_product_gc", "L = 1 - (1 - r)*(1 - g)*(1 - r) luminosity function with gc"},
    {"luma_laplace",    "Laplace filter"},
    {"luma_laplace_gc", "Laplace filter with gamma-correction"}
};

const int LUMA_SUBROUTINES = sizeof(luma_subroutines) / sizeof(subroutine_t);

subroutine_t derivative_subroutines[] = 
{
    {"symm_diff",  "Symmetric difference derivative routine (4 texture fetches)"},
    {"sobel3x3",   "Sobel 3x3 filter (8 fetches)"},
    {"sobel5x5",   "Sobel 5x5 filter (24 fetches)"},
    {"scharr3x3",  "Scharr 3x3 filter (8)"},
    {"scharr5x5",  "Scharr 5x5 filter (24)"},
    {"prewitt3x3", "Prewitt 3x3 filter (8)"},
    {"prewitt5x5", "Prewitt 5x5 filter (24)"}
};

const int DERIVATIVE_SUBROUTINES = sizeof(derivative_subroutines) / sizeof(subroutine_t);


struct demo_window_t : public imgui_window_t
{
    camera_t camera;

    //===================================================================================================================================================================================================================
    // UI variables
    //===================================================================================================================================================================================================================
    bool params_changed;                                /* true if any of the normalmap generation parameters has changed */
    int texture = 0;                                    /* left rendering params :: texture and mip level to display */
    int level = 0;
    bool pause = false;
    normalmap_params_t normalmap_params;                /* normalmap generation parameters */


    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : imgui_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
          camera(8.0, 0.125, glm::mat4(1.0f))        
    {
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO | OPENGL_COMPUTE_SHADER_INFO);
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
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }

    void update_ui()
    {
        params_changed = false;

        ImGui::SetNextWindowSize(ImVec2(512, 768), ImGuiWindowFlags_NoResize | ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Normalmap generator settings", 0);
        ImGui::Text("Application average framerate (%.3f FPS)", ImGui::GetIO().Framerate);

        if (ImGui::CollapsingHeader("Luminosity filter settings"))
        {
            for(int i = 0; i < LUMA_SUBROUTINES; ++i)
                params_changed |= ImGui::RadioButton(luma_subroutines[i].description, &normalmap_params.luma_subroutine, i);
            params_changed |= ImGui::SliderFloat("Gamma", &normalmap_params.gamma, 1.0f, 4.0f, "%.3f");
            params_changed |= ImGui::SliderFloat("Brightness", &normalmap_params.brightness, 0.0f, 8.0f, "%.3f");
        }

        if (ImGui::CollapsingHeader("Initial normal computation filter settings"))
        {
            for(int i = 0; i < DERIVATIVE_SUBROUTINES; ++i)
                params_changed |= ImGui::RadioButton(derivative_subroutines[i].description, &normalmap_params.derivative_subroutine, i);
            params_changed |= ImGui::SliderFloat("Amplitude", &normalmap_params.amplitude, 0.125f, 8.0f, "%.3f");
        }

        if (ImGui::CollapsingHeader("Normal extension filter"))
        {
            params_changed |= ImGui::SliderFloat("Extension radius", &normalmap_params.radius, 1.0f, 16.0f, "%.3f");
            params_changed |= ImGui::SliderFloat("Sharpness", &normalmap_params.sharpness, 0.125f, 8.0f, "%.3f");
        }

        if (ImGui::CollapsingHeader("Mipmap levels combine filter"))
        {
            for(int i = 0; i < MAX_LOD; ++i)
            {
                char level_name[16];
                sprintf(level_name, "LOD%u intensity", i);
                params_changed |= ImGui::SliderFloat(level_name, &normalmap_params.lod_intensity[i], -1.0f, 1.0f, "%.3f");
            }
        }

        if (ImGui::CollapsingHeader("Rendering settings :: texture"))
        {
            ImGui::RadioButton("Diffuse texture",         &texture, 0);
            ImGui::RadioButton("Luminosity texture",      &texture, 1);
            ImGui::RadioButton("Initial normal texture",  &texture, 2);
            ImGui::RadioButton("Extended normal texture", &texture, 3);
            ImGui::RadioButton("Combined normal texture", &texture, 4);
        }

        if (ImGui::CollapsingHeader("Rendering settings :: mipmap level"))
        {
            for(int i = 0; i < MAX_LOD; ++i)
            {
                char level_name[16];
                sprintf(level_name, "LOD%u", i);
                ImGui::RadioButton(level_name, &level, i);
            }
        }

        ImGui::End();
        // ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
        // ImGui::ShowTestWindow(0);            
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

struct normalmap_generator_t
{
    int input_texture;

    //===================================================================================================================================================================================================================
    // compute shaders
    //===================================================================================================================================================================================================================
    glsl_program_t luminosity_filter;
    glsl_program_t normal_filter;
    glsl_program_t extension_filter;
    glsl_program_t level_combine_filter;

    //===================================================================================================================================================================================================================
    // luminosity shader uniforms
    //===================================================================================================================================================================================================================
    uniform_t uni_lf_diffuse_tex,
              uni_lf_luma_image,
              uni_lf_tex_level,
              uni_lf_gamma,
              uni_lf_brightness;

    GLuint luma_subroutine_index[LUMA_SUBROUTINES];

    //===================================================================================================================================================================================================================
    // normal compute shader uniforms
    //===================================================================================================================================================================================================================
    uniform_t uni_nf_luma_tex,
              uni_nf_inv_amplitude,
              uni_nf_normal_image,
              uni_nf_tex_level;

    GLuint derivative_subroutine_index[DERIVATIVE_SUBROUTINES];

    //===================================================================================================================================================================================================================
    // normal extension compute shader uniforms
    //===================================================================================================================================================================================================================
    uniform_t uni_ef_normal_tex,
              uni_ef_normal_image,
              uni_ef_tex_level,
              uni_ef_radius,
              uni_ef_sharpness;

    //===================================================================================================================================================================================================================
    // normal combine compute shader uniforms
    //===================================================================================================================================================================================================================
    uniform_t uni_lc_normal_ext_tex,
              uni_lc_normal_combined_image,
              uni_lc_lod_intensity;

    GLsizei tex_res_x[MAX_LOD], tex_res_y[MAX_LOD];

    GLuint luma_tex_id, normal_tex_id, normal_ext_tex_id, normal_combined_tex_id;

    normalmap_generator_t(GLuint input_texture) :
        input_texture(input_texture),
        luminosity_filter (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/luminosity_filter.cs")),
        normal_filter (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/normal_filter.cs")),
        extension_filter (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/extension_filter.cs")),
        level_combine_filter (glsl_shader_t(GL_COMPUTE_SHADER, "glsl/level_combine_filter.cs"))
    {
        uni_lf_diffuse_tex   = luminosity_filter["diffuse_tex"];
        uni_lf_luma_image    = luminosity_filter["luma_image"];
        uni_lf_tex_level     = luminosity_filter["tex_level"];
        uni_lf_gamma         = luminosity_filter["gamma"];
        uni_lf_brightness    = luminosity_filter["brightness"];

        for(int i = 0; i < LUMA_SUBROUTINES; ++i)
            luma_subroutine_index[i] = luminosity_filter.subroutine_index(GL_COMPUTE_SHADER, luma_subroutines[i].name);

        uni_nf_luma_tex      = normal_filter["luma_tex"];
        uni_nf_inv_amplitude = normal_filter["inv_amplitude"];
        uni_nf_normal_image  = normal_filter["normal_image"];
        uni_nf_tex_level     = normal_filter["tex_level"];

        for(int i = 0; i < DERIVATIVE_SUBROUTINES; ++i)
            derivative_subroutine_index[i] = normal_filter.subroutine_index(GL_COMPUTE_SHADER, derivative_subroutines[i].name);

        uni_ef_normal_tex       = extension_filter["normal_tex"];
        uni_ef_normal_image     = extension_filter["normal_ext_image"];
        uni_ef_tex_level        = extension_filter["tex_level"];
        uni_ef_radius           = extension_filter["radius"];
        uni_ef_sharpness        = extension_filter["sharpness"];

        uni_lc_normal_ext_tex        = level_combine_filter["normal_ext_tex"];
        uni_lc_normal_combined_image = level_combine_filter["normal_combined_image"];
        uni_lc_lod_intensity         = level_combine_filter["lod_intensity"];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, input_texture);

        for (int l = 0; l < MAX_LOD; ++l)
        {
            glGetTexLevelParameteriv(GL_TEXTURE_2D, l, GL_TEXTURE_WIDTH,  &tex_res_x[l]);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, l, GL_TEXTURE_HEIGHT, &tex_res_y[l]);
            debug_msg("\tLevel %u size = %u x %u", l, tex_res_x[l], tex_res_y[l]);
        }

        luma_tex_id            = generate_mipmap_texture(GL_TEXTURE1, tex_res_x[0], tex_res_y[0], GL_R32F,    MAX_LOD);
        normal_tex_id          = generate_mipmap_texture(GL_TEXTURE2, tex_res_x[0], tex_res_y[0], GL_RGBA32F, MAX_LOD);
        normal_ext_tex_id      = generate_mipmap_texture(GL_TEXTURE3, tex_res_x[0], tex_res_y[0], GL_RGBA32F, MAX_LOD);
        normal_combined_tex_id = generate_mipmap_texture(GL_TEXTURE4, tex_res_x[0], tex_res_y[0], GL_RGBA32F, MAX_LOD);
    }

    //===================================================================================================================================================================================================================
    // compute luminosity texture for each mip level
    //===================================================================================================================================================================================================================
    void compute_luminosity(const normalmap_params_t& params)
    {
        luminosity_filter.enable();
        uni_lf_diffuse_tex = 0;
        uni_lf_luma_image = 1;
        uni_lf_gamma = params.gamma;
        uni_lf_brightness = params.brightness;

        uniform_t::subroutine(GL_COMPUTE_SHADER, &luma_subroutine_index[params.luma_subroutine]);

        for (int l = 0; l < MAX_LOD; ++l)
        {
            uni_lf_tex_level = l;
            glBindImageTexture(1, luma_tex_id, l, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
            glDispatchCompute((tex_res_x[l] + 7) >> 3, (tex_res_y[l] + 7) >> 3, 1);
        }
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    //===================================================================================================================================================================================================================
    // compute normal texture for each mip level
    //===================================================================================================================================================================================================================
    void compute_normals(const normalmap_params_t& params)
    {
        normal_filter.enable();
        uni_nf_luma_tex = 1;
        uni_nf_normal_image = 2;
        uni_nf_inv_amplitude = 1.0f / params.amplitude;

        uniform_t::subroutine(GL_COMPUTE_SHADER, &derivative_subroutine_index[params.derivative_subroutine]);

        for (int l = 0; l < MAX_LOD; ++l)
        {
            uni_nf_tex_level = l;
            glBindImageTexture(2, normal_tex_id, l, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glDispatchCompute((tex_res_x[l] + 7) >> 3, (tex_res_y[l] + 7) >> 3, 1);
        }

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    //===================================================================================================================================================================================================================
    // extend normals with normal extension filter
    //===================================================================================================================================================================================================================
    void extend_normals(const normalmap_params_t& params)
    {
        extension_filter.enable();
        uni_ef_normal_tex = 2;
        uni_ef_normal_image = 3;
        uni_ef_radius = params.radius;
        uni_ef_sharpness = params.sharpness;

        for (int l = 0; l < MAX_LOD; ++l)
        {
            uni_ef_tex_level = l;
            glBindImageTexture(3, normal_ext_tex_id, l, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glDispatchCompute((tex_res_x[l] + 7) >> 3, (tex_res_y[l] + 7) >> 3, 1);
        }

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    //===================================================================================================================================================================================================================
    // mipmap levels combine filter
    //===================================================================================================================================================================================================================
    void combine_mip_levels(const normalmap_params_t& params)
    {
        level_combine_filter.enable();
        uni_lc_normal_ext_tex = 3;
        uni_lc_normal_combined_image = 4;
        uni_lc_lod_intensity = params.lod_intensity;

        glBindImageTexture(4, normal_combined_tex_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glDispatchCompute((tex_res_x[0] + 7) >> 3, (tex_res_y[0] + 7) >> 3, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glActiveTexture(GL_TEXTURE4);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void process(const normalmap_params_t& params)
    {
        compute_luminosity(params);
        compute_normals(params);
        extend_normals(params);
        combine_mip_levels(params);
    }

    ~normalmap_generator_t()
    {
        GLuint tex_ids[] = {luma_tex_id, normal_tex_id, normal_ext_tex_id, normal_combined_tex_id};
        glDeleteTextures(4, tex_ids);
    }
};

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
    // Unit 1 : luminosity texture, GL_R32F
    // Unit 2 : normal texture, GL_RGBA32F
    // Unit 3 : extended normal texture, GL_RGBA32F
    // Unit 4 : combined normal texture, GL_RGBA32F
    //===================================================================================================================================================================================================================

    glActiveTexture(GL_TEXTURE0);
    GLuint diffuse_tex_id = image::png::texture2d("../../../resources/tex2d/rock2.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT);

    GLint internal_format;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);
    debug_msg("Texture internal format is %u. Format name = %s", internal_format, gl_aux::internal_format_name(internal_format));

    normalmap_generator_t normalmap_generator(diffuse_tex_id);
    normalmap_generator.process(window.normalmap_params);

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
        // if some parameters have changed in ui regenerate normal map
        //===============================================================================================================================================================================================================
        if (window.params_changed)
            normalmap_generator.process(window.normalmap_params);

        //===============================================================================================================================================================================================================
        // wait for the compute shader to finish its work and show texture
        //===============================================================================================================================================================================================================

        /*
        int tex_res_x = normalmap_generator.tex_res_x[window.level];
        int tex_res_y = normalmap_generator.tex_res_y[window.level];

        glm::ivec4 tex_vp;
        if ((left_vp.w > tex_res_x) && (left_vp.z > tex_res_y))
        {
            int tex_margin_x = margin_x + glm::min(left_vp.w - tex_res_x, left_vp.z - tex_res_y) / 2;
            int tex_margin_y = half_res_y - half_res_x / 2 + tex_margin_x;
            tex_vp = glm::ivec4(tex_margin_x, tex_margin_y, half_res_x - 2 * tex_margin_x, res_y - 2 * tex_margin_y);
        }
        else
            tex_vp = left_vp;
        */

        glm::ivec4 tex_vp = left_vp;
        glViewport(tex_vp.x, tex_vp.y, tex_vp.z, tex_vp.w);
        quad_renderer.enable();
        glBindVertexArray(vao_id);

        uni_qr_teximage = window.texture;
        uni_qr_texlevel = window.level;

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // show cubes using created normal texture
        //===============================================================================================================================================================================================================
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

        //===============================================================================================================================================================================================================
        // After end_frame call ::
        //  - GL_DEPTH_TEST is disabled
        //  - GL_CULL_FACE is disabled
        //  - GL_SCISSOR_TEST is enabled
        //  - GL_BLEND is enabled -- blending mode GL_SRC_ALPHA/GL_ONE_MINUS_SRC_ALPHA with blending function GL_FUNC_ADD
        //  - VAO binding is destroyed
        //===============================================================================================================================================================================================================
        window.end_frame();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_BLEND);
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glDeleteTextures(1, &diffuse_tex_id);
    glfw::terminate();
    return 0;
}