//========================================================================================================================================================================================================================
// DEMO 059 : Physics-Based Rendering
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "image/stb_image.h"

#include "log.hpp"
#include "gl_aux.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "vao.hpp"
#include "vertex.hpp"
#include "image.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
    }

    int env_map = 0;
    int level = 0;
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
            env_map = (env_map + 1) % 3;

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE))
            level = (level + 1) % 8;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

struct pbr_material_t
{
    GLuint albedo_map;
    GLuint normal_map;
    GLuint metallic_map;
    GLuint roughness_map;
    GLuint ao_map;
};

void RenderQuad();

struct sphere_t
{
    vao_t vao;

    sphere_t(int res_x, int res_y)
    {
        int V = (res_x + 1) * (res_y + 1);
        int I = 2 * (res_x + 1) * res_y;

        vertex_pnt2_t* vertices = (vertex_pnt2_t*) malloc(V * sizeof(vertex_pnt2_t));
        GLuint* indices = (GLuint*) malloc (I * sizeof(GLuint));

        int v = 0;
        for (int y = 0; y <= res_y; ++y)
        {
            for (int x = 0; x <= res_x; ++x)
            {
                glm::vec2 uv = glm::vec2(x, y) / glm::vec2(res_x, res_y);
                glm::vec3 position;

                float sn_y = glm::sin(uv.y * constants::pi);
                position.x = glm::cos(uv.x * constants::two_pi) * sn_y;
                position.y = glm::cos(uv.y * constants::pi);
                position.z = glm::sin(uv.x * constants::two_pi) * sn_y;

                vertices[v++] = vertex_pnt2_t(position, position, uv);
            }
        }

        bool even_row = true;
        int i = 0;
        for (int y = 0; y < res_y; ++y)
        {
            if (even_row) // even rows: y == 0, y == 2; and so on
            {
                for (int x = 0; x <= res_x; ++x)
                {
                    indices[i++] = y * (res_x + 1) + x;
                    indices[i++] = (y + 1) * (res_x + 1) + x;
                }
            }
            else
            {
                for (int x = res_x; x >= 0; --x)
                {
                    indices[i++] = (y + 1) * (res_x + 1) + x;
                    indices[i++] = y * (res_x + 1) + x;
                }
            }
            even_row = !even_row;
        }

        vao = vao_t(GL_TRIANGLE_STRIP, vertices, V, indices, I);

        free(vertices);
        free(indices);
    }

    void render()
        { vao.render(); }
};

//=======================================================================================================================================================================================================================
// lights
//=======================================================================================================================================================================================================================
const int light_count = 4;

glm::vec3 light_positions[light_count] = 
{
    glm::vec3(-10.0f,  10.0f, 10.0f),
    glm::vec3( 10.0f,  10.0f, 10.0f),
    glm::vec3(-10.0f, -10.0f, 10.0f),
    glm::vec3( 10.0f, -10.0f, 10.0f),
};

glm::vec3 light_colors[light_count] = 
{
    glm::vec3(300.0f, 300.0f,   0.0f),
    glm::vec3(300.0f,   0.0f, 300.0f),
    glm::vec3(  0.0f, 300.0f, 300.0f),
    glm::vec3(300.0f, 300.0f,   0.0f)
};



