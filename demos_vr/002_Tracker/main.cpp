//========================================================================================================================================================================================================================
// OCULUS DEMO 002 : Rift head tracker
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS

#include <cstdio>
#include <OVR_CAPI.h>

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
    // Get the number of physical trackers and their description
    //===================================================================================================================================================================================================================
    unsigned int trackers = ovr_GetTrackerCount(session);
    for(unsigned int tracker = 0; tracker < trackers; ++tracker)
    {
        ovrTrackerDesc description = ovr_GetTrackerDesc(session, tracker);
        debug_msg("Tracker #%u :", tracker);
        debug_msg("\tHorizontal FOV :: %.2f", constants::one_rad * description.FrustumHFovInRadians);
        debug_msg("\tVertical FOV   :: %.2f", constants::one_rad * description.FrustumVFovInRadians);
        debug_msg("\tNear z-plane   :: %.2f m", description.FrustumNearZInMeters);
        debug_msg("\tFar z-plaane   :: %.2f m", description.FrustumFarZInMeters);

        ovrTrackerPose pose = ovr_GetTrackerPose(session, tracker);

        if (pose.TrackerFlags & ovrTracker_PoseTracked)
        {
            debug_msg("\t\tTracker position    : Vec3 {%.3f, %.3f, %.3f}", 
                        pose.Pose.Position.x, pose.Pose.Position.y, pose.Pose.Position.z);
            debug_msg("\t\tTracker orientation : Quat {%.3f, %.3f, %.3f, %.3f}", 
                        pose.Pose.Orientation.x, pose.Pose.Orientation.y, pose.Pose.Orientation.z, pose.Pose.Orientation.w);
        }

    }

    //===================================================================================================================================================================================================================
    // Query the HMD for ts current tracking state.
    //===================================================================================================================================================================================================================
    debug_msg("Reading sensor information ...");
    FILE* f = fopen("sensor_events.log", "w");
    
    for(int i = 0; i < 1024; ++i)
    {
        double timestamp = ovr_GetTimeInSeconds();
        fprintf(f, "Sensor read #%u : timestamp requested = %.5f\n", i, timestamp);
        ovrTrackingState state = ovr_GetTrackingState(session, timestamp, ovrTrue);

        if (state.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
        {
            ovrPoseStatef head_pose = state.HeadPose;

            ovrVector3f position = head_pose.ThePose.Position;
            ovrQuatf orientation = head_pose.ThePose.Orientation;

            fprintf(f, "\tHead position    : Vec3 {%.3f, %.3f, %.3f}\n",
                        position.x, position.y, position.z);
            fprintf(f, "\tHead orientation : Quat {%.3f, %.3f, %.3f, %.3f}\n",
                        orientation.x, orientation.y, orientation.z, orientation.w);

            ovrVector3f angular_velocity     = head_pose.AngularVelocity;
            ovrVector3f linear_velocity      = head_pose.LinearVelocity;
            ovrVector3f angular_acceleration = head_pose.AngularAcceleration;
            ovrVector3f linear_acceleration  = head_pose.LinearAcceleration;

            fprintf(f, "\t\tAngular Velocity : Vec3 {%.3f, %.3f, %.3f}\n",
                        angular_velocity.x, angular_velocity.y, angular_velocity.z);
            fprintf(f, "\t\tLinear Velocity  : Vec3 {%.3f, %.3f, %.3f}\n",
                        linear_velocity.x, linear_velocity.y, linear_velocity.z);
            fprintf(f, "\t\t\tAngular Acceleration : Vec3 {%.3f, %.3f, %.3f}\n",
                        angular_acceleration.x, angular_acceleration.y, angular_acceleration.z);
            fprintf(f, "\t\t\tLinear Acceleration  : Vec3 {%.3f, %.3f, %.3f}\n",
                        linear_acceleration.x, linear_acceleration.y, linear_acceleration.z);

            double time_stamp = state.HeadPose.TimeInSeconds;

            // CalibratedOrigin
            ovrQuatf origin_orientation = state.CalibratedOrigin.Orientation;
            ovrVector3f origin_position = state.CalibratedOrigin.Position;
        }
        else
            fprintf(f, "\tSkipping, as the position and orientation are not tracked.\n");

        fprintf(f, "\n");
    }    

    fclose(f);

    //===================================================================================================================================================================================================================
    // Shutdown the library
    //===================================================================================================================================================================================================================
    ovr_Destroy(session);
    ovr_Shutdown();
    debug_msg("Exiting ... ");
    return 0;
}