//========================================================================================================================================================================================================================
// OCULUS DEMO 005 : Attractor
//========================================================================================================================================================================================================================

#include <iostream>
#include <memory>
#include <algorithm>

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
#include "log.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "plato.hpp"
#include "polyhedron.hpp"
#include "surface.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    { 
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
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


inline glm::mat4 toGlm(const ovrMatrix4f& ovr_matrix)
    { return glm::transpose(glm::make_mat4(&ovr_matrix.M[0][0])); }

inline glm::mat4 toGlm(const ovrFovPort& fovport, float nearPlane = 0.01f, float farPlane = 10000.0f)
    { return toGlm(ovrMatrix4f_Projection(fovport, nearPlane, farPlane, true)); }

inline glm::vec3 toGlm(const ovrVector3f& ovr_vector)
    { return glm::make_vec3(&ovr_vector.x); }

inline glm::vec2 toGlm(const ovrVector2f& ovr_vector)
    { return glm::make_vec2(&ovr_vector.x); }

inline glm::uvec2 toGlm(const ovrSizei& ovr_vector)
    { return glm::uvec2(ovr_vector.w, ovr_vector.h); }

inline glm::quat toGlm(const ovrQuatf& ovr_quat)
    { return glm::make_quat(&ovr_quat.x); }

inline glm::mat4 toGlm(const ovrPosef& ovr_pose)
{
    glm::mat4 orientation = glm::mat4_cast(toGlm(ovr_pose.Orientation));
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), toGlm(ovr_pose.Position));
    return translation * orientation;
}

inline ovrMatrix4f fromGlm(const glm::mat4& m)
{
    ovrMatrix4f result;
    glm::mat4 transposed(glm::transpose(m));
    memcpy(result.M, &(transposed[0][0]), sizeof(float) * 16);
    return result;
}

inline ovrVector3f fromGlm(const glm::vec3& v)
{
    ovrVector3f result;
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;
    return result;
}

inline ovrVector2f fromGlm(const glm::vec2& v)
{
    ovrVector2f result;
    result.x = v.x;
    result.y = v.y;
    return result;
}

inline ovrSizei fromGlm(const glm::uvec2 & v)
{
    ovrSizei result;
    result.w = v.x;
    result.h = v.y;
    return result;
}

inline ovrQuatf fromGlm(const glm::quat & q)
{
    ovrQuatf result;
    result.x = q.x;
    result.y = q.y;
    result.z = q.z;
    result.w = q.w;
    return result;
}

//=======================================================================================================================================================================================================================
// static variables
//=======================================================================================================================================================================================================================

GLuint _fbo = 0;
GLuint _depthBuffer = 0;

GLuint _mirrorFbo = 0;

ovrEyeRenderDesc _eyeRenderDescs[2];
glm::mat4 _eyeProjections[2];

bool check_fbo_status(GLenum target)
{
    GLuint status = glCheckFramebufferStatus(target);
    if (GL_FRAMEBUFFER_COMPLETE == status) return true;
    const char* error_desc;
    switch (status)
    {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         error_desc = "Framebuffer incomplete attachment."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: error_desc = "Framebuffer missing attachment."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        error_desc = "Framebuffer incomplete draw buffer."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        error_desc = "Framebuffer incomplete read buffer."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        error_desc = "Framebuffer incomplete multisample."; break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      error_desc = "Framebuffer incomplete layer targets."; break;
        case GL_FRAMEBUFFER_UNSUPPORTED:                   error_desc = "Framebuffer unsupported internal format or image."; break;
        default:                                           error_desc = "Unknown framebuffer error";
    }
    return false;
}

struct motion3d_t
{
	glm::vec4 shift;
	glm::vec4 rotor;
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
	}
}

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

