//========================================================================================================================================================================================================================
// OCULUS DEMO 008 : Stencil Shadows + Multiple Viewports via Geometry Shader 
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "ovr/ovr_hmd.hpp"

#include "log.hpp"
#include "gl_info.hpp"
#include "shader.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "image.hpp"
#include "vertex.hpp"
#include "plato.hpp"
#include "polyhedron.hpp"
#include "vao.hpp"
#include "surface.hpp"
#include "torus.hpp"
#include "adjacency.hpp"

glm::mat4 rotation_matrix(const glm::vec3& axis, float angle)
{
    float sn = sin(angle);
    float cs = cos(angle);
    glm::vec3 axis_cs = (1.0f - cs) * axis;
    glm::vec3 axis_sn = sn * axis;
    
    return glm::mat4(glm::vec4(axis_cs.x * axis + glm::vec3(        cs,  axis_sn.z, -axis_sn.y), 0.0),
                     glm::vec4(axis_cs.y * axis + glm::vec3(-axis_sn.z,         cs,  axis_sn.x), 0.0),
                     glm::vec4(axis_cs.z * axis + glm::vec3( axis_sn.y, -axis_sn.x,         cs), 0.0),
                     glm::vec4(                                                 glm::vec3(0.0f), 1.0));
}

//=======================================================================================================================================================================================================================
// Euclidean space camera bound to vr device
//=======================================================================================================================================================================================================================
struct hmd_camera_t
{
    double linear_speed;
    double angular_speed;

    //===================================================================================================================================================================================================================
    // VR device relative (to sensor) position and orientation
    //===================================================================================================================================================================================================================
    glm::mat4 orientation;
    glm::mat4 translation;
    glm::mat4 hmd_view_matrix;
    glm::mat4 eye_view_matrix[ovrEye_Count];

    //===================================================================================================================================================================================================================
    // Mouse / keyboard control accumulated position and orientation
    //===================================================================================================================================================================================================================
    glm::mat4 view_matrix;

    //===================================================================================================================================================================================================================
    // Local HMD-eye view transfomation matrices, assumed to be set once
    //===================================================================================================================================================================================================================
    glm::mat4 eye_local_matrix[ovrEye_Count];

    hmd_camera_t(const double linear_speed = 2.0, const double angular_speed = 0.125, const glm::mat4& view_matrix = glm::mat4(1.0f))
        : linear_speed(linear_speed), angular_speed(angular_speed), view_matrix(view_matrix)
    {
        orientation = glm::mat4(1.0f);
        translation = glm::mat4(0.0f);
    }

    void move_forward(double dt)
        { view_matrix[3] += float(linear_speed * dt) * orientation[2]; }

    void move_backward(double dt)
        { view_matrix[3] -= float(linear_speed * dt) * orientation[2]; }


    void straight_right(double dt)
        { view_matrix[3] -= float(linear_speed * dt) * orientation[0]; }

    void straight_left(double dt)
        { view_matrix[3] += float(linear_speed * dt) * orientation[0]; }

    void rotateXY(const glm::dvec2& direction, double dt)
    {
        glm::vec3 axis = glm::normalize(orientation[0] * float(direction.y) + orientation[1] * float(direction.x));
        float theta = angular_speed * dt;
        glm::mat4 rotation = rotation_matrix(axis, theta);
        view_matrix = rotation * view_matrix;
    }

    glm::mat4 camera_matrix(ovrEyeType eye)
        { return glm::inverse(eye_view_matrix[eye]); }

    void set_hmd_view_matrix(const glm::quat& rotation, const glm::vec3& position)
    {
        orientation = glm::mat4_cast(rotation);
        translation = glm::translate(position);
        hmd_view_matrix = glm::inverse(orientation) * glm::inverse(translation);
        eye_view_matrix[ovrEye_Left ] = eye_local_matrix[ovrEye_Left ] * hmd_view_matrix * view_matrix;
        eye_view_matrix[ovrEye_Right] = eye_local_matrix[ovrEye_Right] * hmd_view_matrix * view_matrix;
    }

    void set_eye_local_matrix(ovrEyeType eye, const glm::quat& rotation, const glm::vec3& position)
    {
        glm::mat4 local_orientation = glm::mat4_cast(rotation);
        glm::mat4 local_translation = glm::translate(position);
        eye_local_matrix[eye] = glm::inverse(local_orientation) * glm::inverse(local_translation);
    }    

    glm::vec3 position(ovrEyeType eye) const
    {
        const glm::mat4& vmatrix = eye_view_matrix[eye];
        return -glm::inverse(glm::mat3(vmatrix)) * glm::vec3(vmatrix[3]);
    }

