//========================================================================================================================================================================================================================
// DEMO 068: Ambient Occlusion Effect Shader
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <random>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/noise.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "imgui_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "plato.hpp"
#include "vertex.hpp"
#include "vao.hpp"
#include "tess.hpp"
#include "attribute.hpp"


std::default_random_engine generator;
std::normal_distribution<float> gaussRand(0.0, 1.0);

const int res_x = 1920;
const int res_y = 1080;

struct demo_window_t : public imgui_window_t
{
    camera_t camera;
    int draw_mode = 0;
    bool blur_enabled = true;    

    GLuint64 ts[5];
    GLuint queryID[5];

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : imgui_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.5f);
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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

        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE))
            draw_mode = (draw_mode + 1) % 5;

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
            blur_enabled = !blur_enabled;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }

    //===================================================================================================================================================================================================================
    // UI data
    //===================================================================================================================================================================================================================
    char fps_str[32];
    bool show_test_window = true;
    bool show_another_window = true;
    int e = 0;
    float f1 = 0.123f, f2 = 0.0f;
    bool blur0 = true;
    bool blur1 = true;

    void update_ui() override
    {
        sprintf(fps_str, "Average FPS: %2.1f", fps());
        set_title(fps_str);

        if (show_another_window)
        {
            ImGui::SetNextWindowSize(ImVec2(512, 768), ImGuiWindowFlags_NoResize | ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Ambient Occlusion", &show_another_window);

            double FPS = fps();
            ImGui::Text("Application average %.3f ms/frame (%.3f FPS)", 1000.0f / FPS, FPS);
            ImGui::Text("Blur enabled : %s", blur_enabled ? "true" : "false");

            double geometry_time = (ts[1] - ts[0]) / 1000000.0;
            double ao_time =       (ts[2] - ts[1]) / 1000000.0;
            double blur_time =     (ts[3] - ts[2]) / 1000000.0;
            double lighting_time = (ts[4] - ts[3]) / 1000000.0;
            double total_time =    (ts[4] - ts[0]) / 1000000.0;

            ImGui::Text("Geometry pass : %2.3f ms", geometry_time);
            ImGui::Text("AO compute : %2.3f ms", ao_time);
            ImGui::Text("Blur pass : %2.3f ms", blur_time);
            ImGui::Text("Lighting pass : %2.3f ms", lighting_time);
            ImGui::Text("Total time : %2.3f ms", total_time);

            if (ImGui::CollapsingHeader("Algorithms"))
            {
                ImGui::RadioButton("World-Space AO with normal + distance textures", &e, 0);
                ImGui::RadioButton("Camera-Space AO with normal + distance textures", &e, 1);
            }


            if (ImGui::CollapsingHeader("Algorithm settings"))
            {
                switch(e)
                {
                    case 0:
                        ImGui::SliderFloat("Radius", &f1,   0.0f,  1.0f, "%.4f");
                        ImGui::SliderFloat("Bias",   &f2, -10.0f, 10.0f, "%.4f");
                        ImGui::Checkbox("Use Blur", &blur0);
                    break;

                    case 1:
                        ImGui::SliderFloat("Radius", &f1, 0.0f, 1.0f, "ratio = %.3f");
                        ImGui::SliderFloat("Bias", &f2, -10.0f, 10.0f, "%.4f", 3.0f);
                        ImGui::Checkbox("Use Blur", &blur1);
                    break;

                    default:
                    break;
                }
            }

            ImGui::End();
        }

        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
            ImGui::ShowTestWindow(&show_test_window);            
        }
    }
};


void check_status()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (GL_FRAMEBUFFER_COMPLETE == status)
    {
        debug_msg("GL_FRAMEBUFFER is COMPLETE.");
        return;
    }

    const char * msg;   
    switch (status)
    {
        case GL_FRAMEBUFFER_UNDEFINED:                     msg = "GL_FRAMEBUFFER_UNDEFINED."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         msg = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: msg = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        msg = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        msg = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER."; break;
        case GL_FRAMEBUFFER_UNSUPPORTED:                   msg = "GL_FRAMEBUFFER_UNSUPPORTED."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        msg = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      msg = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS."; break;
      default:
        msg = "Unknown Framebuffer error.";
    }

    exit_msg("FBO incomplete : %s", msg);
}

//=======================================================================================================================================================================================================================
// Setup 5 :: framebuffer with a single depth
//=======================================================================================================================================================================================================================
struct fbo_depth_t
{
    GLsizei res_x, res_y;

    GLuint fbo_id;
    GLuint texture_id;
    
