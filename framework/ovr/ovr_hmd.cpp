
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/transform.hpp>

#include "ovr_hmd.hpp"
#include "log.hpp"
#include "constants.hpp"
#include "matrix_transform.hpp"

ovr_hmd_t::ovr_hmd_t()
    : target_size(0)
{
    ovrResult result;
    ovrErrorInfo errorInfo;

    //===================================================================================================================================================================================================================
    // Initialize the library                                                                                                                                                                                            
    //===================================================================================================================================================================================================================
    debug_msg("Initializing Octulus Library ...");
    result = ovr_Initialize(0);
    if(result < 0)
    {
        ovr_GetLastErrorInfo(&errorInfo);
        exit_msg("ovr_Initialize failed : code = %d. %s", result, errorInfo.ErrorString);
    }

    //===================================================================================================================================================================================================================
    // Create session = get access to the device                                                                                                                                                                         
    //===================================================================================================================================================================================================================
    debug_msg("Creating OVR session ...");
    result = ovr_Create(&session, &luid);
    if(result < 0)
    {
        ovr_GetLastErrorInfo(&errorInfo);
        exit_msg("ovr_Create failed : code = %d. %s", result, errorInfo.ErrorString);
    }

    //===================================================================================================================================================================================================================
    // Get and log device description                                                                                                                                                                                    
    //===================================================================================================================================================================================================================
    debug_msg("Requesting HMD description ...");
    hmd_desc = ovr_GetHmdDesc(session);
    dump_info();

    //===================================================================================================================================================================================================================
    // Determine texture size and set up rendering description structure                                                                                                                                                 
    //===================================================================================================================================================================================================================
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

void ovr_hmd_t::create_swap_chain()
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

    //===================================================================================================================================================================================================================
    // create OVR texture swapchain                                                                                                                                                                                      
    //===================================================================================================================================================================================================================
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

void ovr_hmd_t::create_mirror_texture()
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

void ovr_hmd_t::update_tracking(int frame)
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

void ovr_hmd_t::set_viewport(ovrEyeType eye)
{
    const ovrRecti& viewport = _sceneLayer.Viewport[eye];
    glViewport(viewport.Pos.x, viewport.Pos.y, viewport.Size.w, viewport.Size.h);
    const glm::quat& rotation = eye_rotation[eye];
    const glm::quat& position = eye_position[eye];
    _sceneLayer.RenderPose[eye].Orientation = ovrQuatf { .x = rotation.x , .y = rotation.y, .z = rotation.z, .w = rotation.w};
    _sceneLayer.RenderPose[eye].Position = ovrVector3f { .x = position.x , .y = position.y, .z = position.z };
}

void ovr_hmd_t::set_indexed_viewport(ovrEyeType eye, GLuint index)
{
    const ovrRecti& viewport = _sceneLayer.Viewport[eye];
    glViewportIndexedf(index, viewport.Pos.x, viewport.Pos.y, viewport.Size.w, viewport.Size.h);
    const glm::quat& rotation = eye_rotation[eye];
    const glm::quat& position = eye_position[eye];
    _sceneLayer.RenderPose[eye].Orientation = ovrQuatf { .x = rotation.x , .y = rotation.y, .z = rotation.z, .w = rotation.w};
    _sceneLayer.RenderPose[eye].Position = ovrVector3f { .x = position.x , .y = position.y, .z = position.z };
}

GLuint ovr_hmd_t::mirror_texture_id()
{
    GLuint texture_id;
    ovr_GetMirrorTextureBufferGL(session, mirror_texture, &texture_id);
    return texture_id;
}

GLuint ovr_hmd_t::swapchain_texture_id()
{
    int index;
    ovr_GetTextureSwapChainCurrentIndex(session, target_texture, &index);
    GLuint texture_id;
    ovr_GetTextureSwapChainBufferGL(session, target_texture, index, &texture_id);
    return texture_id;
}

void ovr_hmd_t::submit_frame(int frame)
{
    ovr_CommitTextureSwapChain(session, target_texture);
    ovrLayerHeader* layer_header = &_sceneLayer.Header;
    ovr_SubmitFrame(session, frame, &_viewScaleDesc, &layer_header, 1);
}

void ovr_hmd_t::dump_info()
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

    for (ovrEyeType eye = ovrEye_Left; eye < ovrEye_Count; eye = static_cast<ovrEyeType>(eye + 1))
    {
        debug_msg("\tLenses info : lens #%d :", eye);
        debug_msg("\t\tDefault FOV : up(%.2f), down(%.2f), left(%.2f), right(%.2f)", 
                constants::one_rad * glm::atan(hmd_desc.DefaultEyeFov[eye].UpTan),
                constants::one_rad * glm::atan(hmd_desc.DefaultEyeFov[eye].DownTan),
                constants::one_rad * glm::atan(hmd_desc.DefaultEyeFov[eye].LeftTan),
                constants::one_rad * glm::atan(hmd_desc.DefaultEyeFov[eye].RightTan));
        debug_msg("\t\tMax FOV : up(%.2f), down(%.2f), left(%.2f), right(%.2f)", 
                constants::one_rad * glm::atan(hmd_desc.MaxEyeFov[eye].UpTan),
                constants::one_rad * glm::atan(hmd_desc.MaxEyeFov[eye].DownTan),
                constants::one_rad * glm::atan(hmd_desc.MaxEyeFov[eye].LeftTan),
                constants::one_rad * glm::atan(hmd_desc.MaxEyeFov[eye].RightTan));
    }   

    debug_msg("\tResolution = %d x %d.", hmd_desc.Resolution.w, hmd_desc.Resolution.h);
    debug_msg("\tRefresh rate = %f cycles per second.", hmd_desc.DisplayRefreshRate);
}

ovr_hmd_t::~ovr_hmd_t()
{
    //===================================================================================================================================================================================================================
    // Destroy OVR session and shutdown the library                                                                                                                                                                      
    //===================================================================================================================================================================================================================
    ovr_Destroy(session);
    ovr_Shutdown();
}

