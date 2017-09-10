#include <iostream>
#include <memory>
#include <algorithm>
//#include <Windows.h>

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

#include <GL/glew.h>
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include <GLFW/glfw3.h>

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

    //===================================================================================================================================================================================================================
    // Mouse / keyboard control accumulated position and orientation
    //===================================================================================================================================================================================================================
    glm::mat4 view_matrix;

    //===================================================================================================================================================================================================================
    // Local HMD-eye view transfomation matrices, assumed to be set once
    //===================================================================================================================================================================================================================
    glm::mat4 eye_view_matrix[ovrEye_Count];

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
        double theta = angular_speed * dt;
        double dx = -direction.x;
        double dy = direction.y;
        double cs = glm::cos(theta);
        double sn = glm::sin(theta);
        double _1mcs = 1.0 - cs;
        float sndx = sn * dx;
        float sndy = sn * dy;
        float _1mcsdx = _1mcs * dx;
        float _1mcsdy = _1mcs * dy;
        glm::mat4 rotation_matrix = glm::mat4 (1.0f - dx * _1mcsdx,      - dy * _1mcsdx,  sndx, 0.0f, 
                                                    - dx * _1mcsdy, 1.0f - dy * _1mcsdy,  sndy, 0.0f,
                                                            - sndx,              - sndy,    cs, 0.0f,
                                                              0.0f,                0.0f,  0.0f, 1.0f);
        view_matrix = rotation_matrix * view_matrix;
    }

    glm::mat4 camera_matrix()
        { return glm::inverse(view_matrix); }

    void set_hmd_view_matrix(const glm::quat& rotation, const glm::vec3& position)
    {
        orientation = glm::mat4_cast(rotation);
        translation = glm::translate(position);
        hmd_view_matrix = glm::inverse(orientation) * glm::inverse(translation);
    }

    void set_eye_local_matrix(ovrEyeType eye, const glm::quat& rotation, const glm::vec3& position)
    {
        glm::mat4 local_orientation = glm::mat4_cast(rotation);
        glm::mat4 local_translation = glm::translate(position);
        eye_view_matrix[eye] = glm::inverse(local_orientation) * glm::inverse(local_translation);
    }    

    glm::mat4 get_eye_view_matrix(ovrEyeType eye)
    {
        return eye_view_matrix[eye] * hmd_view_matrix * view_matrix;
    }

    glm::vec3 position() const
        { return -glm::inverse(glm::mat3(view_matrix)) * glm::vec3(view_matrix[3]); }
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


glm::mat4 perspective(const ovrFovPort& fov, float zNear, float zFar)
{
    float x_scale  = 2.0f / (fov.LeftTan + fov.RightTan);
    float x_offset = 0.5f * (fov.LeftTan - fov.RightTan) * x_scale;
    float y_scale  = 2.0f / (fov.UpTan + fov.DownTan);
    float y_offset = 0.5f * (fov.UpTan - fov.DownTan) * y_scale;
    float dz = zNear - zFar;

    return glm::mat4(glm::vec4(  x_scale,     0.0f,                       0.0f,  0.0f),
                     glm::vec4(     0.0f,  y_scale,                       0.0f,  0.0f),
                     glm::vec4(-x_offset, y_offset,        (zNear + zFar) / dz, -1.0f),
                     glm::vec4(     0.0f,     0.0f, 2.0f * (zFar * zNear) / dz,  0.0f)); 
}

glm::mat4 infinite_perspective(const ovrFovPort& fov, float zNear)
{
    float x_scale  = 2.0f / (fov.LeftTan + fov.RightTan);
    float x_offset = 0.5f * (fov.LeftTan - fov.RightTan) * x_scale;
    float y_scale  = 2.0f / (fov.UpTan + fov.DownTan);
    float y_offset = 0.5f * (fov.UpTan - fov.DownTan) * y_scale;

    return glm::mat4(glm::vec4(  x_scale,     0.0f,          0.0f,  0.0f),
                     glm::vec4(     0.0f,  y_scale,          0.0f,  0.0f),
                     glm::vec4(-x_offset, y_offset,         -1.0f, -1.0f),
                     glm::vec4(     0.0f,     0.0f, -2.0f * zNear,  0.0f)); 
}