    fbo_depth_t(GLsizei res_x, GLsizei res_y, GLenum internal_format, GLint wrap_mode, GLenum texture_unit)
        : res_x(res_x), res_y(res_y)
    {
        debug_msg("Creating FBO with one %dx%d depth attachment. Internal format :: %u", res_x, res_y, internal_format);

        glGenFramebuffers(1, &fbo_id);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

        glActiveTexture(texture_unit);
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
    
        glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, res_x, res_y);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_id, 0);

        check_status();
    }
    
    void bind()
        { glBindFramebuffer(GL_FRAMEBUFFER, fbo_id); }
    
    ~fbo_depth_t() 
    {
        glDeleteTextures(1, &texture_id);
        glDeleteFramebuffers(1, &fbo_id);
    }
};
/*
float factor(const glm::vec3& v)
{
    float q1 = glm::sqrt(glm::abs(0.5f - glm::simplex( 2.0f * v)));
    float q2 = glm::sqrt(glm::abs(0.5f - glm::simplex( 5.0f * v)));
    float q3 = glm::sqrt(glm::abs(0.5f - glm::simplex( 9.0f * v)));
    float q4 = glm::sqrt(glm::abs(0.5f - glm::simplex(17.0f * v)));
    return 0.15f * (q1 + 0.5 * q2 + 0.25 * q3 + 0.125 * q4);    
}
*/
float factor(const glm::vec3& v)
{
    return 0.125 * gaussRand(generator);

    float q1 = glm::abs(0.5f - glm::simplex( 2.0f * v));
    float q2 = glm::abs(0.5f - glm::simplex( 5.0f * v));
    float q3 = glm::abs(0.5f - glm::simplex( 9.0f * v));
    float q4 = glm::abs(0.5f - glm::simplex(17.0f * v));
    return 0.15f * (q1 + 0.5 * q2 + 0.25 * q3 + 0.125 * q4);    
}

vertex_pn_t cube_face_tess_func (const vertex_pn_t& A, const vertex_pn_t& B, const vertex_pn_t& C, const glm::vec3& uvw)
{
    vertex_pn_t vertex;
    vertex.position = uvw.x * A.position + uvw.y * B.position + uvw.z * C.position;
    vertex.normal = glm::normalize(uvw.x * A.normal + uvw.y * B.normal + uvw.z * C.normal);
    vertex.position += factor(vertex.position) * vertex.normal; 
    return vertex;
}

vertex_pn_t cube_edge_tess_func (const vertex_pn_t& A, const vertex_pn_t& B, const glm::vec2& uv)
{
    vertex_pn_t vertex;
    vertex.position = uv.x * A.position + uv.y * B.position;
    vertex.normal = glm::normalize(uv.x * A.normal + uv.y * B.normal);
    vertex.position += factor(vertex.position) * vertex.normal; 
    return vertex;
}