struct ovr_hmd_t
{
    ovrSession session;
    ovrGraphicsLuid luid;
    ovrHmdDesc hmd_desc;
    ovrViewScaleDesc _viewScaleDesc;
    ovrLayerEyeFov _sceneLayer;
    ovrTextureSwapChain _eyeTexture;
    ovrMirrorTexture _mirrorTexture;

    int length;  //ovr_GetTextureSwapChainLength

    glm::ivec2 _renderTargetSize;
    glm::ivec2 _mirrorSize;

    ovr_hmd_t()
        : _renderTargetSize(0)
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
        // Fill in necessary structures
        //===================================================================================================================================================================================================================

        _viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

        memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
        _sceneLayer.Header.Type = ovrLayerType_EyeFov;
        _sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

        for (ovrEyeType eye = ovrEyeType::ovrEye_Left; eye < ovrEyeType::ovrEye_Count; eye = static_cast<ovrEyeType>(eye + 1))
        {

            ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(session, eye, hmd_desc.DefaultEyeFov[eye]);
            ovrMatrix4f ovrPerspectiveProjection = ovrMatrix4f_Projection(erd.Fov, 0.5f, 100.0f, ovrProjection_ClipRangeOpenGL);
            _eyeProjections[eye] = toGlm(ovrPerspectiveProjection);
        
            _viewScaleDesc.HmdToEyeOffset[eye] = erd.HmdToEyeOffset;
        
            ovrFovPort & fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
            ovrSizei eyeSize = ovr_GetFovTextureSize(session, eye, fov, 1.25f);
            _sceneLayer.Viewport[eye].Size = eyeSize;
            _sceneLayer.Viewport[eye].Pos = { _renderTargetSize.x, 0 };

            _renderTargetSize.y = std::max(_renderTargetSize.y, eyeSize.h);
            _renderTargetSize.x += eyeSize.w;
        }

        _mirrorSize = _renderTargetSize;
        _mirrorSize /= 2;        

