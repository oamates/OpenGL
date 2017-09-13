//========================================================================================================================================================================================================================
// DEMO 011 : Rendering procedural textures
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/ext.hpp> 
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "vertex.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "plato.hpp"
#include "image.hpp"
#include "polyhedron.hpp"
#include "vao.hpp"
#include "fbo.hpp"
#include "surface.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    bool static_light = true;
    double light_ts;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        light_ts = frame_ts;
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

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE)) 
        {
            static_light = !static_light;
            light_ts = glfw::time();
        }
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

struct texture_renderer_t
{
    GLuint diffuse_texture_unit;
    GLuint normal_texture_unit;
    fbo_color_t<GL_TEXTURE_2D, 2> fbo_color;
    glsl_program_t program;
    uniform_t model_matrix;
    GLuint vao_id;

    texture_renderer_t(GLuint diffuse_texture_unit, GLuint normal_texture_unit, GLsizei res_x, GLsizei res_y, GLenum internal_format = GL_RGBA32F)
        : diffuse_texture_unit(diffuse_texture_unit),
          normal_texture_unit(normal_texture_unit),
          fbo_color(res_x, res_y, internal_format),
          program(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/texrenderer.vs"),
                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/texrenderer.fs")),
          model_matrix(program["model_matrix"])
    {
        glGenVertexArrays(1, &vao_id);
    }

    void reset_textures(GLuint diffuse_texture_unit, GLuint normal_texture_unit, GLsizei res_x, GLsizei res_y, GLenum internal_format = GL_RGBA32F)
    {
        texture_renderer_t::diffuse_texture_unit = diffuse_texture_unit;
        texture_renderer_t::normal_texture_unit  = normal_texture_unit;
        fbo_color.reset_textures(res_x, res_y, internal_format);
    }
    
    void bind()
        { fbo_color.bind(); }

    void bind_textures()
    {
        
        fbo_color.bind_texture(diffuse_texture_unit, 0);
        fbo_color.bind_texture(normal_texture_unit,  1);
    } 

    void enable()
        { program.enable(); }

    void create_textures()
    {                                                  
        fbo_color.bind();
        glBindVertexArray(vao_id);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void store_textures(const char* diffuse_texture_fname, const char* normal_texture_fname)
    {
        GLuint res_x = fbo_color.res_x;
        GLuint res_y = fbo_color.res_y;
        GLvoid* pixel_buffer = malloc (res_x * res_y * 4);
        fbo_color.bind_texture(diffuse_texture_unit, 0); 
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_buffer);
        image::png::write(diffuse_texture_fname, res_x, res_y, (unsigned char*) pixel_buffer, PNG_COLOR_TYPE_RGBA);
        fbo_color.bind_texture(normal_texture_unit,  1); 
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_buffer);
        image::png::write(normal_texture_fname,  res_x, res_y, (unsigned char*) pixel_buffer, PNG_COLOR_TYPE_RGBA);
        free(pixel_buffer);
    }

    ~texture_renderer_t()
    {
        glDeleteVertexArrays(1, &vao_id);
    }
};


vertex_pft2_t torus_func(const glm::vec2& uv)
{
    vertex_pft2_t vertex;

    float cos_2piu = glm::cos(constants::two_pi * uv.y);
    float sin_2piu = glm::sin(constants::two_pi * uv.y);
    float cos_2piv = glm::cos(constants::two_pi * uv.x);
    float sin_2piv = glm::sin(constants::two_pi * uv.x);

    float R = 2.7f;
    float r = 0.97f;

    vertex.position = glm::vec3((R + r * cos_2piu) * cos_2piv,
                                (R + r * cos_2piu) * sin_2piv, r * sin_2piu);

    vertex.tangent_x = glm::vec3(-sin_2piu * cos_2piv, -sin_2piu * sin_2piv, cos_2piu);
    vertex.tangent_y = glm::vec3(-sin_2piv, cos_2piv, 0.0f);

    vertex.normal = glm::vec3(cos_2piu * cos_2piv, cos_2piu * sin_2piv, sin_2piu);
    vertex.uv = uv;

    return vertex;
};

float P = 2.0f;
float Q = 7.0f;

