//========================================================================================================================================================================================================================
// OCULUS DEMO 005 : Attractor
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
#include "polyhedron.hpp"
#include "plato.hpp"

const float z_near = 0.5f;

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
    bool pause = false;

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

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE))
            pause = !pause;

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

glm::vec3 tri(const glm::vec3& x)
{
    return glm::abs(glm::fract(x) - glm::vec3(0.5f));
}

float potential(const glm::vec3& p)
{    
    glm::vec3 q = p;
    glm::vec3 oq = tri(1.1f * q + tri(1.1f * glm::vec3(q.z, q.x, q.y)));
    float ground = q.z + 3.5f + glm::dot(oq, glm::vec3(0.067));
    q += (oq - glm::vec3(0.25f)) * 0.3f;
    q = glm::cos(0.444f * q + glm::sin(1.112f * glm::vec3(q.z, q.x, q.y)));
    float canyon = 0.95f * (glm::length(p) - 1.05f);
    float sphere = 11.0f - glm::length(p);
    return glm::min(glm::min(ground, canyon), sphere);
}

glm::vec3 gradient(const glm::vec3& p)
{
    const float delta = 0.075f;

    const glm::vec3 dX = glm::vec3(delta, 0.0f, 0.0f);
    const glm::vec3 dY = glm::vec3(0.0f, delta, 0.0f);
    const glm::vec3 dZ = glm::vec3(0.0f, 0.0f, delta);

    glm::vec3 dF = glm::vec3
    (
        potential(p + dX) - potential(p - dX),
        potential(p + dY) - potential(p - dY),
        potential(p + dZ) - potential(p - dZ)
    );

    return glm::normalize(dF);
}