    glm::vec3 head_position() const
    {
        glm::mat4 vmatrix = hmd_view_matrix * view_matrix;
        return -glm::inverse(glm::mat3(vmatrix)) * glm::vec3(vmatrix[3]);
    }
};

//=======================================================================================================================================================================================================================
// Euclidean space camera bound to vr device
//=======================================================================================================================================================================================================================
struct demo_window_t : public glfw_window_t
{
    hmd_camera_t camera;
    bool dynamic_light = false;
    double light_ts;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    { 
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
            dynamic_light = !dynamic_light;
            light_ts = frame_ts;
        }

        if (action != GLFW_RELEASE) return;
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, 1);
        //if (key == GLFW_KEY_R) ovr_RecenterTrackingOrigin(session);
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

vertex_pn_t torus_func(const glm::vec2& uv)
{
    vertex_pn_t vertex;

    float cos_2piu = glm::cos(constants::two_pi * uv.x);
    float sin_2piu = glm::sin(constants::two_pi * uv.x);
    float cos_2piv = glm::cos(constants::two_pi * uv.y);
    float sin_2piv = glm::sin(constants::two_pi * uv.y);

    float R = 1.17f;
    float r = 0.37f;

    vertex.position = glm::vec3((R + r * cos_2piu) * cos_2piv, 2.1f + (R + r * cos_2piu) * sin_2piv, 3.0f + r * sin_2piu);
    vertex.normal = glm::vec3(cos_2piu * cos_2piv, cos_2piu * sin_2piv, sin_2piu);

    return vertex;
}

