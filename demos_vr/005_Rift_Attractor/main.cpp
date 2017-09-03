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
#include "shader.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "log.hpp"
#include "camera3d.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "plato.hpp"
#include "polyhedron.hpp"
#include "surface.hpp"

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

ovrSession session;
ovrGraphicsLuid luid;
ovrHmdDesc hmd_desc;
glm::uvec2 windowSize;
glm::ivec2 windowPosition;
GLFWwindow* window = 0;
unsigned int frame = 0;

GLuint _fbo = 0;
GLuint _depthBuffer = 0;
ovrTextureSwapChain _eyeTexture;

GLuint _mirrorFbo = 0;
ovrMirrorTexture _mirrorTexture;

ovrEyeRenderDesc _eyeRenderDescs[2];
glm::mat4 _eyeProjections[2];

ovrLayerEyeFov _sceneLayer;
ovrViewScaleDesc _viewScaleDesc;

glm::uvec2 _renderTargetSize;
glm::uvec2 _mirrorSize;

//=======================================================================================================================================================================================================================
// Auxiliary error logging functions
//=======================================================================================================================================================================================================================
#if defined(GLAPIENTRY)
    #define GL_CALLBACK GLAPIENTRY
#elif defined(APIENTRY)
    #define GL_CALLBACK APIENTRY
#else
    #define GL_CALLBACK 
#endif

void GL_CALLBACK gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
void ovr_hmd_info(const ovrHmdDesc& hmd_desc);
bool check_fbo_status(GLenum target = GL_FRAMEBUFFER);
bool gl_check_error();
void glfw_error_callback(int error, const char* description);

//========================================================================================================================================================================================================================
// 3d moving camera : standard initial orientation in space
//========================================================================================================================================================================================================================
const float linear_velocity = 0.7f;
const float angular_rate = 0.0001f;
static camera3d camera;
double mouse_x = 0.0, mouse_y = 0.0;
double mouse_event_ts;

//=======================================================================================================================================================================================================================
// Keyboard and mouse callback function
//=======================================================================================================================================================================================================================
void onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if ((key == GLFW_KEY_UP) || (key == GLFW_KEY_W)) camera.move_forward(linear_velocity);
    else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(linear_velocity);
    else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(linear_velocity);
    else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(linear_velocity);

    if (GLFW_RELEASE != action) return;
    if (GLFW_KEY_ESCAPE == key) glfwSetWindowShouldClose(window, 1);
    if (GLFW_KEY_R == key) ovr_RecenterTrackingOrigin(session);

};

void onMouseMove (GLFWwindow*, double x, double y)
{
	double dx = mouse_x - x; mouse_x = x;
	double dy = mouse_y - y; mouse_y = y;
	double ts = glfwGetTime();
	double duration = ts - mouse_event_ts;
	mouse_event_ts = ts;  
    duration = glm::max(duration, 0.01);    
    double norm = sqrt(dx * dx + dy * dy);
    if (norm > 0.01f)
    {
        dx /= norm; dy /= norm;
        double angle = angular_rate * sqrt(norm) / (duration + 0.01);
        camera.rotateXY(dx, -dy, angle);
    };
};



struct motion3d_t
{
	glm::vec4 shift;
	glm::vec4 rotor;
};

vertex_pft2 torus_func(const glm::vec2& uv)
{
    vertex_pft2 vertex;
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
};

struct room
{
    GLuint vao_id, vbo_id;

