//========================================================================================================================================================================================================================
// DEMO 009 : Stencil shadows via GL_TRIANGLES_ADJACENCY primitive
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp> 
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "vertex.hpp"
#include "plato.hpp"
#include "vao.hpp"
#include "image.hpp"
#include "polyhedron.hpp"
#include "torus.hpp"

#include "edge.hpp"


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
            light_ts = frame_ts;
        }
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

struct room_t
{
    GLuint vao_id;    
    vbo_t vbo;

    room_t(float size)
    {
        vertex_pnt2_t vertices[36];

        glm::vec2 unit_square[4] = 
        {
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f)
        };

        int index = 0;
        int vindex = 0;

        for(int i = 0; i < 6; ++i)
        {
            int A = plato::cube::faces[vindex++];
            int B = plato::cube::faces[vindex++];
            int C = plato::cube::faces[vindex++];
            int D = plato::cube::faces[vindex++];
            glm::vec3 normal = -plato::cube::normals[i];
            vertices[index++] = vertex_pnt2_t(size * plato::cube::vertices[A], normal, unit_square[0]);
            vertices[index++] = vertex_pnt2_t(size * plato::cube::vertices[C], normal, unit_square[2]);
            vertices[index++] = vertex_pnt2_t(size * plato::cube::vertices[B], normal, unit_square[1]);
            vertices[index++] = vertex_pnt2_t(size * plato::cube::vertices[A], normal, unit_square[0]);
            vertices[index++] = vertex_pnt2_t(size * plato::cube::vertices[D], normal, unit_square[3]);
            vertices[index++] = vertex_pnt2_t(size * plato::cube::vertices[C], normal, unit_square[2]);
        }

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        vbo.init(vertices, 36);
    }

    void render()
    {
        glBindVertexArray(vao_id);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    ~room_t()
        { glDeleteVertexArrays(1, &vao_id); };
};

