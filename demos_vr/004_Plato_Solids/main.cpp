//========================================================================================================================================================================================================================
// OCULUS DEMO 004 : Simple marble room with plato solids
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
#include "plato.hpp"
#include "polyhedron.hpp"
#include "surface.hpp"

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
};

//=======================================================================================================================================================================================================================
// Euclidean space camera bound to vr device
//=======================================================================================================================================================================================================================
struct demo_window_t : public glfw_window_t
{
    hmd_camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    { 
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

struct motion3d_t
{
    glm::vec4 shift;
    glm::vec4 rotor;
};

vertex_pft2_t torus_func(const glm::vec2& uv)
{
    vertex_pft2_t vertex;
    vertex.uv = uv;

    float cos_2piu = glm::cos(constants::two_pi * uv.y);
    float sin_2piu = glm::sin(constants::two_pi * uv.y);
    float cos_2piv = glm::cos(constants::two_pi * uv.x);
    float sin_2piv = glm::sin(constants::two_pi * uv.x);

    float R = 0.7f;
    float r = 0.3f;

    vertex.position = glm::vec3(
                        (R + r * cos_2piu) * cos_2piv,
                        (R + r * cos_2piu) * sin_2piv,
                             r * sin_2piu);

    vertex.tangent_x = glm::vec3(-sin_2piu * cos_2piv, -sin_2piu * sin_2piv, cos_2piu);
    vertex.tangent_y = glm::vec3(-sin_2piv, cos_2piv, 0.0f);

    vertex.normal = glm::vec3(cos_2piu * cos_2piv, cos_2piu * sin_2piv, sin_2piu);

    return vertex;
};

//=======================================================================================================================================================================================================================
// function that initializes initial model matrices and object rotation axes
//=======================================================================================================================================================================================================================
void fill_shift_rotor_data(motion3d_t* data, const glm::vec3& group_shift, float cell_size, int N)
{
    float middle = 0.5f * float(N) - 0.5f;
    int index = 0;
    for (int i = 0; i < N; ++i) for (int j = 0; j < N; ++j) for (int k = 0; k < N; ++k)
    {
        data[index].shift = glm::vec4(group_shift + cell_size * glm::vec3(float(i) - middle, float(j) - middle, float(k) - middle), 0.0f);
        data[index].rotor = glm::vec4(glm::sphericalRand(1.0f), 2.0f * glm::gaussRand(0.0f, 1.0f));
        index++;
    };
};

//=======================================================================================================================================================================================================================
// Computes reflection matrix
// n is the normal vector to the plane, d is the distance from the plane to the origin, 
// so its equation is <n, v> + d = 0
//=======================================================================================================================================================================================================================
glm::mat4 reflection_matrix(const glm::vec3& n, float d)
{
    float m_2xy = -2.0f * n.x * n.y;
    float m_2xz = -2.0f * n.x * n.z;
    float m_2yz = -2.0f * n.y * n.z;

    return glm::mat4(glm::vec4(1.0f - 2.0f * n.x * n.x,                   m_2xy,                   m_2xz, 0.0f),
                     glm::vec4(                  m_2xy, 1.0f - 2.0f * n.y * n.y,                   m_2yz, 0.0f),
                     glm::vec4(                  m_2xz,                   m_2yz, 1.0f - 2.0f * n.z * n.z, 0.0f),
                     glm::vec4(-2.0f * d * n, 1.0f));
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

    demo_window_t window("Marble room", 4, 3, 3, ovr_hmd.mirror_size.x, ovr_hmd.mirror_size.y, false);

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
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, ovr_hmd.target_size.x, ovr_hmd.target_size.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &mirror_fbo_id);

    //===================================================================================================================================================================================================================
    // phong lighting model shader initialization : for room rendering
    //===================================================================================================================================================================================================================
    glsl_program_t phong_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_light.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_light.fs"));

    phong_light.enable();
    uniform_t uni_pv_matrix = phong_light["projection_view_matrix"];
    uniform_t uni_light_ws  = phong_light["light_ws"];
    uniform_t uni_camera_ws = phong_light["camera_ws"];

    phong_light["diffuse_tex"] = 12;
    phong_light["normal_tex"] = 13;

    //===================================================================================================================================================================================================================
    // phong lighting model shader initialization : for instanced objects rendering
    //===================================================================================================================================================================================================================
    glsl_program_t phong_light_inst(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_light_inst.vs"),
                                    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_light_inst.fs"));

    phong_light_inst.enable();
    uniform_t uni_inst_pv_matrix   = phong_light_inst["projection_view_matrix"];
    uniform_t uni_inst_light_ws    = phong_light_inst["light_ws"];
    uniform_t uni_inst_camera_ws   = phong_light_inst["camera_ws"];
    uniform_t uni_inst_diffuse_tex = phong_light_inst["diffuse_tex"];
    uniform_t uni_inst_normal_tex  = phong_light_inst["normal_tex"];
    uniform_t uni_inst_time        = phong_light_inst["time"];
    uniform_t uni_inst_base        = phong_light_inst["buffer_base"];
    uniform_t uni_inst_solid_scale = phong_light_inst["solid_scale"];

    //===================================================================================================================================================================================================================
    // Initialize buffers : position + tangent frame + texture coordinates 
    //===================================================================================================================================================================================================================
    polyhedron tetrahedron, cube, octahedron, dodecahedron, icosahedron;
    tetrahedron.regular_pft2_vao(4, 4, plato::tetrahedron::vertices, plato::tetrahedron::normals, plato::tetrahedron::faces);
    cube.regular_pft2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces);
    octahedron.regular_pft2_vao(6, 8, plato::octahedron::vertices, plato::octahedron::normals, plato::octahedron::faces);
    dodecahedron.regular_pft2_vao(20, 12, plato::dodecahedron::vertices, plato::dodecahedron::normals, plato::dodecahedron::faces);
    icosahedron.regular_pft2_vao(12, 20, plato::icosahedron::vertices, plato::icosahedron::normals, plato::icosahedron::faces);

    //===================================================================================================================================================================================================================
    // Creating toral mesh
    //===================================================================================================================================================================================================================
    surface_t torus;
    torus.generate_vao<vertex_pft2_t>(torus_func, 128, 32);

    //===================================================================================================================================================================================================================
    // Load textures : diffuse + bump for each polyhedron
    //===================================================================================================================================================================================================================
    const int TEXTURE_COUNT = 12;
    GLuint texture_id[TEXTURE_COUNT];

    const char* texture_filenames [TEXTURE_COUNT] = 
    {
        "../../../resources/plato_tex2d/tetrahedron.png",  "../../../resources/plato_tex2d/tetrahedron_bump.png",
        "../../../resources/plato_tex2d/cube.png",         "../../../resources/plato_tex2d/cube_bump.png",
        "../../../resources/plato_tex2d/octahedron.png",   "../../../resources/plato_tex2d/octahedron_bump.png",
        "../../../resources/plato_tex2d/pentagon.png",     "../../../resources/plato_tex2d/pentagon_bump.png",
        "../../../resources/plato_tex2d/icosahedron.png",  "../../../resources/plato_tex2d/icosahedron_bump.png",
        "../../../resources/tex2d/torus.png",              "../../../resources/tex2d/torus_bump.png"
    };

    for (int i = 0; i < TEXTURE_COUNT; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        texture_id[i] = image::png::texture2d(texture_filenames[i]);
    }

    glActiveTexture(GL_TEXTURE0 + TEXTURE_COUNT);
    GLuint room_diffuse_tex_id = image::png::texture2d("../../../resources/tex2d/pink_stone.png");

    glActiveTexture(GL_TEXTURE0 + TEXTURE_COUNT + 1);
    GLuint room_normal_tex_id = image::png::texture2d("../../../resources/tex2d/pink_stone_bump.png");

    //===================================================================================================================================================================================================================
    // Initialize objects displacement vectors and rotation axes, and write the data to GL_SHADER_STORAGE_BUFFER
    // The buffer will be read according to gl_InstanceID variable and buffer_base uniform
    //===================================================================================================================================================================================================================
    const int N = 4;
    const int group_size = N * N * N;
    const float cell_size = 1.5f;
    const float origin = 1.5f * cell_size * N;
    const GLsizeiptr chunk_size = group_size * sizeof(motion3d_t);  

    motion3d_t data[6 * group_size];

    fill_shift_rotor_data(&data[0 * group_size], glm::vec3(   0.0f,    0.0f,  origin), cell_size, N);
    fill_shift_rotor_data(&data[1 * group_size], glm::vec3(   0.0f,    0.0f, -origin), cell_size, N);
    fill_shift_rotor_data(&data[2 * group_size], glm::vec3(   0.0f,  origin,    0.0f), cell_size, N);
    fill_shift_rotor_data(&data[3 * group_size], glm::vec3(   0.0f, -origin,    0.0f), cell_size, N);
    fill_shift_rotor_data(&data[4 * group_size], glm::vec3( origin,    0.0f,    0.0f), cell_size, N);
    fill_shift_rotor_data(&data[5 * group_size], glm::vec3(-origin,    0.0f,    0.0f), cell_size, N);

    GLuint ssbo_id;
    glGenBuffers(1, &ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ssbo_id, 0, sizeof(data));

    //===================================================================================================================================================================================================================
    // light variables
    //===================================================================================================================================================================================================================

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(torus.vao.ibo.pri);

    const float light_radius = 1.5 * cell_size * N; 
    const float cube_size = 7.5f * cell_size * N;

    polyhedron granite_room;
    granite_room.regular_pft2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces, cube_size, true);

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