        debug_msg("_renderTargetSize  = %d x %d.", _renderTargetSize.x, _renderTargetSize.y);
        debug_msg("_mirrorTextureSize = %d x %d.", _mirrorSize.x, _mirrorSize.y);
    }    

    void create_swap_chain()
    {

        ovrTextureSwapChainDesc desc = 
        {
            .Type = ovrTexture_2D,
            .Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB,
            .ArraySize = 1,
            .Width = _renderTargetSize.x,
            .Height = _renderTargetSize.y,
            .MipLevels = 1,
            .SampleCount = 1,
            .StaticImage = ovrFalse,
            .MiscFlags = 0,
            .BindFlags = 0
        };

        ovrResult result = ovr_CreateTextureSwapChainGL(session, &desc, &_eyeTexture);
        _sceneLayer.ColorTexture[0] = _eyeTexture;
        if (result < 0)
            exit_msg("Failed to create swap textures");

        result = ovr_GetTextureSwapChainLength(session, _eyeTexture, &length);
        if (result < 0)
            exit_msg("Unable to count swap chain textures");

        //===============================================================================================================================================================================================================
        // create OVR texture swapchain
        //===============================================================================================================================================================================================================
        debug_msg("Swapchain length :: %u", length);
        for (int i = 0; i < length; ++i)
        {
            GLuint chainTexId;
            ovr_GetTextureSwapChainBufferGL(session, _eyeTexture, i, &chainTexId);
            glBindTexture(GL_TEXTURE_2D, chainTexId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void create_mirror_texture()
    {
        ovrMirrorTextureDesc mirrorDesc = 
        {
            .Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB,
            .Width = _mirrorSize.x,
            .Height = _mirrorSize.y,
            .MiscFlags = 0
        };

        if (ovr_CreateMirrorTextureGL(session, &mirrorDesc, &_mirrorTexture) < 0)
            exit_msg("Could not create mirror texture");
    }

    GLuint mirror_texture_id()
    {
        GLuint texture_id;
        ovr_GetMirrorTextureBufferGL(session, _mirrorTexture, &texture_id);
        return texture_id;
    }

    GLuint swapchain_texture_id()
    {
        int index;
        ovr_GetTextureSwapChainCurrentIndex(session, _eyeTexture, &index);
        GLuint texture_id;
        ovr_GetTextureSwapChainBufferGL(session, _eyeTexture, index, &texture_id);
        return texture_id;
    }

    void submit_frame(int frame)
    {
        ovr_CommitTextureSwapChain(session, _eyeTexture);
        ovrLayerHeader* layer_header = &_sceneLayer.Header;
        ovr_SubmitFrame(session, frame, &_viewScaleDesc, &layer_header, 1);
    }

    void dump_info()
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


    ~ovr_hmd_t()
    {
        //===============================================================================================================================================================================================================
        // Destroy OVR session and shutdown the library
        //===============================================================================================================================================================================================================
        ovr_Destroy(session);
        ovr_Shutdown();
    }

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

    demo_window_t window("Oculus Rift Attractor", 4, 3, 3, ovr_hmd._mirrorSize.x, ovr_hmd._mirrorSize.y, false);

    //===================================================================================================================================================================================================================
    // Create texture swapchain and mirror texture for displaying in the app window
    //===================================================================================================================================================================================================================
    ovr_hmd.create_swap_chain();
    ovr_hmd.create_mirror_texture();

    //===================================================================================================================================================================================================================
    // Set up the framebuffer objects : one for oculus usage and one for screen blitting
    //===================================================================================================================================================================================================================
    glGenFramebuffers(1, &_fbo);
    glGenRenderbuffers(1, &_depthBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, ovr_hmd._renderTargetSize.x, ovr_hmd._renderTargetSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &_mirrorFbo);

    ovr_RecenterTrackingOrigin(ovr_hmd.session);

	//===================================================================================================================================================================================================================
	// create programs : one for particle compute, the other for render
	//===================================================================================================================================================================================================================
    glsl_program_t particle_compute(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/particle.cs"));
    uniform_t uniform_dt = particle_compute["dt"];
    GLint uniform_time = particle_compute["time"];

    glsl_program_t particle_render(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/particle_render.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/particle_render.fs"));
    uniform_t uniform_projection_view_matrix = particle_render["projection_view_matrix"];

	//===================================================================================================================================================================================================================
	// point data initialization 
	//===================================================================================================================================================================================================================
    const int PARTICLE_GROUP_SIZE  = 128;
    const int PARTICLE_GROUP_COUNT = 65536 / 2;
    const int PARTICLE_COUNT       = PARTICLE_GROUP_SIZE * PARTICLE_GROUP_COUNT;
    const int ATTRACTOR_COUNT      = 2;

    float attractor_masses[ATTRACTOR_COUNT];

    GLuint vao_id, position_buffer, velocity_buffer, attractor_buffer;

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glGenBuffers(1, &position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
    glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);

    glm::vec4* positions = (glm::vec4*) glMapBufferRange(GL_ARRAY_BUFFER, 0, PARTICLE_COUNT * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    for (int i = 0; i < PARTICLE_COUNT; i++)
        positions[i] = glm::vec4(glm::ballRand(100.0f), glm::gaussRand(0.0f, 1.0f));

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &velocity_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, velocity_buffer);
    glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);

    glm::vec4* velocities = (glm::vec4*) glMapBufferRange(GL_ARRAY_BUFFER, 0, PARTICLE_COUNT * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    for (int i = 0; i < PARTICLE_COUNT; i++)
        velocities[i] = glm::vec4(glm::gaussRand(0.0f, 10.0f) * glm::sphericalRand(1.0f), 0.0f);

    glUnmapBuffer(GL_ARRAY_BUFFER);

    GLuint position_tbo, velocity_tbo;
    glGenTextures(1, &position_tbo);
    glBindTexture(GL_TEXTURE_BUFFER, position_tbo);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, position_buffer);

    glGenTextures(1, &velocity_tbo);
    glBindTexture(GL_TEXTURE_BUFFER, velocity_tbo);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, velocity_buffer);

    glGenBuffers(1, &attractor_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, attractor_buffer);
    glBufferData(GL_UNIFORM_BUFFER, ATTRACTOR_COUNT * sizeof(glm::vec4), 0, GL_STATIC_DRAW);

    for (int i = 0; i < ATTRACTOR_COUNT; i++)
        attractor_masses[i] = 1.5f + glm::abs(glm::gaussRand(0.0f, 12.0f));

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, attractor_buffer);
    glBindImageTexture(0, velocity_tbo, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(1, position_tbo, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : 
    // * background color -- dark blue
    // * DEPTH_TEST disabled
    // * Blending enabled
    //===================================================================================================================================================================================================================
	glClearColor(0.01f, 0.0f, 0.05f, 0.0f);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    //===================================================================================================================================================================================================================
    // Main rendering loop
    //===================================================================================================================================================================================================================
    while (!window.should_close())
    {
        window.new_frame();

        //===============================================================================================================================================================================================================
        // update eyes position
        //===============================================================================================================================================================================================================
        ovrPosef eyePoses[2];
        ovr_GetEyePoses(ovr_hmd.session, window.frame, true, ovr_hmd._viewScaleDesc.HmdToEyeOffset, eyePoses, &ovr_hmd._sceneLayer.SensorSampleTime);

        //===============================================================================================================================================================================================================
        // update particle positions
        //===============================================================================================================================================================================================================
        float time = window.frame_ts;
    	float dt = glm::min(25.0 * window.frame_dt, 2.0);

	    glm::vec4* attractors = (glm::vec4*) glMapBufferRange(GL_UNIFORM_BUFFER, 0, ATTRACTOR_COUNT * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (int i = 0; i < ATTRACTOR_COUNT; i++)
    	{
	        attractors[i] = glm::vec4(glm::sin(time * (i + 4) * 0.05f + 1.44f) * 25.0f,
            	                      glm::sin(time * (i + 7) * 0.09f + 1.44f) * 25.0f,
                                      glm::sin(time * (i + 3) * 0.03f + 1.44f) * 25.0f,
        	                          attractor_masses[i]);
    	}

	    glUnmapBuffer(GL_UNIFORM_BUFFER);
    	particle_compute.enable();
	    uniform_dt = dt;
        uniform_time = time;
    	glDispatchCompute(PARTICLE_GROUP_COUNT, 1, 1);

        //===============================================================================================================================================================================================================
        // render the scene for both eyes
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ovr_hmd.swapchain_texture_id(), 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        for (ovrEyeType eye = ovrEyeType::ovrEye_Left; eye < ovrEyeType::ovrEye_Count; eye = static_cast<ovrEyeType>(eye + 1))
        {
            const ovrRecti& vp = ovr_hmd._sceneLayer.Viewport[eye];
            glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
            ovr_hmd._sceneLayer.RenderPose[eye] = eyePoses[eye];

			glm::mat4 projection_matrix = _eyeProjections[eye];
			glm::mat4 camera_ws1 = window.camera.view_matrix;
			glm::mat4 view_matrix = toGlm(eyePoses[eye]) * camera_ws1;
            glm::mat4 projection_view_matrix = projection_matrix * view_matrix;

    	    particle_render.enable();
        	uniform_projection_view_matrix = projection_view_matrix;
    	    glBindVertexArray(vao_id);
 	        glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);
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
        glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ovr_hmd.mirror_texture_id(), 0);
        glBlitFramebuffer(0, 0, ovr_hmd._mirrorSize.x, ovr_hmd._mirrorSize.y, 0, ovr_hmd._mirrorSize.y, ovr_hmd._mirrorSize.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // Destroy GLFW window and terminate the library
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}