    room(float size)
    {
        vertex_pnt2 vertices[36];

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
            vertices[index++] = vertex_pnt2(size * plato::cube::vertices[A], normal, unit_square[0]);
            vertices[index++] = vertex_pnt2(size * plato::cube::vertices[B], normal, unit_square[1]);
            vertices[index++] = vertex_pnt2(size * plato::cube::vertices[C], normal, unit_square[2]);
            vertices[index++] = vertex_pnt2(size * plato::cube::vertices[A], normal, unit_square[0]);
            vertices[index++] = vertex_pnt2(size * plato::cube::vertices[C], normal, unit_square[2]);
            vertices[index++] = vertex_pnt2(size * plato::cube::vertices[D], normal, unit_square[3]);
        };

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        vbo_id = generate_attribute_buffer(&vertices[0], 36);
    };

    void render()
    {
        glBindVertexArray(vao_id);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    };

    ~room()
    {
        glDeleteBuffers(1, &vbo_id);
        glDeleteVertexArrays(1, &vbo_id);
    };
};




//=======================================================================================================================================================================================================================
//  Program entry point
//=======================================================================================================================================================================================================================
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

    if (!glfwInit())
        exit_msg("Failed to initialize GLFW");
    glfwSetErrorCallback(glfw_error_callback);

    _viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

    memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
    _sceneLayer.Header.Type = ovrLayerType_EyeFov;
    _sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

    for (ovrEyeType eye = ovrEyeType::ovrEye_Left; eye < ovrEyeType::ovrEye_Count; eye = static_cast<ovrEyeType>(eye + 1))
    {

        ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(session, eye, hmd_desc.DefaultEyeFov[eye]);
        ovrMatrix4f ovrPerspectiveProjection = ovrMatrix4f_Projection(erd.Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
        _eyeProjections[eye] = toGlm(ovrPerspectiveProjection);
        _viewScaleDesc.HmdToEyeOffset[eye] = erd.HmdToEyeOffset;

        ovrFovPort & fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
        auto eyeSize = ovr_GetFovTextureSize(session, eye, fov, 1.25f);
        _sceneLayer.Viewport[eye].Size = eyeSize;
        _sceneLayer.Viewport[eye].Pos = { _renderTargetSize.x, 0 };

        _renderTargetSize.y = std::max(_renderTargetSize.y, (uint32_t)eyeSize.h);
        _renderTargetSize.x += eyeSize.w;
    }

    // Make the on screen window 1/4 the resolution of the render target
    _mirrorSize = _renderTargetSize;
    _mirrorSize /= 2;

    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    window = glfwCreateWindow(_mirrorSize.x, _mirrorSize.y, "Hello Rift", 0, 0);

    if (!window)
        exit_msg("Unable to create OpenGL window");

    glfwSetKeyCallback(window, onKey);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, 0.0, 0.0);
	glfwSetCursorPosCallback(window, onMouseMove);

    glfwMakeContextCurrent(window);

	mouse_event_ts = glfwGetTime();

    // Initialize the OpenGL bindings. For some reason we have to set this experminetal flag to properly init GLEW if we use a core context.
    glewExperimental = GL_TRUE;
    if (0 != glewInit())
        exit_msg("Failed to initialize GLEW");

    glGetError();

    if (GLEW_KHR_debug)
    {
        GLint v;
        glGetIntegerv(GL_CONTEXT_FLAGS, &v);
        if (v & GL_CONTEXT_FLAG_DEBUG_BIT) glDebugMessageCallback(gl_debug_callback, 0);
    }

    // Disable the v-sync for buffer swap
    glfwSwapInterval(0);

    debug_msg("GL_VENDOR = %s.", glGetString(GL_VENDOR));                                       
    debug_msg("GL_RENDERER = %s.", glGetString(GL_RENDERER));                                   
    debug_msg("GL_VERSION = %s.", glGetString(GL_VERSION));                                     
    debug_msg("GL_SHADING_LANGUAGE_VERSION = %s.", glGetString(GL_SHADING_LANGUAGE_VERSION));   
    debug_msg("GL_EXTENSIONS = %s.", glGetString(GL_EXTENSIONS));

    ovrTextureSwapChainDesc desc = {};
    desc.Type = ovrTexture_2D;
    desc.ArraySize = 1;
    desc.Width = _renderTargetSize.x;
    desc.Height = _renderTargetSize.y;

	debug_msg("_renderTargetSize = %d x %d.", _renderTargetSize.x, _renderTargetSize.y);
    desc.MipLevels = 1;
    desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;
    result = ovr_CreateTextureSwapChainGL(session, &desc, &_eyeTexture);
    _sceneLayer.ColorTexture[0] = _eyeTexture;
    if (result < 0)
        debug_msg("Failed to create swap textures");

    int length = 0;
    result = ovr_GetTextureSwapChainLength(session, _eyeTexture, &length);
    if ((result < 0) || (0 == length))
        exit_msg("Unable to count swap chain textures");

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

    // Set up the framebuffer object
    glGenFramebuffers(1, &_fbo);
    glGenRenderbuffers(1, &_depthBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _renderTargetSize.x, _renderTargetSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    ovrMirrorTextureDesc mirrorDesc;
    memset(&mirrorDesc, 0, sizeof(mirrorDesc));
    mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    mirrorDesc.Width = _mirrorSize.x;
    mirrorDesc.Height = _mirrorSize.y;
    if (ovr_CreateMirrorTextureGL(session, &mirrorDesc, &_mirrorTexture) < 0)
        exit_msg("Could not create mirror texture");
    glGenFramebuffers(1, &_mirrorFbo);

    ovr_RecenterTrackingOrigin(session);

	//===================================================================================================================================================================================================================
	// create programs : one for particle compute, the other for render
	//===================================================================================================================================================================================================================
    glsl_program particle_compute(glsl_shader(GL_COMPUTE_SHADER, "glsl/particle.cs"));
    GLint uniform_dt = particle_compute.uniform_id("dt");
    GLint uniform_time = particle_compute.uniform_id("time");

    glsl_program particle_render(glsl_shader(GL_VERTEX_SHADER,   "glsl/particle_render.vs"),
                                 glsl_shader(GL_FRAGMENT_SHADER, "glsl/particle_render.fs"));
    GLint uniform_projection_view_matrix = particle_render.uniform_id("projection_view_matrix");

	//===================================================================================================================================================================================================================
	// Point data initialization 
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

    double last_time = glfwGetTime();

    //===================================================================================================================================================================================================================
    // Main rendering loop
    //===================================================================================================================================================================================================================

    while (!glfwWindowShouldClose(window))
    {
        ++frame;
        glfwPollEvents();

        //===============================================================================================================================================================================================================
        // Render the scene
        //===============================================================================================================================================================================================================
        ovrPosef eyePoses[2];
        ovr_GetEyePoses(session, frame, true, _viewScaleDesc.HmdToEyeOffset, eyePoses, &_sceneLayer.SensorSampleTime);

        int curIndex;
        ovr_GetTextureSwapChainCurrentIndex(session, _eyeTexture, &curIndex);
        GLuint curTexId;
        ovr_GetTextureSwapChainBufferGL(session, _eyeTexture, curIndex, &curTexId);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);



        double current_time = glfwGetTime();
    	float dt = (current_time - last_time) * 25.0f;
	    glm::vec4* attractors = (glm::vec4* ) glMapBufferRange(GL_UNIFORM_BUFFER, 0, ATTRACTOR_COUNT * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (int i = 0; i < ATTRACTOR_COUNT; i++)
    	{
	        attractors[i] = glm::vec4(glm::sin(current_time * (i + 4) * 0.05f + 1.44f) * 25.0f,
            	                      glm::sin(current_time * (i + 7) * 0.09f + 1.44f) * 25.0f,
                                      glm::sin(current_time * (i + 3) * 0.03f + 1.44f) * 25.0f,
        	                          attractor_masses[i]);
    	};

	    glUnmapBuffer(GL_UNIFORM_BUFFER);
        if (dt >= 2.0f) dt = 2.0f;
    	particle_compute.enable();
	    glUniform1f(uniform_dt, dt);
        glUniform1f(uniform_time, current_time);
    	glDispatchCompute(PARTICLE_GROUP_COUNT, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        for (ovrEyeType eye = ovrEyeType::ovrEye_Left; eye < ovrEyeType::ovrEye_Count; eye = static_cast<ovrEyeType>(eye + 1))
        {
            const auto& vp = _sceneLayer.Viewport[eye];
            glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
            _sceneLayer.RenderPose[eye] = eyePoses[eye];

			glm::mat4 projection_matrix = _eyeProjections[eye];
			glm::mat4 camera_ws1 = camera.view_matrix;
			glm::mat4 view_matrix = toGlm(eyePoses[eye]) * camera_ws1;
            glm::vec4 camera_ws = glm::vec4(-glm::vec3(view_matrix[3]), 1.0f);
            glm::mat4 projection_view_matrix = projection_matrix * view_matrix;

    	    particle_render.enable();
        	glUniformMatrix4fv(uniform_projection_view_matrix, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));        
    	    glBindVertexArray(vao_id);
 	        glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);
        };

	    last_time = current_time;

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        ovr_CommitTextureSwapChain(session, _eyeTexture);
        ovrLayerHeader* headerList = &_sceneLayer.Header;
        ovr_SubmitFrame(session, frame, &_viewScaleDesc, &headerList, 1);

        GLuint mirrorTextureId;
        ovr_GetMirrorTextureBufferGL(session, _mirrorTexture, &mirrorTextureId);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
        glBlitFramebuffer(0, 0, _mirrorSize.x, _mirrorSize.y, 0, _mirrorSize.y, _mirrorSize.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);


        glfwSwapBuffers(window);
    }

    //===================================================================================================================================================================================================================
    // Destroy GLFW window and terminate the library
    //===================================================================================================================================================================================================================
    glfwSetKeyCallback(window, 0);
    glfwSetMouseButtonCallback(window, 0);
    glfwDestroyWindow(window);
    glfwTerminate();

    //===================================================================================================================================================================================================================
    // Destroy OVR session and shutdown the library
    //===================================================================================================================================================================================================================
    ovr_Destroy(session);
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

