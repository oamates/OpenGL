//========================================================================================================================================================================================================================
// OCULUS DEMO 001 : Device initialization
//========================================================================================================================================================================================================================
#include <OVR_CAPI.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "log.hpp"
#include "constants.hpp"

void ovr_hmd_info(const ovrHmdDesc& hmd_desc)
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

    debug_msg("Done");
    debug_msg("LibOVRRT version : %s", ovr_GetVersionString());

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
    // Shutdown the library
    //===================================================================================================================================================================================================================
    ovr_Destroy(session);
    ovr_Shutdown();
    debug_msg("Exiting ... ");
    return 0;
}