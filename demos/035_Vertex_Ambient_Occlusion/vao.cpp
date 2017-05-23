//==============================================================================================================================================================================================
// DEMO 035: Vertex Ambient Occlusion
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "plato.hpp"
#include "image.hpp"
#include "vertex.hpp"
#include "vao.hpp"
#include "tess.hpp"
#include "attribute.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;
    float hue_shift = 0.0f;
    GLint draw_mode = 0;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if (key_state(key) == GLFW_PRESS)
        {
            if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
            else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
            else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
            else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);

        }
        
        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE))
            draw_mode = (draw_mode + 1) % 4;
        if ((key == GLFW_KEY_KP_SUBTRACT) && (action == GLFW_RELEASE))
            draw_mode = (draw_mode + 3) % 4;

        if (key == GLFW_KEY_KP_8)
        {
            hue_shift = glm::mod(hue_shift + 0.01f, 1.0f);
        }
        if (key == GLFW_KEY_KP_2)
        {
            hue_shift = glm::mod(hue_shift - 0.01f, 1.0f);
        }

    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

float factor(const glm::vec3& v)
{
    float q1 = glm::sqrt(glm::sqrt(glm::abs(0.5f - glm::simplex( 2.0f * v))));
    float q2 = glm::sqrt(glm::sqrt(glm::abs(0.5f - glm::simplex( 5.0f * v))));
    float q3 = glm::sqrt(glm::sqrt(glm::abs(0.5f - glm::simplex( 9.0f * v))));
    float q4 = glm::sqrt(glm::sqrt(glm::abs(0.5f - glm::simplex(17.0f * v))));
    return -0.15f * (q1 + 0.5 * q2 + 0.25 * q3 + 0.125 * q4);    
}

vertex_pn_t cube_face_tess_func (const vertex_pn_t& A, const vertex_pn_t& B, const vertex_pn_t& C, const glm::vec3& uvw)
{
    vertex_pnoh_t vertex;
    vertex.position = uvw.x * A.position + uvw.y * B.position + uvw.z * C.position;
    vertex.normal = glm::normalize(uvw.x * A.normal + uvw.y * B.normal + uvw.z * C.normal);
    vertex.position += factor(vertex.position) * vertex.normal; 
    return vertex;
}

vertex_pnoh_t cube_edge_tess_func (const vertex_pnoh_t& A, const vertex_pnoh_t& B, const glm::vec2& uv)
{
    vertex_pnoh_t vertex;
    vertex.position = uv.x * A.position + uv.y * B.position;
    vertex.normal = glm::normalize(uv.x * A.normal + uv.y * B.normal);
    vertex.position += factor(vertex.position) * vertex.normal; 
    return vertex;
}

vao_t load_vao(const char* file_name)
{
    vao_t vao;
    vao_t::header_t header;
    FILE* f = fopen(file_name, "rb");
    fread (&header, sizeof(vao_t::header_t), 1, f);

    assert(header.mode == GL_TRIANGLES);
    assert(header.type == GL_UNSIGNED_INT);

    glGenVertexArrays(1, &vao.id);
    glBindVertexArray(vao.id);
        
    //====================================================================================================================================================================================================================
    // calculate attribute stride size in memory
    //====================================================================================================================================================================================================================
    vao.vbo.size = header.vbo_size;
    vao.vbo.layout = vertex_pnoh_t::layout;
    glGenBuffers(1, &vao.vbo.id);
    glBindBuffer(GL_ARRAY_BUFFER, vao.vbo.id);
        
    //====================================================================================================================================================================================================================
    // set up vertex attributes layout in buffer
    //====================================================================================================================================================================================================================
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pnoh_t), (const GLvoid*) offsetof(vertex_pnoh_t, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pnoh_t), (const GLvoid*) offsetof(vertex_pnoh_t, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_pnoh_t), (const GLvoid*) offsetof(vertex_pnoh_t, occlusion));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_pnoh_t), (const GLvoid*) offsetof(vertex_pnoh_t, hue));

    vertex_pn_t* vertices = (vertex_pn_t*) malloc(header.vbo_size * sizeof(vertex_pn_t));

    //====================================================================================================================================================================================================================
    // map attribute buffer to memory and read file data directly to the address provided by OpenGL
    //====================================================================================================================================================================================================================
    glBufferData(GL_ARRAY_BUFFER, header.vbo_size * sizeof(vertex_pnoh_t), 0, GL_STATIC_DRAW);
    fread(vertices, sizeof(vertex_pn_t), header.vbo_size, f);
        
    //====================================================================================================================================================================================================================
    // map index buffer to memory and read file data directly to it
    //====================================================================================================================================================================================================================
    vao.ibo.size = header.ibo_size;
    vao.ibo.mode = header.mode;
    vao.ibo.type = header.type;


    GLuint index_size = sizeof(GLuint);    
    glGenBuffers(1, &vao.ibo.id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao.ibo.id);
    glm::uvec3* faces = (glm::uvec3*) malloc(index_size * header.ibo_size);
    GLuint F = header.ibo_size / 3;

    fread(faces, index_size, header.ibo_size, f);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size * header.ibo_size, faces, GL_STATIC_DRAW);
    fclose(f);

    //====================================================================================================================================================================================================================
    // calculate per-vertex occlusion
    //====================================================================================================================================================================================================================
    vertex_pnoh_t* vbobuf_ptr = (vertex_pnoh_t*) vbo_t::map(GL_WRITE_ONLY);

    // calculate vertex occlusion

    vbo_t::unmap();

    debug_msg("VAO Loaded :: \n\tvertex_count = %d. \n\tvertex_layout = %d. \n\tindex_type = %d. \n\tprimitive_mode = %d. \n\tindex_count = %d\n\n\n", 
              vao.vbo.size, vao.vbo.layout, vao.ibo.type, vao.ibo.mode, vao.ibo.size);
    debug_msg("Done ... \nGL_UNSIGNED_INT = %d.\nGL_TRIANGLES = %d", GL_UNSIGNED_INT, GL_TRIANGLES);

    free(vertices);
    free(faces);
    return vao;
}