bool gl_check_error()
{
    GLenum error_code = glGetError();
    if (!error_code) return false;
    const char* error_desc;
    switch (error_code) 
    {
        case GL_INVALID_ENUM:      error_desc = "An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag."; break;
        case GL_INVALID_VALUE:     error_desc = "A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag."; break;
        case GL_INVALID_OPERATION: error_desc = "The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag.."; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
                                   error_desc = "The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag."; break;
        case GL_OUT_OF_MEMORY:     error_desc = "There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded."; break;
        case GL_STACK_UNDERFLOW:   error_desc = "An attempt has been made to perform an operation that would cause an internal stack to underflow."; break;
        case GL_STACK_OVERFLOW:    error_desc = "An attempt has been made to perform an operation that would cause an internal stack to overflow."; break;
        default:                   error_desc = "Unknown error.";
    }
    debug_msg("OpenGL Error : %s", error_desc);
    return true;
}

void GL_CALLBACK gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam)
{
    const char *type_str, *severity_str;
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               type_str = "ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_str = "DEPRECATED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  type_str = "UNDEFINED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         type_str = "PORTABILITY"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         type_str = "PERFORMANCE"; break;
        default:                                type_str = "OTHER";
    }

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_LOW:    severity_str = "LOW";    break;
        case GL_DEBUG_SEVERITY_MEDIUM: severity_str = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_HIGH:   severity_str = "HIGH";   break;
        default: severity_str = "UNKNOWN";
    }
    debug_msg("OpenGL Debug :: type = \'%s\' : severity = \'%s\' : id = %d : message = \'%s\'", type_str, severity_str, id, message);
}

void glfw_error_callback(int error, const char* description)
{
    exit_msg("GLFW Error :: error id = %d. description = \'%s\'", error, description);
}