vertex_pf_t torus_func(const glm::vec2& uv)
{
    vertex_pf_t vertex;

    float cos_2piu = glm::cos(constants::two_pi * uv.x);
    float sin_2piu = glm::sin(constants::two_pi * uv.x);
    float cos_2piv = glm::cos(constants::two_pi * uv.y);
    float sin_2piv = glm::sin(constants::two_pi * uv.y);

    float R = 2.7f;
    float r = 0.97f;

    vertex.position = glm::vec3((R + r * cos_2piu) * cos_2piv, 2.1f + (R + r * cos_2piu) * sin_2piv, 3.0f + r * sin_2piu);
    vertex.tangent_x = glm::vec3(-sin_2piu * cos_2piv, -sin_2piu * sin_2piv, cos_2piu);
    vertex.tangent_y = glm::vec3(-sin_2piv, cos_2piv, 0.0f);
    vertex.normal = glm::vec3(cos_2piu * cos_2piv, cos_2piu * sin_2piv, sin_2piu);

    return vertex;
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

    demo_window_t window("Stencil shadows via GL_TRIANGLES_ADJACENCY primitive", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // z-buffer fill shader programs : for pnt2-type and pf-type vertices
    //===================================================================================================================================================================================================================
    glsl_program_t ambient_zfill_pnt2(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ambient_zfill_pnt2.vs"),
                                      glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ambient_zfill_pnt2.fs"));
    ambient_zfill_pnt2.dump_info();
    uniform_t uni_azfill_pnt2_pvmatrix = ambient_zfill_pnt2["projection_view_matrix"];
    ambient_zfill_pnt2.enable();
    ambient_zfill_pnt2["diffuse_texture"] = 0;

    glsl_program_t ambient_zfill_pf(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ambient_zfill_pf.vs"),
                                    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ambient_zfill_pf.fs"));
    ambient_zfill_pf.dump_info();
    uniform_t uni_azfill_pf_pvmatrix = ambient_zfill_pf["projection_view_matrix"];

    //===================================================================================================================================================================================================================
    // shadow volume generating shader program
    //===================================================================================================================================================================================================================
    glsl_program_t shadow_volume(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/shadow_volume.vs"),
                                 glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/shadow_volume.gs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/shadow_volume.fs"));
    shadow_volume.dump_info();
    uniform_t uni_sv_pvmatrix = shadow_volume["projection_view_matrix"];
    uniform_t uni_sv_light_ws = shadow_volume["light_ws"];

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
    phong_lighting_pnt2["diffuse_texture"] = 0;

    glsl_program_t phong_lighting_pf(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_lighting_pf.vs"),
                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_lighting_pf.fs"));
    phong_lighting_pf.dump_info();
    phong_lighting_pf.enable();
    uniform_t uni_pl_pf_pvmatrix  = phong_lighting_pf["projection_view_matrix"];
    uniform_t uni_pl_pf_camera_ws = phong_lighting_pf["camera_ws"];
    uniform_t uni_pl_pf_light_ws  = phong_lighting_pf["light_ws"];

    //===================================================================================================================================================================================================================
    // generate torus with adjacency index buffer
    //===================================================================================================================================================================================================================
    torus_t torus;
    adjacency_vao_t torus_adjacency;
    torus.generate_vao<vertex_pf_t>(torus_func, 37, 67, &torus_adjacency);

    //===================================================================================================================================================================================================================
    // generate icosahedron with adjacency index buffer
    //===================================================================================================================================================================================================================

    GLuint V = plato::icosahedron::V;
    GLuint F = plato::icosahedron::F;

    GLuint vao_id, vbo_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, V * sizeof(glm::vec3), plato::icosahedron::vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    ibo_t adjacency_ibo = build_adjacency_ibo<GLuint>((glm::uvec3*) &plato::icosahedron::triangles[0], F);                          

    //===================================================================================================================================================================================================================
    // generate standard position + normal + uv icosahedron data 
    //===================================================================================================================================================================================================================
    polyhedron icosahedron;
    icosahedron.regular_pnt2_vao(12, 20, plato::icosahedron::vertices, plato::icosahedron::normals, plato::icosahedron::faces);

    const float cube_size = 23.33;
    room_t granite_room(cube_size);    

    GLuint cube_texture_id        = image::png::texture2d("../../../resources/tex2d/marble.png");
    GLuint icosahedron_texture_id = image::png::texture2d("../../../resources/plato_tex2d/icosahedron.png");
    glActiveTexture(GL_TEXTURE0);

    //===================================================================================================================================================================================================================
    // global OpenGL state
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    const float light_radius = 12.5f;
    glm::vec3 light_ws = glm::vec3(light_radius, 0.0f, 0.0f);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(-1);

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
            double time = window.frame_ts;
            double dt = time - window.light_ts;
            double cs = glm::cos(0.25f * dt); 
            double sn = glm::sin(0.25f * dt);
            light_ws = glm::vec3(light_ws.x * cs - light_ws.y * sn, light_ws.x * sn + light_ws.y * cs, 0.0f);
            window.light_ts = time;
        }
        
        //===============================================================================================================================================================================================================
        // render scene with ambient lights only and fill z-buffer with depth values
        //===============================================================================================================================================================================================================        
        ambient_zfill_pnt2.enable();
        uni_azfill_pnt2_pvmatrix = projection_view_matrix;
      
        glBindTexture(GL_TEXTURE_2D, icosahedron_texture_id);                                    
        icosahedron.render();
        glBindTexture(GL_TEXTURE_2D, cube_texture_id);                                    
        granite_room.render();

        ambient_zfill_pf.enable();
        uni_azfill_pf_pvmatrix = projection_view_matrix;
        torus.render();

        //===============================================================================================================================================================================================================
        // pass the geometry of shadow casters through shadow volume generating geometry shader program
        // stencil test must be enabled but must always pass (only the depth test matters) otherwise stencil buffer will not be modified
        //===============================================================================================================================================================================================================
        glDepthMask(GL_FALSE);                                                          // enable depth writes
        glDisable(GL_CULL_FACE);                                                        // disable cull-face as we need both front and back faces to be rasterized
        glDrawBuffer(GL_NONE);                                                          // disable color writes, maybe not be needed as fragment shader does not output anything anyway
        glEnable(GL_STENCIL_TEST);                                                      // enable stencil test and ...
        glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);                                        // ... set it to always pass
        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_KEEP);                  // invert stencil value when either front or back shadow face is rasterized ...
        glStencilOpSeparate(GL_BACK,  GL_KEEP, GL_DECR_WRAP, GL_KEEP);                  // invert stencil value when either front or back shadow face is rasterized ...
                
        shadow_volume.enable();
        uni_sv_pvmatrix = projection_view_matrix;
        uni_sv_light_ws = light_ws;     

        glBindVertexArray(vao_id);                                                      // render objects that generate shadow volume only
        adjacency_ibo.render(); 
        torus_adjacency.render();

        //===============================================================================================================================================================================================================
        // render light diffuse and specular components into lit areas where stencil value is zero
        //===============================================================================================================================================================================================================
        glEnable(GL_CULL_FACE);                                                         // cullface can be enabled back at this point

        glDrawBuffer(GL_BACK);                                                          // enable color buffer writes
        glEnable(GL_BLEND);                                                             // ambient component is already in the color buffer
        glBlendEquation(GL_FUNC_ADD);                                                   // and we want to just add the diffuse and specular components to 
        glBlendFunc(GL_ONE, GL_ONE);                                                    // lit areas

        glStencilFunc(GL_EQUAL, 0, 0xFFFFFFFF);                                         // stencil test must be enabled and the scene be rendered to area where stencil value is zero
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);                                         // prevent update to the stencil buffer

        phong_lighting_pnt2.enable();
        uni_pl_pnt2_pvmatrix  = projection_view_matrix;
        uni_pl_pnt2_camera_ws = camera_ws;
        uni_pl_pnt2_light_ws  = light_ws;

        glBindTexture(GL_TEXTURE_2D, icosahedron_texture_id);                                    
        icosahedron.render();

        glBindTexture(GL_TEXTURE_2D, cube_texture_id);                                    
        granite_room.render();

        phong_lighting_pf.enable();
        uni_pl_pf_pvmatrix  = projection_view_matrix;
        uni_pl_pf_camera_ws = camera_ws;
        uni_pl_pf_light_ws  = light_ws;
        torus.render();

        glDepthMask(GL_TRUE);                                                           // enable depth writes for next render cycle,
        glDisable(GL_BLEND);                                                            // disable blending, and ...
        glDisable(GL_STENCIL_TEST);                                                     //  ... disable stencil test

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