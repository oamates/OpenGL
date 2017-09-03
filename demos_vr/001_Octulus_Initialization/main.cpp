//========================================================================================================================================================================================================================
// OCTULUS DEMO 001 : Initialization
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS

#include <OVR_CAPI.h>

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