//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Physics-Based Rendering", 4, 3, 3, 1920, 1080);

    //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    //===================================================================================================================================================================================================================
    // main PBR shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t pbr_shader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/pbr.vs"),
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/pbr.fs"));
    pbr_shader.enable();
    pbr_shader["irradiance_map"] = 0;
    pbr_shader["prefilter_map"]  = 1;
    pbr_shader["brdf"]           = 2;
    pbr_shader["albedo_map"]     = 3;
    pbr_shader["normal_map"]     = 4;
    pbr_shader["metallic_map"]   = 5;
    pbr_shader["roughness_map"]  = 6;
    pbr_shader["ao_map"]         = 7;

    uniform_t uni_pbr_pv_matrix       = pbr_shader["projection_view_matrix"];
    uniform_t uni_pbr_model_matrix    = pbr_shader["model_matrix"];
    uniform_t uni_pbr_camera_ws       = pbr_shader["camera_ws"];
    uniform_t uni_pbr_light_positions = pbr_shader["light_positions"];
    uniform_t uni_pbr_light_colors    = pbr_shader["light_colors"];
    uni_pbr_light_positions           = light_positions;
    uni_pbr_light_colors              = light_colors;

    //===================================================================================================================================================================================================================
    // load PBR materials : 
    // TODO :: combine metallic + roughness + ao into a single texture
    //===================================================================================================================================================================================================================
    pbr_material_t pbr_materials[] = 
    {
        {   // rusted iron material
            .albedo_map    = image::png::texture2d("../../../resources/tex2d/pbr/rusted_iron/albedo.png"),
            .normal_map    = image::png::texture2d("../../../resources/tex2d/pbr/rusted_iron/normal.png"),
            .metallic_map  = image::png::texture2d("../../../resources/tex2d/pbr/rusted_iron/metallic.png"),
            .roughness_map = image::png::texture2d("../../../resources/tex2d/pbr/rusted_iron/roughness.png"),
            .ao_map        = image::png::texture2d("../../../resources/tex2d/pbr/rusted_iron/ao.png")
        },
        {   // gold
            .albedo_map    = image::png::texture2d("../../../resources/tex2d/pbr/gold/albedo.png"),
            .normal_map    = image::png::texture2d("../../../resources/tex2d/pbr/gold/normal.png"),
            .metallic_map  = image::png::texture2d("../../../resources/tex2d/pbr/gold/metallic.png"),
            .roughness_map = image::png::texture2d("../../../resources/tex2d/pbr/gold/roughness.png"),
            .ao_map        = image::png::texture2d("../../../resources/tex2d/pbr/gold/ao.png")
        },
        {   // grass
            .albedo_map    = image::png::texture2d("../../../resources/tex2d/pbr/grass/albedo.png"),
            .normal_map    = image::png::texture2d("../../../resources/tex2d/pbr/grass/normal.png"),
            .metallic_map  = image::png::texture2d("../../../resources/tex2d/pbr/grass/metallic.png"),
            .roughness_map = image::png::texture2d("../../../resources/tex2d/pbr/grass/roughness.png"),
            .ao_map        = image::png::texture2d("../../../resources/tex2d/pbr/grass/ao.png")
        },
        {   // plastic
            .albedo_map    = image::png::texture2d("../../../resources/tex2d/pbr/plastic/albedo.png"),
            .normal_map    = image::png::texture2d("../../../resources/tex2d/pbr/plastic/normal.png"),
            .metallic_map  = image::png::texture2d("../../../resources/tex2d/pbr/plastic/metallic.png"),
            .roughness_map = image::png::texture2d("../../../resources/tex2d/pbr/plastic/roughness.png"),
            .ao_map        = image::png::texture2d("../../../resources/tex2d/pbr/plastic/ao.png")
        },
        {
            // wall
            .albedo_map    = image::png::texture2d("../../../resources/tex2d/pbr/wall/albedo.png"),
            .normal_map    = image::png::texture2d("../../../resources/tex2d/pbr/wall/normal.png"),
            .metallic_map  = image::png::texture2d("../../../resources/tex2d/pbr/wall/metallic.png"),
            .roughness_map = image::png::texture2d("../../../resources/tex2d/pbr/wall/roughness.png"),
            .ao_map        = image::png::texture2d("../../../resources/tex2d/pbr/wall/ao.png")
        }
    };

    const int pbr_material_count = sizeof(pbr_materials) / sizeof(pbr_material_t);

    //===================================================================================================================================================================================================================
    // load the HDR environment map to unit 0
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
//    stbi_set_flip_vertically_on_load(true);
//    GLuint hdr_texture_id = image::stbi::hdr2d("../../../resources/hdr/hansaplatz_4k.hdr", 0, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
//    GLuint hdr_texture_id = image::stbi::hdr2d("../../../resources/hdr/kiara_8_sunset_4k.hdr", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE);
//    GLuint hdr_texture_id = image::stbi::hdr2d("../../../resources/hdr/newport_loft.hdr", 0, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
//    GLuint hdr_texture_id = image::stbi::hdr2d("../../../resources/hdr/park_bench_4k.hdr", 0, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    GLuint hdr_texture_id = image::stbi::hdr2d("../../../resources/hdr/rathaus_4k.hdr", 0, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);

    //===================================================================================================================================================================================================================
    // setup cubemap in unit 1 to render to and attach to framebuffer
    //===================================================================================================================================================================================================================
    const int ENV_TEX_LEVELS = 10;
    const int ENV_TEX_RESOLUTION = 1 << (ENV_TEX_LEVELS - 1);

    glActiveTexture(GL_TEXTURE1);
    GLuint environment_cubemap;
    glGenTextures(1, &environment_cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environment_cubemap);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, ENV_TEX_LEVELS, GL_RGB16F, ENV_TEX_RESOLUTION, ENV_TEX_RESOLUTION);      
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  

    //===================================================================================================================================================================================================================
    // fake VAO for full viewport quad rendering
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glsl_shader_t cubemap_vs(GL_VERTEX_SHADER, "glsl/cubemap.vs");
    glsl_shader_t cubemap_gs(GL_GEOMETRY_SHADER, "glsl/cubemap.gs");

    //===================================================================================================================================================================================================================
    // shader that converts HDR equirectangular environment map to cubemap equivalent
    //===================================================================================================================================================================================================================
    glsl_program_t hdr_projector(cubemap_vs, cubemap_gs, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/hdr_projector.fs"));

    hdr_projector.enable();
    hdr_projector["equirectangular_map"] = 0;

    //===================================================================================================================================================================================================================
    // setup framebuffer and back project equirectangular map to cubemap, then let OpenGL generate mipmaps from the first mip face
    //===================================================================================================================================================================================================================
    GLuint fbo_id;
    glGenFramebuffers(1, &fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, ENV_TEX_RESOLUTION, ENV_TEX_RESOLUTION);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, environment_cubemap, 0);
    glDrawArrays(GL_POINTS, 0, 1);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    //===================================================================================================================================================================================================================
    // create an irradiance cubemap, and re-scale capture FBO to irradiance scale
    //===================================================================================================================================================================================================================
    const int IRRADIANCE_TEX_RESOLUTION = 64;
    GLuint irradiance_cubemap;
    glGenTextures(1, &irradiance_cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_cubemap);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB32F, IRRADIANCE_TEX_RESOLUTION, IRRADIANCE_TEX_RESOLUTION);      
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //===================================================================================================================================================================================================================
    // render to irradiance cubemap by computing spherical convolution of environment map with ... kernel 
    //===================================================================================================================================================================================================================
    glsl_program_t irradiance_conv(cubemap_vs, cubemap_gs, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/irradiance_conv.fs"));

    irradiance_conv.enable();
    irradiance_conv["environment_map"] = 0;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environment_cubemap);
    glViewport(0, 0, IRRADIANCE_TEX_RESOLUTION, IRRADIANCE_TEX_RESOLUTION);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, irradiance_cubemap, 0);
    glDrawArrays(GL_POINTS, 0, 1);



    //===================================================================================================================================================================================================================
    // create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale
    //===================================================================================================================================================================================================================
    const int PREFILTER_TEX_LEVELS = 8;
    const int PREFILTER_TEX_RESOLUTION = 1 << (PREFILTER_TEX_LEVELS - 1);
    
    GLuint prefilter_cubemap;
    glGenTextures(1, &prefilter_cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_cubemap);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, PREFILTER_TEX_LEVELS, GL_RGB16F, PREFILTER_TEX_RESOLUTION, PREFILTER_TEX_RESOLUTION);      
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter to mip_linear 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //===================================================================================================================================================================================================================
    // run a quasi monte-carlo simulation on the environment lighting to create prefilter cubemap
    //===================================================================================================================================================================================================================
    glsl_program_t prefilter(cubemap_vs, cubemap_gs, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/prefilter.fs"));

    prefilter.enable();
    prefilter["environment_map"] = 0;
    uniform_t uni_pf_roughness = prefilter["roughness"];
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environment_cubemap);

    GLuint mip_dim = PREFILTER_TEX_RESOLUTION;
    for (GLuint level = 0; level < PREFILTER_TEX_LEVELS; ++level)
    {
        glViewport(0, 0, mip_dim, mip_dim);
        float roughness = (float) level / (float)(PREFILTER_TEX_LEVELS - 1);
        uni_pf_roughness = roughness;
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, prefilter_cubemap, level);
        glDrawArrays(GL_POINTS, 0, 1);
        mip_dim >>= 1;
    }