//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("AO via Compute Shader", 4, 4, 3, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // generate SSAO sample kernel points
    //===================================================================================================================================================================================================================
    glm::vec4 ssao_kernel[32];

    for (GLuint i = 0; i < 32; ++i)
    {
        glm::vec3 v = glm::normalize(glm::vec3(gaussRand(generator), gaussRand(generator), gaussRand(generator)));
        if (v.z < 0) v.z = -v.z;
        ssao_kernel[i] = glm::vec4(v, 0.125 * glm::abs(gaussRand(generator)));
    }

    //===================================================================================================================================================================================================================
    // camera params
    //===================================================================================================================================================================================================================
    glm::mat4& projection_matrix = window.camera.projection_matrix;
    glm::vec2 inv_focal_scale = glm::vec2(projection_matrix[0][0], projection_matrix[1][1]);
    glm::vec2 focal_scale = 1.0f / inv_focal_scale;

    //===================================================================================================================================================================================================================
    // compile shaders and load static uniforms
    //===================================================================================================================================================================================================================
    glsl_program_t geometry_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao_geometry.vs"), 
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_geometry.fs"));
    geometry_pass.enable();
    uniform_t uni_gp_pvm_matrix = geometry_pass["pvm_matrix"];

    glsl_program_t ssao_cs(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/ssao.cs"));
    ssao_cs.enable();
    ssao_cs["depth_tex"] = 1;
    ssao_cs["resolution"] = glm::vec2(res_x, res_y);
    ssao_cs["texel_size"] = glm::vec4(1.0f / res_x, 1.0f / res_y, -1.0f / res_x, -1.0f / res_y);
    ssao_cs["focal_scale"] = focal_scale;
    ssao_cs["inv_focal_scale"] = inv_focal_scale;
    ssao_cs["samples"] = ssao_kernel;

    glsl_program_t hblur_cs(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/hblur.cs"));
    hblur_cs.enable();
    hblur_cs["data_in"] = 2;
    hblur_cs["texel_size"] = glm::vec3(1.0f / res_x, 1.0f / res_y, 0.0f);

    glsl_program_t vblur_cs(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/vblur.cs"));
    vblur_cs.enable();
    vblur_cs["data_in"] = 3;
    vblur_cs["texel_size"] = glm::vec3(1.0f / res_x, 1.0f / res_y, 0.0f);

    glsl_program_t lighting_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao_lighting.vs"), 
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_lighting.fs"));
    lighting_pass.enable();
    lighting_pass["ssao_blurred_tex"] = 4;
    lighting_pass["tb_tex"]           = 5;

    uniform_t uni_lp_pvm_matrix       = lighting_pass["pvm_matrix"];
    uniform_t uni_lp_model_matrix     = lighting_pass["model_matrix"];
    uniform_t uni_lp_normal_matrix    = lighting_pass["normal_matrix"];
    uniform_t uni_lp_camera_ws        = lighting_pass["camera_ws"];
    uniform_t uni_lp_light_ws         = lighting_pass["light_ws"];
    uniform_t uni_lp_draw_mode        = lighting_pass["draw_mode"];
    uniform_t uni_lp_Ks               = lighting_pass["Ks"];
    uniform_t uni_lp_Ns               = lighting_pass["Ns"];
    uniform_t uni_lp_bf               = lighting_pass["bf"];
    uniform_t uni_lp_tex_scale        = lighting_pass["tex_scale"];    

    //===================================================================================================================================================================================================================
    // load model
    //===================================================================================================================================================================================================================
    vao_t model;
    model.init("../../../resources/models/vao/demon.vao");
    debug_msg("VAO Loaded :: \n\tvertex_count = %d. \n\tvertex_layout = %d. \n\tindex_type = %d. \n\tprimitive_mode = %d. \n\tindex_count = %d\n\n\n", 
        model.vbo.size, model.vbo.layout, model.ibo.type, model.ibo.mode, model.ibo.size);

    vertex_pn_t initial_vertices[plato::icosahedron::V]; 
    for(GLuint v = 0; v < plato::icosahedron::V; ++v)
        initial_vertices[v] = vertex_pn_t(plato::icosahedron::vertices[v], -plato::icosahedron::vertices[v]);    

    vao_t cube_vao = tess::generate_vao_mt(initial_vertices, plato::icosahedron::V, plato::icosahedron::quads, plato::icosahedron::Q, cube_edge_tess_func, cube_face_tess_func, 128); 

    //===================================================================================================================================================================================================================
    // Texture unit 1 : depth texture
    // Texture unit 2 : ssao texture
    // Texture unit 3 : horizontally blurred ssao
    // Texture unit 3 : vertically blurred ssao
    //===================================================================================================================================================================================================================

    fbo_depth_t geometry_fbo(res_x, res_y, GL_DEPTH_COMPONENT32, GL_CLAMP_TO_EDGE, GL_TEXTURE1);

    GLuint ssao_tex_id;
    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &ssao_tex_id);
    glBindTexture(GL_TEXTURE_2D, ssao_tex_id);
    
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, res_x, res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindImageTexture(0, ssao_tex_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

    GLuint ssao_hblur_id;
    glActiveTexture(GL_TEXTURE3);
    glGenTextures(1, &ssao_hblur_id);
    glBindTexture(GL_TEXTURE_2D, ssao_hblur_id);
    
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, res_x, res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindImageTexture(1, ssao_hblur_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);


    GLuint ssao_vblur_id;
    glActiveTexture(GL_TEXTURE4);
    glGenTextures(1, &ssao_vblur_id);
    glBindTexture(GL_TEXTURE_2D, ssao_vblur_id);
    
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, res_x, res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindImageTexture(2, ssao_vblur_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);


    //===================================================================================================================================================================================================================
    // load 2D texture for trilinear blending in lighting shader
    //================================  ===================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE5);
    GLuint demon_tex_id = image::png::texture2d("../../../resources/tex2d/plumbum.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    GLuint room_tex_id = image::png::texture2d("../../../resources/tex2d/chiseled_ice.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, window.res_x, window.res_y);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(-1);

    glm::mat4 demon_model_matrix = glm::scale(glm::translate(glm::vec3(0.0, -3.0, -8.0)), glm::vec3(2.5f));
    glm::mat4 room_model_matrix = glm::scale(glm::vec3(23.5f));
    glm::mat3 demon_normal_matrix = glm::mat3(1.0f);
    glm::mat3 room_normal_matrix = glm::mat3(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while (!window.should_close())
    {
        window.new_frame();
        glGenQueries(5, window.queryID);
        glQueryCounter(window.queryID[0], GL_TIMESTAMP);

        //===============================================================================================================================================================================================================
        // 1. Update matrix and geometric data
        //===============================================================================================================================================================================================================
        float time = window.frame_ts;
        glm::vec3 light_ws = glm::vec3(3.0f * glm::cos(time), 3.0f * glm::sin(time), 7.0f);
        glm::mat4& view_matrix = window.camera.view_matrix;
        glm::mat4 projection_view_matrix = projection_matrix * view_matrix;
        glm::mat3 camera_matrix = glm::inverse(glm::mat3(view_matrix));
        glm::vec3 camera_ws = -camera_matrix * glm::vec3(view_matrix[3]);

        glm::mat4 demon_pvm_matrix = projection_view_matrix * demon_model_matrix;
        glm::mat4 room_pvm_matrix = projection_view_matrix * room_model_matrix;

        //===============================================================================================================================================================================================================
        // 2. Geometry Pass: render scene's geometry / color data into gbuffer
        //===============================================================================================================================================================================================================
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glDrawBuffer(GL_NONE);

        geometry_pass.enable();

        glCullFace(GL_BACK);
        uni_gp_pvm_matrix = demon_pvm_matrix;
        model.render();

        uni_gp_pvm_matrix = room_pvm_matrix;
        glCullFace(GL_FRONT);
        cube_vao.render();
        glQueryCounter(window.queryID[1], GL_TIMESTAMP);        

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, geometry_fbo.fbo_id);
        glBlitFramebuffer(0, 0, res_x, res_y, 0, 0, res_x, res_y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        //===============================================================================================================================================================================================================
        // 3. Compute Shader SSAO pass
        //===============================================================================================================================================================================================================
        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, geometry_fbo.texture_id);

        ssao_cs.enable();
        glDispatchCompute(res_x / 8, res_y / 8, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glQueryCounter(window.queryID[2], GL_TIMESTAMP);

        //===============================================================================================================================================================================================================
        // 4. Compute Shader blur pass
        //===============================================================================================================================================================================================================
        glActiveTexture(GL_TEXTURE4);

        if (window.blur_enabled)
        {
            hblur_cs.enable();
            glDispatchCompute(res_x / 8, res_y / 8, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            vblur_cs.enable();
            glDispatchCompute(res_x / 8, res_y / 8, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            glBindTexture(GL_TEXTURE_2D, ssao_vblur_id);
        }
        else    
            glBindTexture(GL_TEXTURE_2D, ssao_tex_id);

        glQueryCounter(window.queryID[3], GL_TIMESTAMP);


        //===============================================================================================================================================================================================================
        // 5. Lighting Pass :: deferred Blinn-Phong lighting with added screen-space ambient occlusion
        //===============================================================================================================================================================================================================
        glDrawBuffer(GL_BACK);
        glDepthFunc(GL_LEQUAL);

        lighting_pass.enable();

        uni_lp_camera_ws = camera_ws;
        uni_lp_light_ws = light_ws;
        uni_lp_draw_mode = window.draw_mode;

        glActiveTexture(GL_TEXTURE5);

        glBindTexture(GL_TEXTURE_2D, demon_tex_id);
        uni_lp_Ks = 0.92f;
        uni_lp_Ns = 24.0f;
        uni_lp_bf = 0.0427f;
        uni_lp_tex_scale = 0.1275f;
        uni_lp_pvm_matrix = demon_pvm_matrix;
        uni_lp_normal_matrix = demon_normal_matrix;
        uni_lp_model_matrix = demon_model_matrix;
        glCullFace(GL_BACK);
        model.render();

        glBindTexture(GL_TEXTURE_2D, room_tex_id);
        uni_lp_Ks = 1.14f;
        uni_lp_Ns = 80.0f;
        uni_lp_bf = 0.00625f;
        uni_lp_tex_scale = 0.0775f;
        uni_lp_pvm_matrix = room_pvm_matrix;
        uni_lp_normal_matrix = room_normal_matrix;
        uni_lp_model_matrix = room_model_matrix;
        glCullFace(GL_FRONT);
        cube_vao.render();

        glQueryCounter(window.queryID[4], GL_TIMESTAMP);

        //===============================================================================================================================================================================================================
        // 6. Record operations timings
        //===============================================================================================================================================================================================================
        bool stop = false;
        while (!stop)
        {
            stop = true;
            for(int i = 0; i < 5; ++i)
            {
                GLint available;
                glGetQueryObjectiv(window.queryID[i], GL_QUERY_RESULT_AVAILABLE, &available);
                if (!available)
                    stop = false;
            }
        }

        for(int i = 0; i < 5; ++i)
            glGetQueryObjectui64v(window.queryID[i], GL_QUERY_RESULT, &window.ts[i]);        

        //===============================================================================================================================================================================================================
        // 7. show UI and restore OpenGL setting
        //===============================================================================================================================================================================================================
        window.end_frame();

        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
        glViewport(0, 0, res_x, res_y);
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}