//=======================================================================================================================================================================================================================
// static variables
//=======================================================================================================================================================================================================================

struct ovr_hmd_t
{
    //===================================================================================================================================================================================================================
    // device and session variables
    //===================================================================================================================================================================================================================
    ovrSession session;
    ovrGraphicsLuid luid;
    ovrHmdDesc hmd_desc;

    //===================================================================================================================================================================================================================
    // rendering API variables
    //===================================================================================================================================================================================================================
    int swapchain_length;

    glm::ivec2 target_size;
    ovrTextureSwapChain target_texture;

    glm::ivec2 mirror_size;
    ovrMirrorTexture mirror_texture;

    //===================================================================================================================================================================================================================
    // geometric variables : 
    //===================================================================================================================================================================================================================
    ovrEyeRenderDesc _eyeRenderDescs[ovrEye_Count];
    ovrViewScaleDesc _viewScaleDesc;
    ovrLayerEyeFov _sceneLayer;

    glm::mat4 projection_matrix[ovrEye_Count];

    glm::quat head_rotation;
    glm::vec3 head_position;

    glm::quat eye_rotation[ovrEye_Count];
    glm::vec3 eye_position[ovrEye_Count];

    glm::quat eye_rotation_rel[ovrEye_Count];
    glm::vec3 eye_position_rel[ovrEye_Count];

    ovr_hmd_t()
        : target_size(0)
    {
        ovrResult result;
        ovrErrorInfo errorInfo;

        //===============================================================================================================================================================================================================
        // Initialize the library
        //===============================================================================================================================================================================================================
        debug_msg("Initializing Octulus Library ...");
        result = ovr_Initialize(0);
        if(result < 0)
        {
            ovr_GetLastErrorInfo(&errorInfo);
            exit_msg("ovr_Initialize failed : code = %d. %s", result, errorInfo.ErrorString);
        }

        //===============================================================================================================================================================================================================
        // Create session = get access to the device
        //===============================================================================================================================================================================================================
        debug_msg("Creating OVR session ...");
        result = ovr_Create(&session, &luid);
        if(result < 0)
        {
            ovr_GetLastErrorInfo(&errorInfo);
            exit_msg("ovr_Create failed : code = %d. %s", result, errorInfo.ErrorString);
        }

        //===============================================================================================================================================================================================================
        // Get and log device description
        //===============================================================================================================================================================================================================
        debug_msg("Requesting HMD description ...");
        hmd_desc = ovr_GetHmdDesc(session);
        dump_info();

        //===============================================================================================================================================================================================================
        // Determine texture size and set up rendering description structure
        //===============================================================================================================================================================================================================
        _viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

        memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
        _sceneLayer.Header.Type = ovrLayerType_EyeFov;
        _sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft; // | ovrLayerFlag_HighQuality

        for (ovrEyeType eye = ovrEyeType::ovrEye_Left; eye < ovrEyeType::ovrEye_Count; eye = static_cast<ovrEyeType>(eye + 1))
        {
            _eyeRenderDescs[eye] = ovr_GetRenderDesc(session, eye, hmd_desc.DefaultEyeFov[eye]);
            projection_matrix[eye] = infinite_perspective(_eyeRenderDescs[eye].Fov, 0.5f);        
            _viewScaleDesc.HmdToEyePose[eye] = _eyeRenderDescs[eye].HmdToEyePose;

            eye_rotation_rel[eye] = glm::make_quat(&_viewScaleDesc.HmdToEyePose[eye].Orientation.x);
            eye_position_rel[eye] = glm::make_vec3(&_viewScaleDesc.HmdToEyePose[eye].Position.x);

            const glm::quat& rotation = eye_rotation_rel[eye];
            const glm::vec3& position = eye_position_rel[eye];

            debug_msg("HmdToEyePose[%u] : Orientation = Quat {%.5f, %.5f, %.5f, %.5f}", eye, rotation.x, rotation.y, rotation.z, rotation.w);
            debug_msg("                   Position    = Vec3 {%.5f, %.5f, %.5f}", position.x, position.y, position.z);

            _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
            ovrSizei texture_size = ovr_GetFovTextureSize(session, eye, _eyeRenderDescs[eye].Fov, 1.0f);
            _sceneLayer.Viewport[eye] = ovrRecti 
            {
                .Pos = { .x = target_size.x, .y = 0 }, 
                .Size = texture_size
            };

            target_size.x += texture_size.w;
            target_size.y = std::max(target_size.y, texture_size.h);
        }

        mirror_size = target_size / 2;        

        debug_msg("target_size = %d x %d.", target_size.x, target_size.y);
        debug_msg("mirror_size = %d x %d.", mirror_size.x, mirror_size.y);
    }    