/*

    //===================================================================================================================================================================================================================
    // pbr: generate a 2D LUT from the BRDF equations used.
    //===================================================================================================================================================================================================================
    glsl_program_t brdfShader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/brdf.vs"),
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/brdf.fs"));

    GLuint brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);
    
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);                              // pre-allocate enough memory for the LUT texture.    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);                                    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    brdfShader.enable();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderQuad();



    glsl_program_t quad_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/quad.fs"));
    quad_renderer.enable();
    quad_renderer["tex2d"] = 0;



    sphere_t sphere(64, 64);
*/

    //===================================================================================================================================================================================================================
    // environmant (skybox) shader initialization
    // assumes SRGB input and does HDR tonemap and gamma correction
    //===================================================================================================================================================================================================================
    glsl_program_t env_shader(cubemap_vs,
                              glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/cubemap_render.gs"),
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/cubemap_render.fs"));
    env_shader.enable();
    env_shader["environment_map"] = 0;
    env_shader["projection_matrix"] = window.camera.projection_matrix;
    uniform_t uni_env_view_matrix = env_shader["view_matrix"];
    uniform_t uni_env_level = env_shader["level"];


    //===================================================================================================================================================================================================================
    // then before rendering, configure the viewport to the actual screen dimensions
    //===================================================================================================================================================================================================================
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window.res_x, window.res_y);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while (!window.should_close())
    {
        window.new_frame();

        GLfloat time = window.frame_ts;
        glm::vec3 camera_ws = window.camera.position();
        glm::mat4& view_matrix = window.camera.view_matrix;
        glm::mat3 view_matrix3 = glm::mat3(view_matrix);
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

/*

        //===============================================================================================================================================================================================================
        // clear the colorbuffer
        //===============================================================================================================================================================================================================
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //===============================================================================================================================================================================================================
        // render scene, supplying the convoluted irradiance map to the final shader
        //===============================================================================================================================================================================================================
        pbr_shader.enable();
        glm::mat4 view = window.camera.view_matrix;
        uni_pbr_pv_matrix = projection_view_matrix;
        uni_pbr_camera_ws = camera_ws;

        //===============================================================================================================================================================================================================
        // bind pre-computed IBL data
        //===============================================================================================================================================================================================================
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_cubemap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_cubemap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);


        glm::vec3 sphere_centers[] = 
        {
            glm::vec3(-5.0f, 0.0f, 2.0f),
            glm::vec3(-3.0f, 0.0f, 2.0f),
            glm::vec3(-1.0f, 0.0f, 2.0f),
            glm::vec3( 1.0f, 0.0f, 2.0f),
            glm::vec3( 3.0f, 0.0f, 2.0f)
        };

        //===============================================================================================================================================================================================================
        // render 5 spheres with different material textures
        //===============================================================================================================================================================================================================
        for (int m = 0; m < pbr_material_count; ++m)
        {
            glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, pbr_materials[m].albedo_map);
            glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, pbr_materials[m].normal_map);
            glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, pbr_materials[m].metallic_map);
            glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, pbr_materials[m].roughness_map);
            glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, pbr_materials[m].ao_map);
            uni_pbr_model_matrix = glm::translate(sphere_centers[m]);
            sphere.render();
        }

        //===============================================================================================================================================================================================================
        // render light source (simply re-render sphere at light positions)
        // this looks a bit off as we use the same shader, but it'll make their positions obvious and 
        // keeps the codeprint small.
        //===============================================================================================================================================================================================================
        for (GLuint i = 0; i < light_count; ++i)
        {
            glm::vec3 new_position = light_positions[i] + glm::vec3(5.0f * sin(0.5f * time), 0.0f, 0.0f);
            light_positions[i] = new_position;
            uni_pbr_model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), new_position), glm::vec3(0.5f));
            sphere.render();
        }
        uni_pbr_light_positions = light_positions;
*/
        //===============================================================================================================================================================================================================
        // render skybox (render as last to prevent overdraw)
        //===============================================================================================================================================================================================================


        env_shader.enable();
        uni_env_view_matrix = view_matrix3;
        uni_env_level = (float) window.level;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, (window.env_map == 0) ? environment_cubemap :
                                           (window.env_map == 1) ? irradiance_cubemap : prefilter_cubemap); // display irradiance map
        glDrawArrays(GL_POINTS, 0, 1);

        //===============================================================================================================================================================================================================
        // render BRDF map to screen
        // brdfShader.enable();
        // RenderQuad();
        //===============================================================================================================================================================================================================

        //brdfShader.enable();
        //RenderQuad();

        window.end_frame();
    }

    glfw::terminate();
    return 0;
}

// RenderQuad() Renders a 1x1 XY quad in NDC
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
    if (quadVAO == 0)
    {
        GLfloat quadVertices[] = {
            // Positions        // Texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // Setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}