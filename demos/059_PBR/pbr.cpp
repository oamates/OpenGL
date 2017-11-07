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
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

void renderSphere();
void renderCube();
void RenderQuad();

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

    // Setup OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glm::mat4 projection_matrix = window.camera.projection_matrix;

    //===================================================================================================================================================================================================================
    // lights
    //===================================================================================================================================================================================================================
    const int NR_LIGHTS = 4;
    glm::vec3 lightPositions[NR_LIGHTS] = {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3( 10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3( 10.0f, -10.0f, 10.0f),
    };
    glm::vec3 lightColors[NR_LIGHTS] = {
        glm::vec3(300.0f, 300.0f,   0.0f),
        glm::vec3(300.0f,   0.0f, 300.0f),
        glm::vec3(  0.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f,   0.0f)
    };

    //===================================================================================================================================================================================================================
    // load and initialize shaders
    //===================================================================================================================================================================================================================
    glsl_program_t pbr_shader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/pbr.vs"),
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/pbr.fs"));
    pbr_shader.enable();
    pbr_shader["irradianceMap"] = 0;
    pbr_shader["prefilterMap"]  = 1;
    pbr_shader["brdfLUT"]       = 2;
    pbr_shader["albedoMap"]     = 3;
    pbr_shader["normalMap"]     = 4;
    pbr_shader["metallicMap"]   = 5;
    pbr_shader["roughnessMap"]  = 6;
    pbr_shader["aoMap"]         = 7;

    uniform_t uni_pbr_pv_matrix = pbr_shader["projection_view_matrix"];
    uniform_t uni_pbr_model_matrix = pbr_shader["model_matrix"];
    uniform_t uni_pbr_camera_ws = pbr_shader["camera_ws"];
    uniform_t uni_pbr_lightPositions = pbr_shader["lightPositions"];
    uniform_t uni_pbr_lightColors = pbr_shader["lightColors"];
    uni_pbr_lightPositions = lightPositions;
    uni_pbr_lightColors = lightColors;

    glsl_program_t background_shader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/background.vs"),
                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/background.fs"));
    background_shader.enable();
    background_shader["environmentMap"] = 0;
    background_shader["projection_matrix"] = projection_matrix;
    uniform_t uni_bg_view_matrix = background_shader["view_matrix"];

    //===================================================================================================================================================================================================================
    // load PBR material textures
    //===================================================================================================================================================================================================================

    // rusted iron
    GLuint ironAlbedoMap       = image::png::texture2d("../../../resources/tex2d/pbr/rusted_iron/albedo.png");
    GLuint ironNormalMap       = image::png::texture2d("../../../resources/tex2d/pbr/rusted_iron/normal.png");
    GLuint ironMetallicMap     = image::png::texture2d("../../../resources/tex2d/pbr/rusted_iron/metallic.png");
    GLuint ironRoughnessMap    = image::png::texture2d("../../../resources/tex2d/pbr/rusted_iron/roughness.png");
    GLuint ironAOMap           = image::png::texture2d("../../../resources/tex2d/pbr/rusted_iron/ao.png");

    // gold
    GLuint goldAlbedoMap       = image::png::texture2d("../../../resources/tex2d/pbr/gold/albedo.png");
    GLuint goldNormalMap       = image::png::texture2d("../../../resources/tex2d/pbr/gold/normal.png");
    GLuint goldMetallicMap     = image::png::texture2d("../../../resources/tex2d/pbr/gold/metallic.png");
    GLuint goldRoughnessMap    = image::png::texture2d("../../../resources/tex2d/pbr/gold/roughness.png");
    GLuint goldAOMap           = image::png::texture2d("../../../resources/tex2d/pbr/gold/ao.png");

    // grass
    GLuint grassAlbedoMap      = image::png::texture2d("../../../resources/tex2d/pbr/grass/albedo.png");
    GLuint grassNormalMap      = image::png::texture2d("../../../resources/tex2d/pbr/grass/normal.png");
    GLuint grassMetallicMap    = image::png::texture2d("../../../resources/tex2d/pbr/grass/metallic.png");
    GLuint grassRoughnessMap   = image::png::texture2d("../../../resources/tex2d/pbr/grass/roughness.png");
    GLuint grassAOMap          = image::png::texture2d("../../../resources/tex2d/pbr/grass/ao.png");

    // plastic
    GLuint plasticAlbedoMap    = image::png::texture2d("../../../resources/tex2d/pbr/plastic/albedo.png");
    GLuint plasticNormalMap    = image::png::texture2d("../../../resources/tex2d/pbr/plastic/normal.png");
    GLuint plasticMetallicMap  = image::png::texture2d("../../../resources/tex2d/pbr/plastic/metallic.png");
    GLuint plasticRoughnessMap = image::png::texture2d("../../../resources/tex2d/pbr/plastic/roughness.png");
    GLuint plasticAOMap        = image::png::texture2d("../../../resources/tex2d/pbr/plastic/ao.png");

    // wall
    GLuint wallAlbedoMap       = image::png::texture2d("../../../resources/tex2d/pbr/wall/albedo.png");
    GLuint wallNormalMap       = image::png::texture2d("../../../resources/tex2d/pbr/wall/normal.png");
    GLuint wallMetallicMap     = image::png::texture2d("../../../resources/tex2d/pbr/wall/metallic.png");
    GLuint wallRoughnessMap    = image::png::texture2d("../../../resources/tex2d/pbr/wall/roughness.png");
    GLuint wallAOMap           = image::png::texture2d("../../../resources/tex2d/pbr/wall/ao.png");

    //===================================================================================================================================================================================================================
    // pbr: setup framebuffer
    //===================================================================================================================================================================================================================
    GLuint fbo_id;
    glGenFramebuffers(1, &fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    GLuint rbo_id;
    glGenRenderbuffers(1, &rbo_id);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_id);

    //===================================================================================================================================================================================================================
    // pbr: load the HDR environment map
    //===================================================================================================================================================================================================================
    stbi_set_flip_vertically_on_load(true);
    GLuint hdr_texture_id = image::stbi::hdr2d("../../../resources/hdr/newport_loft.hdr", 0, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);

    //===================================================================================================================================================================================================================
    // pbr: setup cubemap to render to and attach to framebuffer
    //===================================================================================================================================================================================================================
    GLuint environment_cubemap;
    glGenTextures(1, &environment_cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environment_cubemap);
    for (GLuint i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
    //===================================================================================================================================================================================================================
    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
    //===================================================================================================================================================================================================================
    const glm::mat4 hdr_projection_matrix = glm::perspective(constants::half_pi, 1.0f, 0.125f, 8.0f);
    const glm::mat4 hdr_view_matrix[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    glsl_shader_t cubemap_vertex(GL_VERTEX_SHADER, "glsl/cubemap.vs");

    //===================================================================================================================================================================================================================
    // pbr: convert HDR equirectangular environment map to cubemap equivalent and let OpenGL generate mipmaps from the first mip face
    //===================================================================================================================================================================================================================
    glsl_program_t hdr_projector(cubemap_vertex, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/hdr_projector.fs"));

    hdr_projector.enable();
    hdr_projector["equirectangularMap"] = 0;
    hdr_projector["projection_matrix"] = hdr_projection_matrix;
    uniform_t uni_hp_view_matrix = hdr_projector["view_matrix"];

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdr_texture_id);

    glViewport(0, 0, 512, 512);
    for (GLuint i = 0; i < 6; ++i)
    {
        uni_hp_view_matrix = hdr_view_matrix[i];
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environment_cubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderCube();
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, environment_cubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    //===================================================================================================================================================================================================================
    // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale
    //===================================================================================================================================================================================================================
    GLuint irradiance_cubemap;
    glGenTextures(1, &irradiance_cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_cubemap);
    for (GLuint i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, 32, 32, 0, GL_RGB, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //===================================================================================================================================================================================================================
    // pbr: solve diffuse integral by convolution to create irradiance cubemap
    //===================================================================================================================================================================================================================
    glsl_program_t irradianceShader(cubemap_vertex, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/irradiance_convolution.fs"));

    irradianceShader.enable();
    irradianceShader["environmentMap"] = 0;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environment_cubemap);
    irradianceShader["projection_matrix"] = hdr_projection_matrix;

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
    glViewport(0, 0, 32, 32);

    for (GLuint i = 0; i < 6; ++i)
    {
        irradianceShader["view_matrix"] = hdr_view_matrix[i];
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_cubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderCube();
    }

    //===================================================================================================================================================================================================================
    // pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale
    //===================================================================================================================================================================================================================
    GLuint prefilter_cubemap;
    glGenTextures(1, &prefilter_cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_cubemap);
    for (GLuint i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter to mip_linear 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    //===================================================================================================================================================================================================================
    // pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
    //===================================================================================================================================================================================================================
    glsl_program_t prefilterShader(cubemap_vertex, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/prefilter.fs"));

    prefilterShader.enable();
    prefilterShader["environmentMap"] = 0;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environment_cubemap);
    prefilterShader["projection_matrix"] = hdr_projection_matrix;

    GLuint maxMipLevels = 5;
    GLuint mipWidth = 128;
    GLuint mipHeight = 128;
    for (GLuint mip = 0; mip < maxMipLevels; ++mip)
    {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader["roughness"] = roughness;
        for (GLuint i = 0; i < 6; ++i)
        {
            prefilterShader["view_matrix"] = hdr_view_matrix[i];
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_cubemap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        }
        mipWidth >>= 1;
        mipHeight >>= 1;
    }

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


    //===================================================================================================================================================================================================================
    // then before rendering, configure the viewport to the actual screen dimensions
    //===================================================================================================================================================================================================================
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window.res_x, window.res_y);


    glsl_program_t quad_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/quad.fs"));
    quad_renderer.enable();
    quad_renderer["tex2d"] = 0;

    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while (!window.should_close())
    {
        window.new_frame();

/*
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdr_texture_id);
        glBindVertexArray(vao_id);
        quad_renderer.enable();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);        
*/

        GLfloat time = window.frame_ts;
        glm::vec3 camera_ws = window.camera.position();
        glm::mat4& view_matrix = window.camera.view_matrix;
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        //===============================================================================================================================================================================================================
        // clear the colorbuffer
        //===============================================================================================================================================================================================================
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //===============================================================================================================================================================================================================
        // render scene, supplying the convoluted irradiance map to the final shader
        //===============================================================================================================================================================================================================
        pbr_shader.enable();
        glm::mat4 model;
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

        //===============================================================================================================================================================================================================
        // rusted iron
        //===============================================================================================================================================================================================================
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, ironAlbedoMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, ironNormalMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, ironMetallicMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, ironRoughnessMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, ironAOMap);
        uni_pbr_model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, 2.0f));
        renderSphere();

        //===============================================================================================================================================================================================================
        // gold
        //===============================================================================================================================================================================================================
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, goldAlbedoMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, goldNormalMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, goldMetallicMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, goldRoughnessMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, goldAOMap);
        uni_pbr_model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 2.0f));
        renderSphere();

        //===============================================================================================================================================================================================================
        // grass
        //===============================================================================================================================================================================================================
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, grassAlbedoMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, grassNormalMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, grassMetallicMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, grassRoughnessMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, grassAOMap);
        uni_pbr_model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 2.0f));
        renderSphere();

        //===============================================================================================================================================================================================================
        // plastic
        //===============================================================================================================================================================================================================
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, plasticAlbedoMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, plasticNormalMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, plasticMetallicMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, plasticRoughnessMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, plasticAOMap);
        uni_pbr_model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 2.0f));
        renderSphere();

        //===============================================================================================================================================================================================================
        // wall
        //===============================================================================================================================================================================================================
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, wallAlbedoMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, wallNormalMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, wallMetallicMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, wallRoughnessMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, wallAOMap);
        uni_pbr_model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 2.0f));
        renderSphere();
 
        //===============================================================================================================================================================================================================
        // render light source (simply re-render sphere at light positions)
        // this looks a bit off as we use the same shader, but it'll make their positions obvious and 
        // keeps the codeprint small.
        //===============================================================================================================================================================================================================
        for (GLuint i = 0; i < NR_LIGHTS; ++i)
        {
            glm::vec3 newPos = lightPositions[i] + glm::vec3(5.0f * sin(0.5f * time), 0.0f, 0.0f);
            lightPositions[i] = newPos;
            uni_pbr_model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), newPos), glm::vec3(0.5f));
            renderSphere();
        }
        uni_pbr_lightPositions = lightPositions;

        //===============================================================================================================================================================================================================
        // render skybox (render as last to prevent overdraw)
        //===============================================================================================================================================================================================================
        background_shader.enable();
        uni_bg_view_matrix = view_matrix;
        glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, environment_cubemap);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_cubemap); // display irradiance map
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_cubemap); // display prefilter map
        renderCube();

        //===============================================================================================================================================================================================================
        // render BRDF map to screen
        // brdfShader.enable();
        // RenderQuad();
        //===============================================================================================================================================================================================================

        brdfShader.enable();
        RenderQuad();

        window.end_frame();
    }

    glfw::terminate();
    return 0;
}


GLuint sphereVAO = 0;
unsigned int indexCount;

void renderSphere()
{
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uv;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359;
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = indices.size();

        std::vector<float> data;
        for (int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        float stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(5 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}
// RenderCube() Renders a 1x1 3D cube in NDC.
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
void renderCube()
{
    // Initialize (if necessary)
    if (cubeVAO == 0)
    {
        GLfloat vertices[] = {
            // Back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,  // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,// top-left
            // Front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,  // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,  // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,  // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,  // bottom-left
            // Left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,  // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // Right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // Bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,// bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // Top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,// top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,// top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // Fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // Link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // Render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
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