    void create_swap_chain()
    {
        ovrTextureSwapChainDesc desc = 
        {
            .Type = ovrTexture_2D,
            .Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB,
            .ArraySize = 1,
            .Width = target_size.x,
            .Height = target_size.y,
            .MipLevels = 1,
            .SampleCount = 1,
            .StaticImage = ovrFalse,
            .MiscFlags = 0,
            .BindFlags = 0
        };

        ovrResult result = ovr_CreateTextureSwapChainGL(session, &desc, &target_texture);
        _sceneLayer.ColorTexture[ovrEye_Left]  = target_texture;
        _sceneLayer.ColorTexture[ovrEye_Right] = target_texture;
        if (result < 0)
            exit_msg("Failed to create swap textures");

        result = ovr_GetTextureSwapChainLength(session, target_texture, &swapchain_length);
        if (result < 0)
            exit_msg("Unable to count swap chain textures");

        //===============================================================================================================================================================================================================
        // create OVR texture swapchain
        //===============================================================================================================================================================================================================
        debug_msg("Swapchain length :: %u", swapchain_length);
        for (int i = 0; i < swapchain_length; ++i)
        {
            GLuint texture_id;
            ovr_GetTextureSwapChainBufferGL(session, target_texture, i, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void create_mirror_texture()
    {
        ovrMirrorTextureDesc mirror_texture_desc = 
        {
            .Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB,
            .Width  = mirror_size.x,
            .Height = mirror_size.y,
            .MiscFlags = 0
        };

        if (ovr_CreateMirrorTextureGL(session, &mirror_texture_desc, &mirror_texture) < 0)
            exit_msg("Could not create mirror texture");
    }

    void update_tracking(int frame)
    {
        double frame_time = ovr_GetPredictedDisplayTime(session, frame);
        ovrTrackingState tracking_state = ovr_GetTrackingState(session, frame_time, true);

        const ovrPosef& head_pose = tracking_state.HeadPose.ThePose;
        head_rotation = glm::make_quat(&head_pose.Orientation.x);
        head_position = glm::make_vec3(&head_pose.Position.x);

        eye_rotation[ovrEye_Left ] = head_rotation * eye_rotation_rel[ovrEye_Left ];
        eye_rotation[ovrEye_Right] = head_rotation * eye_rotation_rel[ovrEye_Right];

        eye_position[ovrEye_Left ] = head_position + head_rotation * eye_position_rel[ovrEye_Left ];
        eye_position[ovrEye_Right] = head_position + head_rotation * eye_position_rel[ovrEye_Right];

        _sceneLayer.SensorSampleTime = ovr_GetTimeInSeconds();
    }

    void set_viewport(ovrEyeType eye)
    {
        const ovrRecti& viewport = _sceneLayer.Viewport[eye];
        glViewport(viewport.Pos.x, viewport.Pos.y, viewport.Size.w, viewport.Size.h);
        const glm::quat& rotation = eye_rotation[eye];
        const glm::quat& position = eye_position[eye];
        _sceneLayer.RenderPose[eye].Orientation = ovrQuatf { .x = rotation.x , .y = rotation.y, .z = rotation.z, .w = rotation.w};
        _sceneLayer.RenderPose[eye].Position = ovrVector3f { .x = position.x , .y = position.y, .z = position.z };
    }

    GLuint mirror_texture_id()
    {
        GLuint texture_id;
        ovr_GetMirrorTextureBufferGL(session, mirror_texture, &texture_id);
        return texture_id;
    }

    GLuint swapchain_texture_id()
    {
        int index;
        ovr_GetTextureSwapChainCurrentIndex(session, target_texture, &index);
        GLuint texture_id;
        ovr_GetTextureSwapChainBufferGL(session, target_texture, index, &texture_id);
        return texture_id;
    }

    void submit_frame(int frame)
    {
        ovr_CommitTextureSwapChain(session, target_texture);
        ovrLayerHeader* layer_header = &_sceneLayer.Header;
        ovr_SubmitFrame(session, frame, &_viewScaleDesc, &layer_header, 1);
    }

    void dump_info()
    {
        /* Basic device information */
        const char* type_str;
        switch (hmd_desc.Type)
        {
            case ovrHmd_None    : type_str = "None";    break;
            case ovrHmd_DK1     : type_str = "DK1";     break;
            case ovrHmd_DKHD    : type_str = "DKHD";    break;
            case ovrHmd_DK2     : type_str = "DK2";     break;
            case ovrHmd_CB      : type_str = "CB";      break;
            case ovrHmd_Other   : type_str = "Other";   break;
            case ovrHmd_E3_2015 : type_str = "E3_2015"; break;
            case ovrHmd_ES06    : type_str = "ES06";    break;
            case ovrHmd_ES09    : type_str = "ES09";    break;
            case ovrHmd_ES11    : type_str = "ES11";    break;
            case ovrHmd_CV1     : type_str = "CV1";     break;
            default             : type_str = "Unknown";
        }

        debug_msg("\tVR Device information : type = %s (value = %d)", type_str, hmd_desc.Type);
        debug_msg("\tProduct Name = %s", hmd_desc.ProductName);
        debug_msg("\tManufacturer = %s", hmd_desc.Manufacturer);
        debug_msg("\tVendorId = %d", hmd_desc.VendorId);
        debug_msg("\tProductId = %d", hmd_desc.ProductId);
        debug_msg("\tSerialNumber = %.24s", hmd_desc.SerialNumber);
        debug_msg("\tFirmware = %d.%d", hmd_desc.FirmwareMajor, hmd_desc.FirmwareMinor);

        /* HMD (Head-Mounted Display) capabilities, currently this includes only debug mode availability flag */
        debug_msg("\tAvailableHmdCaps = %x", hmd_desc.AvailableHmdCaps);
            debug_msg("\t\tDebug mode is %s", hmd_desc.AvailableHmdCaps & ovrHmdCap_DebugDevice ? "available" : "not available");
        debug_msg("\tDefaultHmdCaps = %x", hmd_desc.DefaultHmdCaps);
            debug_msg("\t\tDebug mode is %s by default", hmd_desc.AvailableHmdCaps & ovrHmdCap_DebugDevice ? "on" : "off");
    
        /* Tracking capabilities */
        debug_msg("\tAvailableTrackingCaps = %x", hmd_desc.AvailableTrackingCaps);
            debug_msg("\t\tOrientation tracking (IMU) : %s", hmd_desc.AvailableTrackingCaps & ovrTrackingCap_Orientation      ? "supported" : "not supported");
            debug_msg("\t\tYaw drift correction       : %s", hmd_desc.AvailableTrackingCaps & ovrTrackingCap_MagYawCorrection ? "supported" : "not supported");
            debug_msg("\t\tPositional tracking        : %s", hmd_desc.AvailableTrackingCaps & ovrTrackingCap_Position         ? "supported" : "not supported");

        debug_msg("\tDefaultTrackingCaps = %x", hmd_desc.DefaultTrackingCaps);
            debug_msg("\t\tOrientation tracking (IMU) : %s", hmd_desc.AvailableTrackingCaps & ovrTrackingCap_Orientation      ? "on" : "off");
            debug_msg("\t\tYaw drift correction       : %s", hmd_desc.AvailableTrackingCaps & ovrTrackingCap_MagYawCorrection ? "on" : "off");
            debug_msg("\t\tPositional tracking        : %s", hmd_desc.AvailableTrackingCaps & ovrTrackingCap_Position         ? "on" : "off");

        for (int i = 0; i < ovrEye_Count; ++i)
        {
            debug_msg("\tLenses info : lens #%d :", i);
            debug_msg("\t\tDefault FOV : up(%.2f), down(%.2f), left(%.2f), right(%.2f)", 
                    constants::one_rad * glm::atan(hmd_desc.DefaultEyeFov[i].UpTan),
                    constants::one_rad * glm::atan(hmd_desc.DefaultEyeFov[i].DownTan),
                    constants::one_rad * glm::atan(hmd_desc.DefaultEyeFov[i].LeftTan),
                    constants::one_rad * glm::atan(hmd_desc.DefaultEyeFov[i].RightTan));
            debug_msg("\t\tMax FOV : up(%.2f), down(%.2f), left(%.2f), right(%.2f)", 
                    constants::one_rad * glm::atan(hmd_desc.MaxEyeFov[i].UpTan),
                    constants::one_rad * glm::atan(hmd_desc.MaxEyeFov[i].DownTan),
                    constants::one_rad * glm::atan(hmd_desc.MaxEyeFov[i].LeftTan),
                    constants::one_rad * glm::atan(hmd_desc.MaxEyeFov[i].RightTan));
        }   

        debug_msg("\tResolution = %d x %d.", hmd_desc.Resolution.w, hmd_desc.Resolution.h);
        debug_msg("\tRefresh rate = %f cycles per second.", hmd_desc.DisplayRefreshRate);
    }

    ~ovr_hmd_t()
    {
        //===============================================================================================================================================================================================================
        // Destroy OVR session and shutdown the library
        //===============================================================================================================================================================================================================
        ovr_Destroy(session);
        ovr_Shutdown();
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

    ovr_RecenterTrackingOrigin(ovr_hmd.session);

	//===================================================================================================================================================================================================================
	// phong lighting model shader initialization
	//===================================================================================================================================================================================================================
    glsl_program_t phong_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_light.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_light.fs"));

	phong_light.enable();
	uniform_t uni_pv_matrix = phong_light["projection_view_matrix"];
    uniform_t uni_light_ws  = phong_light["light_ws"];
    uniform_t uni_camera_ws = phong_light["camera_ws"];

    phong_light["diffuse_tex"] = 0;
    phong_light["normal_tex"] = 1;

    glActiveTexture(GL_TEXTURE0);
    GLuint marble_texture_id = image::png::texture2d("../../../resources/tex2d/marble.png");

	//===================================================================================================================================================================================================================
	// light variables
	//===================================================================================================================================================================================================================
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    const float cube_size = 10.0f;
    const float light_radius = 5.0f;

    room_t granite_room(cube_size);

    //===================================================================================================================================================================================================================
    // Main rendering loop
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
        glm::vec3 light_ws = light_radius * glm::vec3(cos(0.5f * time), sin(0.5f * time), -0.66f);

        phong_light.enable();

        for (ovrEyeType eye = ovrEyeType::ovrEye_Left; eye < ovrEyeType::ovrEye_Count; eye = static_cast<ovrEyeType>(eye + 1))
        {
            ovr_hmd.set_viewport(eye);

			glm::mat4 projection_matrix = ovr_hmd.projection_matrix[eye];
            glm::mat4 view_matrix = window.camera.get_eye_view_matrix(eye);

            glm::vec3 camera_ws = window.camera.position();
            glm::mat4 projection_view_matrix = projection_matrix * view_matrix;
    
            uni_pv_matrix = projection_view_matrix;
            uni_light_ws  = light_ws;
            uni_camera_ws = camera_ws;
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
    // Destroy GLFW window and terminate the library
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}