glm::vec3 move(glm::vec3& position, glm::vec3& velocity, float dt)
{
    glm::vec3 v0 = gradient(position);
    glm::vec3 v1 = velocity;

    glm::vec3 v = glm::normalize(v1 + v0);
    velocity = v;
    position = position + dt * v;
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

    demo_window_t window("Ray Marcher", 4, 4, 3, ovr_hmd.mirror_size.x, ovr_hmd.mirror_size.y, false);

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
    // Shader and uniform variables initialization
    //===================================================================================================================================================================================================================
    glsl_program_t ray_marcher(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ray_marcher.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/canyon.fs"));
    ray_marcher.enable();
    uniform_t uni_rm_camera_matrix = ray_marcher["camera_matrix"];
    uniform_t uni_rm_camera_ws = ray_marcher["camera_ws"];
    uniform_t uni_rm_light_ws = ray_marcher["light_ws"];
    uniform_t uni_rm_time = ray_marcher["time"];
    uniform_t uni_rm_focal_scale = ray_marcher["focal_scale"];
    uniform_t uni_rm_focal_shift = ray_marcher["focal_shift"];

    ray_marcher["stone_tex"] = 0;
    ray_marcher["grass_tex"] = 1;
    ray_marcher["z_near"] = z_near;

    glm::vec2 focal_scale[ovrEye_Count] = 
    {
        glm::vec2(1.0f / ovr_hmd.projection_matrix[ovrEye_Left ][0][0], 1.0f / ovr_hmd.projection_matrix[ovrEye_Left ][1][1]),
        glm::vec2(1.0f / ovr_hmd.projection_matrix[ovrEye_Right][0][0], 1.0f / ovr_hmd.projection_matrix[ovrEye_Right][1][1]),
    };

    glm::vec2 focal_shift[ovrEye_Count] = 
    {
        glm::vec2(ovr_hmd.projection_matrix[ovrEye_Left ][2][0], ovr_hmd.projection_matrix[ovrEye_Left ][2][1]),
        glm::vec2(ovr_hmd.projection_matrix[ovrEye_Right][2][0], ovr_hmd.projection_matrix[ovrEye_Right][2][1]),
    };

    //===================================================================================================================================================================================================================
    // phong lighting model shader initialization : for room rendering
    //===================================================================================================================================================================================================================
    glsl_program_t phong_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_light.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_light.fs"));

    phong_light.enable();
    uniform_t uni_pl_pv_matrix = phong_light["projection_view_matrix"];
    uniform_t uni_pl_light_ws  = phong_light["light_ws"];
    uniform_t uni_pl_camera_ws = phong_light["camera_ws"];
    phong_light["diffuse_tex"] = 2;
    phong_light["normal_tex"] = 3;
    phong_light["shift"] = glm::vec3(0.0f, 0.0f, 2.75f);

    //===================================================================================================================================================================================================================
    // grass generating shader
    //===================================================================================================================================================================================================================
    glsl_program_t grass_generator(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/grass_gen.vs"),
                                   glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/grass_gen.gs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/grass_gen.fs"));

    grass_generator.enable();
    uniform_t uni_gg_pv_matrix = grass_generator["projection_view_matrix"];
    uniform_t uni_gg_light_ws  = grass_generator["light_ws"];
    uniform_t uni_gg_camera_ws = grass_generator["camera_ws"];
    uniform_t uni_gg_origin    = grass_generator["origin"];

    const float inv_grass_scale = 32.0f;
    grass_generator["grass_scale"] = 1.0f / inv_grass_scale;

    //===================================================================================================================================================================================================================
    // load textures
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint stone_tex_id = image::png::texture2d("../../../resources/tex2d/moss.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    glActiveTexture(GL_TEXTURE1);
    GLuint grass_tex_id = image::png::texture2d("../../../resources/tex2d/ground_grass.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    glActiveTexture(GL_TEXTURE2);
    GLuint room_diffuse_tex_id = image::png::texture2d("../../../resources/tex2d/pink_stone.png");

    glActiveTexture(GL_TEXTURE3);
    GLuint room_normal_tex_id = image::png::texture2d("../../../resources/tex2d/pink_stone_bump.png");

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    glm::vec3 light_ws = glm::sphericalRand(11.0f);
    glm::vec3 light_velocity = glm::sphericalRand(1.0f);
    float p = potential(light_ws);

    while(p < 0.5f)
    {
        move(light_ws, light_velocity, 0.125f);
        p = potential(light_ws);
    }

    const float cube_size = 2.75;
    polyhedron cube;
    cube.regular_pft2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces, cube_size);

    glEnable(GL_DEPTH_TEST);

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
        if (!window.pause)
        {
            move(light_ws, light_velocity, window.frame_dt);
            float p = potential(light_ws);

            while(p < 0.5f)
            {
                move(light_ws, light_velocity, 0.125f);
                p = potential(light_ws);
            }
        }

        //===============================================================================================================================================================================================================
        // bind swapchain texture ...
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ovr_hmd.swapchain_texture_id(), 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (ovrEyeType eye = ovrEyeType::ovrEye_Left; eye < ovrEyeType::ovrEye_Count; eye = static_cast<ovrEyeType>(eye + 1))
        {
            ovr_hmd.set_viewport(eye);            


            glm::mat4 projection_matrix = ovr_hmd.projection_matrix[eye];
            glm::mat4 view_matrix = window.camera.eye_view_matrix[eye];
            glm::mat4 projection_view_matrix = projection_matrix * view_matrix;
            glm::mat4 cmatrix4x4 = glm::inverse(window.camera.eye_view_matrix[eye]);
            glm::mat3 camera_matrix = glm::mat3(cmatrix4x4);
            glm::vec3 camera_ws = glm::vec3(cmatrix4x4[3]);

            //===========================================================================================================================================================================================================
            // render raymarch scene
            //===========================================================================================================================================================================================================
            glDepthFunc(GL_ALWAYS);
            ray_marcher.enable();

            uni_rm_camera_matrix = camera_matrix;
            uni_rm_light_ws = light_ws;
            uni_rm_camera_ws = camera_ws;
            uni_rm_focal_scale = focal_scale[eye];
            uni_rm_focal_shift = focal_shift[eye];
            uni_rm_time = time;

            glBindVertexArray(vao_id);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            //===========================================================================================================================================================================================================
            // render cube
            //===========================================================================================================================================================================================================
            glDepthFunc(GL_LESS);
            phong_light.enable();

            uni_pl_pv_matrix = projection_view_matrix;
            uni_pl_light_ws  = light_ws;
            uni_pl_camera_ws = camera_ws;

            cube.render();

            //===============================================================================================================================================================================================================
            // render grass
            //===============================================================================================================================================================================================================
            grass_generator.enable();
            glBindVertexArray(vao_id);

            uni_gg_pv_matrix = projection_view_matrix;
            uni_gg_light_ws  = light_ws;
            uni_gg_camera_ws = camera_ws;

            const int half_res = 256;
            const int full_res = 2 * half_res + 1;
            uni_gg_origin = glm::ivec2(inv_grass_scale * camera_ws.x, inv_grass_scale * camera_ws.x) - glm::ivec2(half_res);

            glDrawArraysInstanced(GL_POINTS, 0, full_res, full_res);
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
    glDeleteVertexArrays(1, &vao_id);
    glDeleteTextures(1, &stone_tex_id);
    glDeleteTextures(1, &grass_tex_id);
    glDeleteTextures(1, &room_diffuse_tex_id);
    glDeleteTextures(1, &room_normal_tex_id);

    glfw::terminate();
    return 0;
}