//=======================================================================================================================================================================================================================
//  Program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char** argv)
{
    //===================================================================================================================================================================================================================
    // initialize vr device
    //===================================================================================================================================================================================================================
    ovr_hmd_t ovr_hmd;

    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Rift Attractor", 4, 4, 3, ovr_hmd.mirror_size.x, ovr_hmd.mirror_size.y, false);

    //===================================================================================================================================================================================================================
    // Create texture swapchain and mirror texture for displaying in the app window
    //===================================================================================================================================================================================================================
    ovr_hmd.create_swap_chain();
    ovr_hmd.create_mirror_texture();
    window.camera.set_eye_local_matrix(ovrEye_Left,  ovr_hmd.eye_rotation_rel[ovrEye_Left ], ovr_hmd.eye_position_rel[ovrEye_Left ]);
    window.camera.set_eye_local_matrix(ovrEye_Right, ovr_hmd.eye_rotation_rel[ovrEye_Right], ovr_hmd.eye_position_rel[ovrEye_Right]);

    //===================================================================================================================================================================================================================
    // Set up the framebuffer objects : one for oculus usage and one for screen blitting
    //===================================================================================================================================================================================================================
    GLuint fbo_id;
    GLuint rbo_id;
    GLuint mirror_fbo_id;

    glGenFramebuffers(1, &fbo_id);
    glGenRenderbuffers(1, &rbo_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, ovr_hmd.target_size.x, ovr_hmd.target_size.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &mirror_fbo_id);

    //===================================================================================================================================================================================================================
    // z-buffer fill shader programs : for positoin-frame type vertices
    //===================================================================================================================================================================================================================
    glsl_program_t ambient_zfill(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ambient_zfill.vs"),
                                 glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/ambient_zfill.gs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ambient_zfill.fs"));
    ambient_zfill.dump_info();
    ambient_zfill.enable();
    uniform_t uni_zf_pvmatrix = ambient_zfill["projection_view_matrix"];
    uniform_t uni_zf_light_ws = ambient_zfill["light_ws"];
    uniform_t uni_zf_shift    = ambient_zfill["shift"];
    ambient_zfill["tb_tex"] = 0;


    //===================================================================================================================================================================================================================
    // shadow volume generating shader program
    //===================================================================================================================================================================================================================
    glsl_program_t shadow_volume(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/shadow_volume.vs"),
                                 glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/shadow_volume.gs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/shadow_volume.fs"));
    shadow_volume.dump_info();
    uniform_t uni_sv_pvmatrix = shadow_volume["projection_view_matrix"];
    uniform_t uni_sv_light_ws = shadow_volume["light_ws"];
    uniform_t uni_sv_shift    = shadow_volume["shift"];

    //===================================================================================================================================================================================================================
    // phong lighting for position + tangent frame vertices
    // procedural (possibly tri-linear blended) texturing assumed
    //===================================================================================================================================================================================================================
    glsl_program_t phong_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_light.vs"),
                               glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/phong_light.gs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_light.fs"));
    phong_light.dump_info();
    phong_light.enable();
    uniform_t uni_pl_pvmatrix  = phong_light["projection_view_matrix"];
    uniform_t uni_pl_camera_ws = phong_light["camera_ws"];
    uniform_t uni_pl_light_ws  = phong_light["light_ws"];
    uniform_t uni_pl_shift     = phong_light["shift"];
    phong_light["tb_tex"] = 0;

    //===================================================================================================================================================================================================================
    // cube room and its inhabitants ...
    //===================================================================================================================================================================================================================
    const float room_size = 17.5f;
    polyhedron room;
    room.regular_pn_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces, room_size, true);

    //===================================================================================================================================================================================================================
    // ... torus with adjacency index buffer
    //===================================================================================================================================================================================================================
    torus_t torus;
    adjacency_vao_t torus_adjacency;
    torus.generate_vao<vertex_pn_t>(torus_func, 37, 67, &torus_adjacency);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(torus.vao.ibo.pri);

    GLint pv;
    glGetIntegerv(GL_LAYER_PROVOKING_VERTEX, &pv);
    debug_msg("GL_LAYER_PROVOKING_VERTEX = %u", pv);
    glGetIntegerv(GL_PROVOKING_VERTEX, &pv);
    debug_msg("GL_PROVOKING_VERTEX = %u", pv);
    glGetIntegerv(GL_VIEWPORT_INDEX_PROVOKING_VERTEX, &pv);
    debug_msg("GL_VIEWPORT_INDEX_PROVOKING_VERTEX = %u", pv);

    debug_msg("GL_FIRST_VERTEX_CONVENTION = %u", GL_FIRST_VERTEX_CONVENTION);
    debug_msg("GL_LAST_VERTEX_CONVENTION = %u", GL_LAST_VERTEX_CONVENTION);
    debug_msg("GL_UNDEFINED_VERTEX = %u", GL_UNDEFINED_VERTEX);

    //===================================================================================================================================================================================================================
    // ... and 5 plato solids with their adjacency index buffer
    //===================================================================================================================================================================================================================
    polyhedron tetrahedron, cube, octahedron, dodecahedron, icosahedron;

    tetrahedron.regular_pn_vao (plato::tetrahedron::V,  plato::tetrahedron::F,  plato::tetrahedron::vertices,  plato::tetrahedron::normals,  plato::tetrahedron::faces);
    cube.regular_pn_vao        (plato::cube::V,         plato::cube::F,         plato::cube::vertices,         plato::cube::normals,         plato::cube::faces);
    octahedron.regular_pn_vao  (plato::octahedron::V,   plato::octahedron::F,   plato::octahedron::vertices,   plato::octahedron::normals,   plato::octahedron::faces);
    dodecahedron.regular_pn_vao(plato::dodecahedron::V, plato::dodecahedron::F, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);
    icosahedron.regular_pn_vao (plato::icosahedron::V,  plato::icosahedron::F,  plato::icosahedron::vertices,  plato::icosahedron::normals,  plato::icosahedron::faces);
    
    vao_t tetrahedron_adjacency  = build_adjacency_vao<GLuint>(plato::tetrahedron::vertices,  plato::tetrahedron::triangles,  plato::tetrahedron::V,  plato::tetrahedron::T);
    vao_t cube_adjacency         = build_adjacency_vao<GLuint>(plato::cube::vertices,         plato::cube::triangles,         plato::cube::V,         plato::cube::T);
    vao_t octahedron_adjacency   = build_adjacency_vao<GLuint>(plato::octahedron::vertices,   plato::octahedron::triangles,   plato::octahedron::V,   plato::octahedron::T);
    vao_t dodecahedron_adjacency = build_adjacency_vao<GLuint>(plato::dodecahedron::vertices, plato::dodecahedron::triangles, plato::dodecahedron::V, plato::dodecahedron::T);
    vao_t icosahedron_adjacency  = build_adjacency_vao<GLuint>(plato::icosahedron::vertices,  plato::icosahedron::triangles,  plato::icosahedron::V,  plato::icosahedron::T);

    //===================================================================================================================================================================================================================
    // load different material textures for trilinear blending
    //===================================================================================================================================================================================================================
    GLuint clay_tex_id        = image::png::texture2d("../../../resources/tex2d/clay.png",        0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    GLuint crystalline_tex_id = image::png::texture2d("../../../resources/tex2d/crystalline.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    GLuint marble_tex_id      = image::png::texture2d("../../../resources/tex2d/marble.png",      0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    GLuint ice_tex_id         = image::png::texture2d("../../../resources/tex2d/ice2.png",        0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    GLuint pink_stone_tex_id  = image::png::texture2d("../../../resources/tex2d/pink_stone.png",  0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    GLuint plumbum_tex_id     = image::png::texture2d("../../../resources/tex2d/plumbum.png",     0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    GLuint emerald_tex_id     = image::png::texture2d("../../../resources/tex2d/emerald.png",     0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    //===================================================================================================================================================================================================================
    // global OpenGL state
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glActiveTexture(GL_TEXTURE0);

    const float shift = 5.0;
    const float light_radius = 0.475 * room_size;

    //===================================================================================================================================================================================================================
    // main rendering loop
    //===================================================================================================================================================================================================================
    while (!window.should_close())
    {
        window.new_frame();

        //===============================================================================================================================================================================================================
        // update tracking data : head and eyes positions and orientation and set up eyes view matrices
        //===============================================================================================================================================================================================================
        ovr_hmd.update_tracking(window.frame);
        window.camera.set_hmd_view_matrix(ovr_hmd.head_rotation, ovr_hmd.head_position);

        float time = window.frame_ts;
        float angle = 0.125 * time;
        glm::vec3 light_ws = light_radius * glm::vec3(glm::cos(angle), glm::sin(angle), 0.0f);

        if (window.dynamic_light) 
        {
            double time = window.frame_ts;
            double dt = time - window.light_ts;
            double cs = glm::cos(0.25f * dt); 
            double sn = glm::sin(0.25f * dt);
            light_ws = glm::vec3(light_ws.x * cs - light_ws.y * sn, light_ws.x * sn + light_ws.y * cs, 0.0f);
            window.light_ts = time;
        }

        //===============================================================================================================================================================================================================
        // bind swapchain texture
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ovr_hmd.swapchain_texture_id(), 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        ovr_hmd.set_indexed_viewport(ovrEye_Left,  0);
        ovr_hmd.set_indexed_viewport(ovrEye_Right, 1);

        glActiveTexture(GL_TEXTURE0);
        //===============================================================================================================================================================================================================
        // render the scene for both eyes simultaneously
        //===============================================================================================================================================================================================================
        glm::mat4 projection_view_matrix[ovrEye_Count] = 
        {
            ovr_hmd.projection_matrix[ovrEye_Left ] * window.camera.eye_view_matrix[ovrEye_Left ],
            ovr_hmd.projection_matrix[ovrEye_Right] * window.camera.eye_view_matrix[ovrEye_Right]
        };

        glm::vec3 camera_ws[ovrEye_Count] =
        {
            window.camera.position(ovrEye_Left ),
            window.camera.position(ovrEye_Right)
        };

        //===============================================================================================================================================================================================================
        // render scene with ambient lights only and fill z-buffer with depth values
        //===============================================================================================================================================================================================================        
        ambient_zfill.enable();
        uni_zf_pvmatrix = projection_view_matrix;
        uni_zf_light_ws = light_ws;

        uni_zf_shift = glm::vec3(0.0, 0.0,  shift);
        glBindTexture(GL_TEXTURE_2D, clay_tex_id);
        tetrahedron.render();

        uni_zf_shift = glm::vec3(0.0, 0.0, -shift);
        glBindTexture(GL_TEXTURE_2D, crystalline_tex_id);
        cube.render();

        uni_zf_shift = glm::vec3(0.0,  shift, 0.0);
        glBindTexture(GL_TEXTURE_2D, marble_tex_id);
        octahedron.render();

        uni_zf_shift = glm::vec3(0.0, -shift, 0.0);
        glBindTexture(GL_TEXTURE_2D, ice_tex_id);
        dodecahedron.render();

        uni_zf_shift = glm::vec3( shift, 0.0, 0.0);
        glBindTexture(GL_TEXTURE_2D, pink_stone_tex_id);
        icosahedron.render(); 

        uni_zf_shift = glm::vec3(-shift, 0.0, 0.0);
        glBindTexture(GL_TEXTURE_2D, plumbum_tex_id);
        torus.render();

        uni_zf_shift = glm::vec3(0.0);
        glBindTexture(GL_TEXTURE_2D, emerald_tex_id);
        room.render();

        //===============================================================================================================================================================================================================
        // pass the geometry of shadow casters through shadow volume generating program
        // stencil test must be enabled but must always pass (only the depth test matters) otherwise stencil buffer will not be modified
        //===============================================================================================================================================================================================================
        glDrawBuffer(GL_NONE);                                                          // disable color writes, maybe not be needed as fragment shader does not output anything anyway

        glDepthMask(GL_FALSE);                                                          // disable depth writes
        glDisable(GL_CULL_FACE);                                                        // disable cull-face as we need both front and back faces to be rasterized
        glEnable(GL_STENCIL_TEST);                                                      // enable stencil test and ...
        glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);                                        // ... set it to always pass
        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_KEEP);                  // invert stencil value when either front or back shadow face is rasterized ...
        glStencilOpSeparate(GL_BACK,  GL_KEEP, GL_DECR_WRAP, GL_KEEP);                  // invert stencil value when either front or back shadow face is rasterized ...

        shadow_volume.enable();
        uni_sv_pvmatrix = projection_view_matrix;
        uni_sv_light_ws = light_ws;

        uni_sv_shift = glm::vec3(0.0, 0.0,  shift);
        tetrahedron_adjacency.render();
        uni_sv_shift = glm::vec3(0.0, 0.0, -shift);
        cube_adjacency.render();

        uni_sv_shift = glm::vec3(0.0,  shift, 0.0);
        octahedron_adjacency.render();

        uni_sv_shift = glm::vec3(0.0, -shift, 0.0);
        dodecahedron_adjacency.render();

        uni_sv_shift = glm::vec3( shift, 0.0, 0.0);
        icosahedron_adjacency.render(); 

        uni_sv_shift = glm::vec3(-shift, 0.0, 0.0);
//        torus_adjacency.render();

        //===============================================================================================================================================================================================================
        // render light diffuse and specular components into lit areas where stencil value is zero
        //===============================================================================================================================================================================================================
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glEnable(GL_CULL_FACE);                                                         // cullface can be enabled back at this point
        glEnable(GL_BLEND);                                                             // ambient component is already in the color buffer
        glBlendEquation(GL_FUNC_ADD);                                                   // and we want to just add the diffuse and specular components to 
        glBlendFunc(GL_ONE, GL_ONE);                                                    // lit areas

        glStencilFunc(GL_EQUAL, 0, 0xFFFFFFFF);                                         // stencil test must be enabled and the scene be rendered to area where stencil value is zero
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);                                         // prevent update to the stencil buffer

        phong_light.enable();
        uni_pl_pvmatrix  = projection_view_matrix;
        uni_pl_camera_ws = camera_ws;
        uni_pl_light_ws  = light_ws;

        uni_pl_shift = glm::vec3(0.0, 0.0,  shift);
        glBindTexture(GL_TEXTURE_2D, clay_tex_id);
        tetrahedron.render();

        uni_pl_shift = glm::vec3(0.0, 0.0, -shift);
        glBindTexture(GL_TEXTURE_2D, crystalline_tex_id);
        cube.render();

        uni_pl_shift = glm::vec3(0.0,  shift, 0.0);
        glBindTexture(GL_TEXTURE_2D, marble_tex_id);
        octahedron.render();

        uni_pl_shift = glm::vec3(0.0, -shift, 0.0);
        glBindTexture(GL_TEXTURE_2D, ice_tex_id);
        dodecahedron.render();

        uni_pl_shift = glm::vec3( shift, 0.0, 0.0);
        glBindTexture(GL_TEXTURE_2D, pink_stone_tex_id);
        icosahedron.render(); 

        uni_pl_shift = glm::vec3(-shift, 0.0, 0.0);
        glBindTexture(GL_TEXTURE_2D, plumbum_tex_id);
        torus.render();

        uni_pl_shift = glm::vec3(0.0);
        glBindTexture(GL_TEXTURE_2D, emerald_tex_id);
        room.render();

        glDepthMask(GL_TRUE);                                                           // enable depth writes for next render cycle,
        glDisable(GL_BLEND);                                                            // disable blending, and ...
        glDisable(GL_STENCIL_TEST);                                                     //  ... disable stencil test

        //===============================================================================================================================================================================================================
        // submit the texture to vr device
        //===============================================================================================================================================================================================================
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        ovr_hmd.submit_frame(window.frame);

        //===============================================================================================================================================================================================================
        // set default framebuffer as destination, mirror fbo as source and blit the mirror texture to screen
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mirror_fbo_id);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ovr_hmd.mirror_texture_id(), 0);
        glBlitFramebuffer(0, 0, ovr_hmd.mirror_size.x, ovr_hmd.mirror_size.y, 0, ovr_hmd.mirror_size.y, ovr_hmd.mirror_size.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // destroy GLFW window and terminate the library
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}