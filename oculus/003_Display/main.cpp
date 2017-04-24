//========================================================================================================================================================================================================================
// OCTULUS DEMO 003 : Rift display
//========================================================================================================================================================================================================================
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#define GLEW_STATIC
#include <GL/glew.h>                                                                                                        // OpenGL extensions

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "log.hpp"
#include "constants.hpp"

void ovr_hmd_info(const ovrHmdDesc& hmdDesc);

int main(int argc, char** argv)
{
    ovrResult result;                                                   // result < 0 indicates failure and its value is the error code

    //===================================================================================================================================================================================================================
    // Initialize the library
    //===================================================================================================================================================================================================================
    debug_msg("Initializing Octulus Library ...");
    result = ovr_Initialize(0);
    if(result < 0)
    {
        ovrErrorInfo errorInfo;
        ovr_GetLastErrorInfo(&errorInfo);
        exit_msg("ovr_Initialize failed : code = %d. %s", result, errorInfo.ErrorString);
    }

    //===================================================================================================================================================================================================================
    // Create session = get access to the device
    //===================================================================================================================================================================================================================
    debug_msg("Creating OVR session ...");
    ovrSession session;
    ovrGraphicsLuid luid;
    result = ovr_Create(&session, &luid);
    if(result < 0)
    {
        ovrErrorInfo errorInfo;
        ovr_GetLastErrorInfo(&errorInfo);
        exit_msg("ovr_Create failed : code = %d. %s", result, errorInfo.ErrorString);
    }

    //===================================================================================================================================================================================================================
    // Get and log device description
    //===================================================================================================================================================================================================================
    debug_msg("Requesting HMD description ...");
    ovrHmdDesc hmd_desc = ovr_GetHmdDesc(session);
    ovr_hmd_info(hmd_desc);

    //===================================================================================================================================================================================================================
    // Read sensor information
    //===================================================================================================================================================================================================================

//    struct ovrTrackingState
//    {
//        // Predicted head pose (and derivatives) at the requested absolute time.
//        ovrPoseStatef  HeadPose;
//
//        // HeadPose tracking status described by ovrStatusBits.
//        unsigned int   StatusFlags;
//
//        // The most recent calculated pose for each hand when hand controller tracking is present. HandPoses[ovrHand_Left] refers to the left hand and HandPoses[ovrHand_Right] to the right hand.
//        // These values can be combined with ovrInputState for complete hand controller information.
//        ovrPoseStatef  HandPoses[2];
//
//        /// HandPoses status flags described by ovrStatusBits. Only ovrStatus_OrientationTracked and ovrStatus_PositionTracked are reported.
//        unsigned int   HandStatusFlags[2];
//
//        // The pose of the origin captured during calibration. Like all other poses here, this is expressed in the space set by ovr_RecenterTrackingOrigin, and so will change every time that is called. 
//        // This pose can be used to calculate where the calibrated origin lands in the new recentered space. If an application never calls ovr_RecenterTrackingOrigin, expect this value to be the identity
//        // pose and as such will point respective origin based on ovrTrackingOrigin requested when calling ovr_GetTrackingState.
//        ovrPosef      CalibratedOrigin;
//
//    };
//
//    struct ovrPoseStatef
//    {
//        ovrPosef     ThePose;               // Position and orientation.
//        ovrVector3f  AngularVelocity;       // Angular velocity in radians per second.
//        ovrVector3f  LinearVelocity;        // Velocity in meters per second.
//        ovrVector3f  AngularAcceleration;   // Angular acceleration in radians per second per second.
//        ovrVector3f  LinearAcceleration;    // Acceleration in meters per second per second.
//        double       TimeInSeconds;         // Absolute time that this pose refers to. \see ovr_GetTimeInSeconds
//    };
//
//    struct ovrPosef
//    {
//        ovrQuatf     Orientation;
//        ovrVector3f  Position;
//    };


    // Query the HMD for ts current tracking state.
    debug_msg("Reading sensor information ...");
    ovrTrackingState state = ovr_GetTrackingState(session, ovr_GetTimeInSeconds(), ovrTrue);

    if (state.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
    {
        // ovrPoseStatef
        ovrQuatf orientation = state.HeadPose.ThePose.Orientation;
        ovrVector3f position = state.HeadPose.ThePose.Position;

        ovrVector3f angular_velocity     = state.HeadPose.AngularVelocity;    
        ovrVector3f linear_velocity      = state.HeadPose.LinearVelocity;     
        ovrVector3f angular_acceleration = state.HeadPose.AngularAcceleration;
        ovrVector3f linear_acceleration  = state.HeadPose.LinearAcceleration; 

        double time_stamp = state.HeadPose.TimeInSeconds;

        // CalibratedOrigin
        ovrQuatf origin_orientation = state.CalibratedOrigin.Orientation;
        ovrVector3f origin_position = state.CalibratedOrigin.Position;
    }

    //===================================================================================================================================================================================================================
    // Configure Stereo settings and create texture swap chain
    //===================================================================================================================================================================================================================

    ovrSizei texture_size_l = ovr_GetFovTextureSize(session, ovrEye_Left,  hmd_desc.DefaultEyeFov[ovrEye_Left],  1.0f);
    ovrSizei texture_size_r = ovr_GetFovTextureSize(session, ovrEye_Right, hmd_desc.DefaultEyeFov[ovrEye_Right], 1.0f);

    ovrTextureSwapChain textureSwapChain = 0;
    ovrTextureSwapChainDesc desc;
    desc.Type = ovrTexture_2D;
    desc.ArraySize = 1;
    desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.Width = texture_size_l.w + texture_size_r.w;
    desc.Height = (texture_size_l.h < texture_size_r.h) ? texture_size_r.h : texture_size_l.h;
    desc.MipLevels = 1;
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;
    if (ovr_CreateTextureSwapChainGL(session, &desc, &textureSwapChain) == ovrSuccess)
    {
        // Sample texture access:
        GLuint texture_id;
        ovr_GetTextureSwapChainBufferGL(session, textureSwapChain, 0, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
    }

    //===================================================================================================================================================================================================================
    // Shutdown the library
    //===================================================================================================================================================================================================================
    ovr_Shutdown();
    debug_msg("Exiting ... ");
    return 0;
}
    

void ovr_hmd_info(const ovrHmdDesc& hmd_desc)
{
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

    debug_msg("\tVR Device information : type = %s (value = %d).", type_str, hmd_desc.Type);
    debug_msg("\tProduct Name = %s.", hmd_desc.ProductName);
    debug_msg("\tManufacturer = %s.", hmd_desc.Manufacturer);
    debug_msg("\tVendorId = %d.", hmd_desc.VendorId);
    debug_msg("\tProductId = %d.", hmd_desc.ProductId);
    debug_msg("\tSerialNumber = %.24s.", hmd_desc.SerialNumber);
    debug_msg("\tFirmware = %d.%d", hmd_desc.FirmwareMajor, hmd_desc.FirmwareMinor);

    debug_msg("\tAvailableHmdCaps = %x", hmd_desc.AvailableHmdCaps);
    debug_msg("\tDefaultHmdCaps = %x", hmd_desc.DefaultHmdCaps);
    debug_msg("\tAvailableTrackingCaps = %x", hmd_desc.AvailableTrackingCaps);
    debug_msg("\tDefaultTrackingCaps = %x", hmd_desc.DefaultTrackingCaps);


    for (int i = 0; i < ovrEye_Count; ++i)
    {
        debug_msg("\tLenses info : lens #%d :", i);
        debug_msg("\t\tDefault FOV : up(%f), down(%f), left(%f), right(%f)", 
                        constants::one_rad * glm::atan(hmd_desc.DefaultEyeFov[i].UpTan),
                        constants::one_rad * glm::atan(hmd_desc.DefaultEyeFov[i].DownTan),
                        constants::one_rad * glm::atan(hmd_desc.DefaultEyeFov[i].LeftTan),
                        constants::one_rad * glm::atan(hmd_desc.DefaultEyeFov[i].RightTan));
        debug_msg("\t\tMax FOV : up(%f), down(%f), left(%f), right(%f)", 
                        constants::one_rad * glm::atan(hmd_desc.MaxEyeFov[i].UpTan),
                        constants::one_rad * glm::atan(hmd_desc.MaxEyeFov[i].DownTan),
                        constants::one_rad * glm::atan(hmd_desc.MaxEyeFov[i].LeftTan),
                        constants::one_rad * glm::atan(hmd_desc.MaxEyeFov[i].RightTan));
    }

    debug_msg("\tResolution = %dx%d.", hmd_desc.Resolution.w, hmd_desc.Resolution.h);
    debug_msg("\tRefresh rate = %f.", hmd_desc.DisplayRefreshRate);
}