        //===============================================================================================================================================================================================================
        // render the scene for both eyes
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ovr_hmd.swapchain_texture_id(), 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = window.frame_ts;

        float cs0 = glm::cos(0.25f * time);
        float sn0 = glm::sin(0.25f * time);
        float cs1 = glm::cos(0.1317f * time);
        float sn1 = glm::sin(0.1317f * time);
        glm::vec3 light_ws = light_radius * glm::vec3(cs0 * cs1, sn0 * cs1, sn1);


        for (ovrEyeType eye = ovrEyeType::ovrEye_Left; eye < ovrEyeType::ovrEye_Count; eye = static_cast<ovrEyeType>(eye + 1))
        {
            ovr_hmd.set_viewport(eye);
            glm::mat4 projection_matrix = ovr_hmd.projection_matrix[eye];
            glm::mat4 view_matrix = window.camera.eye_view_matrix[eye];
            glm::vec3 camera_ws = window.camera.position(eye);
            glm::mat4 projection_view_matrix = projection_matrix * view_matrix;

            //===============================================================================================================================================================================================================
            // Render objects
            //===============================================================================================================================================================================================================
            phong_light_inst.enable();
            uni_inst_pv_matrix = projection_view_matrix;
            uni_inst_camera_ws = camera_ws;
            uni_inst_light_ws  = light_ws;
            uni_inst_time = time;

            uni_inst_base = (int) 0 * group_size;
            uni_inst_diffuse_tex = 0;
            uni_inst_normal_tex  = 1;
            uni_inst_solid_scale = 0.95f;
            tetrahedron.instanced_render(group_size);
    
            uni_inst_base = (int) 1 * group_size;
            uni_inst_diffuse_tex = 2;
            uni_inst_normal_tex  = 3;
            uni_inst_solid_scale = 0.75f;
            cube.instanced_render(group_size);
    
            uni_inst_base = (int) 2 * group_size;
            uni_inst_diffuse_tex = 4;
            uni_inst_normal_tex  = 5;
            uni_inst_solid_scale = 0.85f;
            octahedron.instanced_render(group_size);
    
            uni_inst_base = (int) 3 * group_size;
            uni_inst_diffuse_tex = 6;
            uni_inst_normal_tex  = 7;
            uni_inst_solid_scale = 0.75f;
            dodecahedron.instanced_render(group_size);
    
            uni_inst_base = (int) 4 * group_size;
            uni_inst_diffuse_tex = 8;
            uni_inst_normal_tex  = 9;
            uni_inst_solid_scale = 0.8f;
            icosahedron.instanced_render(group_size);
    
            uni_inst_base = (int) 5 * group_size;
            uni_inst_diffuse_tex = 10;
            uni_inst_normal_tex  = 11;
            uni_inst_solid_scale = 0.67f;
            torus.instanced_render(group_size);

            //===============================================================================================================================================================================================================
            // Render room
            //===============================================================================================================================================================================================================
            phong_light.enable();
            uni_pv_matrix = projection_view_matrix;
            uni_camera_ws = camera_ws;
            uni_light_ws  = light_ws;

            granite_room.render();
        }

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

        //===============================================================================================================================================================================================================
        // submit the texture to vr device
        //===============================================================================================================================================================================================================
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