vertex_pft2_t torusPQ_func(const glm::vec2& uv)
{
    vertex_pft2_t vertex;


    const float R = 3.17f;
    const float r = 1.23f;
    const float r0 = 0.82f;
    
    float csPx = glm::cos(two_pi * P * uv.x);
    float snPx = glm::sin(two_pi * P * uv.x);
    float csQx = glm::cos(two_pi * Q * uv.x);
    float snQx = glm::sin(two_pi * Q * uv.x);

    float H = R + r * csQx;
    glm::vec3 U = glm::vec3(H * csPx, H * snPx, -r * snQx);
    glm::vec3 N = glm::vec3(csQx * csPx, csQx * snPx, -snQx); 
    glm::vec3 T = glm::vec3(- H * P * snPx - Q * r * snQx * csPx,
                              H * P * csPx - Q * r * snQx * snPx,
                            - Q * r * csQx);

    T = glm::normalize(T);
    glm::vec3 B = glm::cross(N, T);

    float csY = glm::cos(two_pi * uv.y);
    float snY = glm::sin(two_pi * uv.y);


    vertex.tangent_x = T;
    vertex.normal    = - N * csY - B * snY;
    vertex.tangent_y =   N * snY - B * csY;                             
    vertex.position  = U + r0 * vertex.normal;
    vertex.uv = uv;

    return vertex;
}

struct torus_knot_t
{
    float P;
    float Q;
    GLenum diffuse_texture_unit;
    GLenum normal_texture_unit;
    GLuint diffuse_subroutine_index;
    glm::mat4 model_matrix;
    
