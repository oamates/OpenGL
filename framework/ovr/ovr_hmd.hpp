#ifndef _ovr_hmd_included_98625896387562834756812037561820375681237468120374678
#define _ovr_hmd_included_98625896387562834756812037561820375681237468120374678

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#define GLEW_STATIC
#include <GL/glew.h> 														                                                // OpenGL extensions

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"

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

    //===================================================================================================================================================================================================================
    // public methods
    //===================================================================================================================================================================================================================
    ovr_hmd_t();
    void create_swap_chain();

    void create_mirror_texture();
    void update_tracking(int frame);

    void set_viewport(ovrEyeType eye);
    void set_indexed_viewport(ovrEyeType eye, GLuint index);

    GLuint mirror_texture_id();
    GLuint swapchain_texture_id();
    void submit_frame(int frame);
    void dump_info();

    ~ovr_hmd_t();

};

#endif // _ovr_hmd_included_98625896387562834756812037561820375681237468120374678