//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Vertex Ambient Occlusion", 4, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // generate kernel points
    //===================================================================================================================================================================================================================
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    glm::vec3 ssaoKernel[64];
    for (GLuint i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        GLfloat scale = GLfloat(i) / 64.0f;
        scale = 0.1f + 0.9f * scale * scale;
        sample *= scale;
        ssaoKernel[i] = sample;
    }

    //===================================================================================================================================================================================================================
    // create tesselated cube
    //===================================================================================================================================================================================================================
/*
    vertex_pnoh_t initial_vertices[plato::cube::V];
 
    for(GLuint v = 0; v < plato::cube::V; ++v)
        initial_vertices[v] = vertex_pnoh_t(plato::cube::vertices[v], -plato::cube::vertices[v], 0.5f, 0.5f);    

    vao_t cube_vao = tess::generate_vao_mt(initial_vertices, plato::cube::V, plato::cube::quads, plato::cube::Q, cube_edge_tess_func, cube_face_tess_func, 256); 
*/

    vertex_pnoh_t initial_vertices[plato::icosahedron::V];
 
    for(GLuint v = 0; v < plato::icosahedron::V; ++v)
        initial_vertices[v] = vertex_pnoh_t(plato::icosahedron::vertices[v], -plato::icosahedron::vertices[v], 0.5f, 0.5f);    

    vao_t cube_vao = tess::generate_vao_mt(initial_vertices, plato::icosahedron::V, plato::icosahedron::quads, plato::icosahedron::Q, cube_edge_tess_func, cube_face_tess_func, 128); 


    //===================================================================================================================================================================================================================
    // compile shaders and load static uniforms
    //===================================================================================================================================================================================================================
    glsl_program_t shaderGeometryPass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao_geometry.vs"), 
                                      glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_geometry.fs"));
    shaderGeometryPass.enable();
    shaderGeometryPass["projection_matrix"] = window.camera.projection_matrix;

    glsl_program_t shaderLightingPass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao.vs"), 
                                      glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_lighting.fs"));
    shaderLightingPass.enable();
    shaderLightingPass["gPosition"] = 0;
    shaderLightingPass["gNormal"] = 1;
    shaderLightingPass["ssao"] = 2; 
    shaderLightingPass["light_color"] = glm::vec3(0.2, 0.2, 0.7);

    glsl_program_t shaderSSAO(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao.vs"), 
                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao.fs"));
    shaderSSAO.enable();
    shaderSSAO["gPosition"] = 0;
    shaderSSAO["gNormal"] = 1;
    shaderSSAO["texNoise"] = 2;
    shaderSSAO["projection_matrix"] = window.camera.projection_matrix;
    shaderSSAO["samples"] = ssaoKernel;

    glsl_program_t shaderSSAOBlur(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ssao.vs"), 
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_blur.fs"));

    //====================================================================================================================================================================================================================
    // Load models
    //====================================================================================================================================================================================================================
    vao_t demon_vao = load_vao("../../../resources/models/vao/demon.vao");
    vao_t king_kong_vao = load_vao("../../../resources/models/vao/king_kong.vao");
    vao_t skull_vao = load_vao("../../../resources/models/vao/skull.vao");
    // "../../../resources/models/vao/ashtray.vao";
    // "../../../resources/models/vao/bust.vao",
    // "../../../resources/models/vao/chubby_girl.vao",
    // "../../../resources/models/vao/demon.vao",    
    // "../../../resources/models/vao/dragon.vao",   
    // "../../../resources/models/vao/female_01.vao",
    // "../../../resources/models/vao/female_02.vao",
    // "../../../resources/models/vao/female_03.vao",
    // "../../../resources/models/vao/king_kong.vao",
    // "../../../resources/models/vao/predator.vao", 
    // "../../../resources/models/vao/skull.vao",    
    // "../../../resources/models/vao/trefoil.vao"


    glm::vec3 light_ws = glm::vec3(2.0, 4.0, -2.0);

    GLuint gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    GLuint gPosition, gNormal;

    //===================================================================================================================================================================================================================
    // position buffer
    //===================================================================================================================================================================================================================
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gPosition, 0);

    //===================================================================================================================================================================================================================
    // normal color buffer
    //===================================================================================================================================================================================================================
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, gNormal, 0);

    GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window.res_x, window.res_y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        exit_msg("GBuffer Framebuffer not complete!");
    //
    //===================================================================================================================================================================================================================
    // framebuffer to hold SSAO processing stage 
    //===================================================================================================================================================================================================================
    GLuint ssaoFBO, ssaoBlurFBO;
    glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    GLuint ssaoColorBuffer, ssaoColorBufferBlur;

    //===================================================================================================================================================================================================================
    // SSAO color buffer
    //===================================================================================================================================================================================================================
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        exit_msg("SSAO Framebuffer not complete!");

    //===================================================================================================================================================================================================================
    // and blur stage
    //===================================================================================================================================================================================================================
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16F, window.res_x, window.res_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        exit_msg("SSAO Blur Framebuffer not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //===================================================================================================================================================================================================================
    // noise texture
    //===================================================================================================================================================================================================================
    glm::vec3 ssaoNoise[256];
    for (GLuint i = 0; i < 256; i++)
        ssaoNoise[i] = glm::vec3 (randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);

    GLuint noiseTexture; 
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 16, 16, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    GLuint quad_vao_id;
    glGenVertexArrays(1, &quad_vao_id);        

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glEnable(GL_CULL_FACE);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while (!window.should_close())
    {
        window.new_frame();
        //===============================================================================================================================================================================================================
        // 1. Geometry Pass: render scene's geometry/color data into gbuffer
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 projection = window.camera.projection_matrix;
        glm::mat4 view_matrix = window.camera.view_matrix;
        shaderGeometryPass.enable();
        shaderGeometryPass["view_matrix"] = view_matrix;
        shaderGeometryPass["model_matrix"] = glm::scale(glm::vec3(100.0f, 100.0f, 100.0f));
        glCullFace(GL_FRONT);
        cube_vao.render();

        glCullFace(GL_BACK);

        shaderGeometryPass["model_matrix"] = glm::translate(glm::scale(glm::vec3(8.0f)), glm::vec3(0.0f, 1.0f, -6.75));
        demon_vao.render();

        glm::mat4 model_matrix = glm::rotate(-0.08f, glm::vec3(1.0, 0.0, 0.0)) * glm::rotate(constants::pi, glm::vec3(0.0, 1.0, 0.0));
        shaderGeometryPass["model_matrix"] = glm::translate(glm::scale(model_matrix, glm::vec3(0.5f)), glm::vec3(-45.0f, -80.0f, -152.25));
        king_kong_vao.render();

        model_matrix = glm::rotate(constants::half_pi, glm::vec3(0.0, 1.0, 0.0));
        shaderGeometryPass["model_matrix"] = glm::translate(glm::scale(model_matrix, glm::vec3(18.5f)), glm::vec3(0.0f, 0.0f, -3.5f));
        skull_vao.render();

        //===============================================================================================================================================================================================================
        // 2. Create SSAO texture
        //===============================================================================================================================================================================================================
        glBindVertexArray(quad_vao_id);        
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        shaderSSAO.enable();
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // 3. Blur SSAO texture to remove noise
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        shaderSSAOBlur.enable();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // 4. Lighting Pass: traditional deferred Blinn-Phong lighting now with added screen-space ambient occlusion
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderLightingPass.enable();
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);

        glm::vec3 light_cs = window.camera.view_matrix * glm::vec4(light_ws, 1.0f);
        shaderLightingPass["hue_shift"] = window.hue_shift;
        shaderLightingPass["light_cs"] = light_cs;
        shaderLightingPass["draw_mode"] = window.draw_mode;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}