    torus_knot_t(float P, float Q, GLenum diffuse_texture_unit, GLenum normal_texture_unit, GLuint diffuse_subroutine_index, const glm::mat4& model_matrix)
        : P(P), Q(Q), diffuse_texture_unit(diffuse_texture_unit), normal_texture_unit(normal_texture_unit),
          diffuse_subroutine_index(diffuse_subroutine_index), model_matrix(model_matrix)
    { }
};

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

    demo_window_t window("Post processing effects.", 8, 4, 0, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // fill textures with procedural data
    //===================================================================================================================================================================================================================
    glDisable(GL_DEPTH_TEST);
    texture_renderer_t texture_renderer(GL_TEXTURE0, GL_TEXTURE1, 1536, 512);
    texture_renderer.enable();

    //===================================================================================================================================================================================================================
    // get indices of procedural color functions and uniform subroutines
    //===================================================================================================================================================================================================================
    GLuint si_simplex_turbulence  = texture_renderer.program.subroutine_index(GL_FRAGMENT_SHADER, "simplex_turbulence");
    GLuint si_cellular_turbulence = texture_renderer.program.subroutine_index(GL_FRAGMENT_SHADER, "cellular_turbulence");
    GLuint si_marble_light        = texture_renderer.program.subroutine_index(GL_FRAGMENT_SHADER, "marble_light");
    GLuint si_marble_complex      = texture_renderer.program.subroutine_index(GL_FRAGMENT_SHADER, "marble_complex");
    GLuint si_torus_func          = texture_renderer.program.subroutine_index(GL_FRAGMENT_SHADER, "torus_func");
    GLuint si_torusPQ_func        = texture_renderer.program.subroutine_index(GL_FRAGMENT_SHADER, "torusPQ_func");
    GLuint su_color_func          = texture_renderer.program.subroutine_location(GL_FRAGMENT_SHADER, "color_func");
    GLuint su_surface_func        = texture_renderer.program.subroutine_location(GL_FRAGMENT_SHADER, "surface_func");


    //===================================================================================================================================================================================================================
    // set up the central torus
    //===================================================================================================================================================================================================================
    glm::mat4 torus_model_matrix = glm::mat4(1.0f);
    GLuint indices[2];
    indices[su_color_func]   = si_simplex_turbulence;
    indices[su_surface_func] = si_torus_func;
    uniform_t::subroutines(GL_FRAGMENT_SHADER, 2, indices);
    texture_renderer.model_matrix = torus_model_matrix;
    texture_renderer.create_textures();
    texture_renderer.bind_textures();
    texture_renderer.store_textures("diffuse.png", "normal.png");
    uniform_t uni_P = texture_renderer.program["P"];
    uniform_t uni_Q = texture_renderer.program["Q"];
    //===================================================================================================================================================================================================================
    // set up six knots
    //===================================================================================================================================================================================================================
    const float size = 10.0f;

    torus_knot_t knots[] = 
    {
        torus_knot_t(2.0f, 3.0f, GL_TEXTURE2,  GL_TEXTURE3,  si_simplex_turbulence,  glm::translate(glm::vec3( size,  0.0f, 0.0f))),
        torus_knot_t(3.0f, 2.0f, GL_TEXTURE4,  GL_TEXTURE5,  si_cellular_turbulence, glm::translate(glm::vec3(-size,  0.0f, 0.0f))),
        torus_knot_t(2.0f, 5.0f, GL_TEXTURE6,  GL_TEXTURE7,  si_marble_light,        glm::translate(glm::vec3( 0.0f,  size, 0.0f))),
        torus_knot_t(5.0f, 2.0f, GL_TEXTURE8,  GL_TEXTURE9,  si_marble_complex,      glm::translate(glm::vec3( 0.0f, -size, 0.0f))),
        torus_knot_t(3.0f, 5.0f, GL_TEXTURE10, GL_TEXTURE11, si_marble_light,        glm::translate(glm::vec3( 0.0f,  0.0f, size))),
        torus_knot_t(5.0f, 3.0f, GL_TEXTURE12, GL_TEXTURE13, si_marble_complex,      glm::translate(glm::vec3( 0.0f,  0.0f,-size)))
    };

    const unsigned int KNOTS_COUNT = sizeof(knots) / sizeof(torus_knot_t);
    indices[su_surface_func] = si_torusPQ_func;

    for(unsigned int i = 0; i < KNOTS_COUNT; ++i)
    {
        texture_renderer.reset_textures(knots[i].diffuse_texture_unit, knots[i].normal_texture_unit, 2048, 256);
        texture_renderer.model_matrix = knots[i].model_matrix;
        uni_P = knots[i].P;
        uni_Q = knots[i].Q;
        
        indices[su_color_func] = knots[i].diffuse_subroutine_index;
        uniform_t::subroutines(GL_FRAGMENT_SHADER, 2, indices);
        texture_renderer.create_textures();
        texture_renderer.bind_textures();
        char diffuse_fname[64];
        char normal_fname[64];
        sprintf(diffuse_fname, "diffuse_%u.png", i);
        sprintf(normal_fname,  "normal_%u.png", i);

        texture_renderer.store_textures(diffuse_fname, normal_fname);
    }

    //===================================================================================================================================================================================================================
    // restore default GL setting and default framebuffer
    //===================================================================================================================================================================================================================
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window.res_x, window.res_y);
    glEnable(GL_DEPTH_TEST);

	//===================================================================================================================================================================================================================
	// z-buffer fill shader programs : for pnt2-type and pft2-type vertices
	//===================================================================================================================================================================================================================
    glsl_program_t ambient_zfill_pnt2(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ambient_zfill_pnt2.vs"),
                                      glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ambient_zfill_pnt2.fs"));
    ambient_zfill_pnt2.dump_info();
	uniform_t uni_azfill_pnt2_pvmatrix = ambient_zfill_pnt2["projection_view_matrix"];
    ambient_zfill_pnt2.enable();
    ambient_zfill_pnt2["diffuse_texture"] = 14;

    glsl_program_t ambient_zfill_pf(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ambient_zfill_pf.vs"),
                                    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ambient_zfill_pf.fs"));
    ambient_zfill_pf.dump_info();
    ambient_zfill_pf.enable();
	uniform_t uni_azfill_pf_pvmatrix        = ambient_zfill_pf["projection_view_matrix"];
	uniform_t uni_azfill_pf_model_matrix    = ambient_zfill_pf["model_matrix"];
	uniform_t uni_azfill_pf_camera_ws       = ambient_zfill_pf["camera_ws"];
	uniform_t uni_azfill_pf_light_ws        = ambient_zfill_pf["light_ws"];
    uniform_t uni_azfill_pf_diffuse_texture = ambient_zfill_pf["diffuse_texture"];
    uniform_t uni_azfill_pf_normal_texture  = ambient_zfill_pf["normal_texture"];

	//===================================================================================================================================================================================================================
	// shadow volume generating shader program
	//===================================================================================================================================================================================================================
    glsl_program_t shadow_volume(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/shadow_volume.vs"),
                                 glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/shadow_volume.gs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/shadow_volume.fs"));
    shadow_volume.dump_info();
    shadow_volume.enable();
	uniform_t uni_sv_pvmatrix     = shadow_volume["projection_view_matrix"];
	uniform_t uni_sv_model_matrix = shadow_volume["model_matrix"];
	uniform_t uni_sv_light_ws     = shadow_volume["light_ws"];

	//===================================================================================================================================================================================================================
	// phong lighting : diffuse + specular for pnt2-type and pf-type vertices
	//===================================================================================================================================================================================================================
    glsl_program_t phong_lighting_pnt2(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_lighting_pnt2.vs"),
                                       glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_lighting_pnt2.fs"));
    phong_lighting_pnt2.dump_info();
    phong_lighting_pnt2.enable();
	uniform_t uni_pl_pnt2_pvmatrix  = phong_lighting_pnt2["projection_view_matrix"];
	uniform_t uni_pl_pnt2_camera_ws = phong_lighting_pnt2["camera_ws"];
	uniform_t uni_pl_pnt2_light_ws  = phong_lighting_pnt2["light_ws"];
    phong_lighting_pnt2["diffuse_texture"] = 14;

    glsl_program_t phong_lighting_pf(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_lighting_pf.vs"),
                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_lighting_pf.fs"));
    phong_lighting_pf.dump_info();
    phong_lighting_pf.enable();
	uniform_t uni_pl_pf_pvmatrix        = phong_lighting_pf["projection_view_matrix"];
	uniform_t uni_pl_pf_model_matrix    = phong_lighting_pf["model_matrix"];
	uniform_t uni_pl_pf_camera_ws       = phong_lighting_pf["camera_ws"];
	uniform_t uni_pl_pf_light_ws        = phong_lighting_pf["light_ws"];
    uniform_t uni_pl_pf_diffuse_texture = phong_lighting_pf["diffuse_texture"];
    uniform_t uni_pl_pf_normal_texture  = phong_lighting_pf["normal_texture"];

	//===================================================================================================================================================================================================================
	// generate torus with rectangle topology as we need texture coordinates
	//===================================================================================================================================================================================================================
    surface_t torus;
    torus.generate_vao<vertex_pft2_t>(torus_func, 128, 32);

    surface_t torus_knot[KNOTS_COUNT];
    for(unsigned int i = 0; i < KNOTS_COUNT; ++i)
    {
        P = knots[i].P;
        Q = knots[i].Q;
        torus_knot[i].generate_vao<vertex_pft2_t>(torusPQ_func, 200, 40);
    }

    const float cube_size = 40.0;
    polyhedron granite_room;
    granite_room.regular_pnt2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces, cube_size, true);

    glActiveTexture(GL_TEXTURE14);
    GLuint cube_texture_id = image::png::texture2d("../../../resources/tex2d/marble.png");

    //===================================================================================================================================================================================================================
    // global OpenGL state
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    const float light_radius = 18.5f;
    glm::vec3 light_ws = glm::vec3(light_radius, 0.0f, 0.0f);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(torus.vao.ibo.pri);

    //===================================================================================================================================================================================================================
    // main loop
    //===================================================================================================================================================================================================================

    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        window.new_frame();

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::vec3 camera_ws = window.camera.position();
        if (!window.static_light) 
        {
            double dt = window.frame_ts - window.light_ts;
            double cs = glm::cos(0.25 * dt); 
            double sn = glm::sin(0.25 * dt);
            light_ws = glm::vec3(light_ws.x * cs - light_ws.y * sn, light_ws.x * sn + light_ws.y * cs, 0.0f);
            window.light_ts = window.frame_ts;
        }
        
        //===============================================================================================================================================================================================================
        // render scene with ambient lights only and fill z-buffer with depth values
        //===============================================================================================================================================================================================================
        glDisable(GL_BLEND);                                                            // disable blending
        glDisable(GL_STENCIL_TEST);                                                     // disable stencil test
        
        ambient_zfill_pnt2.enable();
        uni_azfill_pnt2_pvmatrix = projection_view_matrix;
        granite_room.render();

        ambient_zfill_pf.enable();
        uni_azfill_pf_pvmatrix  = projection_view_matrix;
	    uni_azfill_pf_camera_ws = camera_ws;
	    uni_azfill_pf_light_ws  = light_ws;

        uni_azfill_pf_model_matrix = torus_model_matrix;
        uni_azfill_pf_diffuse_texture = 0;
        uni_azfill_pf_normal_texture  = 1;
        torus.render();

        for(unsigned int i = 0; i < KNOTS_COUNT; ++i)
        {
            uni_azfill_pf_model_matrix = knots[i].model_matrix;
            uni_azfill_pf_diffuse_texture = (GLint) knots[i].diffuse_texture_unit - GL_TEXTURE0;
            uni_azfill_pf_normal_texture  = (GLint) knots[i].normal_texture_unit - GL_TEXTURE0;
            torus_knot[i].render();
        }
        //===============================================================================================================================================================================================================
        // pass the geometry of shadow casters through shadow volume generating geometry shader program
        // stencil test must be enabled but must always pass (only the depth test matters) otherwise stencil buffer will not be modified
        //===============================================================================================================================================================================================================
        glDepthMask(GL_FALSE);                                                          // enable depth writes
        glDisable(GL_CULL_FACE);                                                        // disable cull-face as we need both front and back faces to be rasterized
        glDrawBuffer(GL_NONE);                                                          // disable color writes, maybe not be needed as fragment shader does not output anything anyway
        glEnable(GL_STENCIL_TEST);                                                      // enable stencil test and ...
        glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);                                        // ... set it to always pass
        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_KEEP);                  // increment stencil value when front shadow face is rasterized
        glStencilOpSeparate(GL_BACK,  GL_KEEP, GL_DECR_WRAP, GL_KEEP);                  // decrement stencil value when back face is rasterized
		        
        shadow_volume.enable();
        uni_sv_pvmatrix = projection_view_matrix;
        uni_sv_light_ws = light_ws;     

        uni_sv_model_matrix = torus_model_matrix;
        torus.render();

        for(unsigned int i = 0; i < KNOTS_COUNT; ++i)
        {
            uni_sv_model_matrix = knots[i].model_matrix;
            torus_knot[i].render();
        }

        //===============================================================================================================================================================================================================
        // render light diffuse and specular components into lit areas where stencil value is zero
        //===============================================================================================================================================================================================================
        glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);                                                           // enable depth writes

        glDrawBuffer(GL_BACK);                                                          // enable color buffer writes
     	glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

        glStencilFunc(GL_EQUAL, 0, 0xFFFFFFFF);                                               // stencil test must be enabled
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);                                         // prevent update to the stencil buffer

        phong_lighting_pnt2.enable();
        uni_pl_pnt2_pvmatrix  = projection_view_matrix;
        uni_pl_pnt2_camera_ws = camera_ws;
        uni_pl_pnt2_light_ws  = light_ws;
        granite_room.render();

        phong_lighting_pf.enable();
        uni_pl_pf_pvmatrix  = projection_view_matrix;
        uni_pl_pf_camera_ws = camera_ws;
        uni_pl_pf_light_ws  = light_ws;

        uni_pl_pf_model_matrix = torus_model_matrix;
        uni_pl_pf_diffuse_texture = 0;
        uni_pl_pf_normal_texture  = 1;
        torus.render();
        for(unsigned int i = 0; i < KNOTS_COUNT; ++i)
        {
            uni_azfill_pf_model_matrix = knots[i].model_matrix;
            uni_azfill_pf_diffuse_texture = (GLint) knots[i].diffuse_texture_unit - GL_TEXTURE0;
            uni_azfill_pf_normal_texture  = (GLint) knots[i].normal_texture_unit - GL_TEXTURE0;
            torus_knot[i].render();
        }
       
        //===============================================================================================================================================================================================================
        // done. swap da buffers
        //===============================================================================================================================================================================================================
        window.end_frame();
    }
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}