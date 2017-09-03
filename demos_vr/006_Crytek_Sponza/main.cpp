#include <cstdio>
#include <vector>
#include <map>
#include <istream>
#include <sstream>
#include <fstream>

//#include <iostream>
//#include <memory>
//#include <algorithm>

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
#include "constants.hpp"
#include "glfw_window.hpp"
#include "camera3d.hpp"
#include "shader.hpp"
#include "texture.hpp"




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

//========================================================================================================================================================================================================================
// object loader functions
//========================================================================================================================================================================================================================

struct material_t
{
    std::string name;

    glm::vec3 Ka;                                                                           // "Ka", (rgb-spectral) ambient reflectivity factor
    glm::vec3 Kd;                                                                           // "Kd", diffuse reflectivity factor
    glm::vec3 Ks;                                                                           // "Ks", specular reflectivity factor
    float Ns;                                                                               // "Ns", shininess (exponent) in Phong formula for specular component
    float d;                                                                                // "d", dissolve factor : 0 - fully transparent, 1 - fully opaque

    std::string map_Ka;                                                                     // "map_Ka" : ambient reflectivity texture data file
    std::string map_Kd;                                                                     // "map_Kd" : diffuse reflectivity texture data file
    std::string map_Ks;                                                                     // "map_Ks" : specular reflectivity texture data file
    std::string map_Ns;                                                                     // "map_Ns" : single channel (!) texture file that contains shininess exponent
    std::string map_d;                                                                      // "map_d"  : dissolve/mask/alpha texture
    std::string map_bump;                                                                   // "map_bump" or "bump" : single channel (!) texture that contains heightmap data 

    /* Proper usage of the following five parameters that can be encountered in an mtl-file requires writing a ray-trace engine. */
    /* This will definitely be done at some level of maturity of the ATAS project, but for now let us just ignore them. */

    // int illum;                                                                           // enum indicating illumination model, as WaveFront understands it
    // glm::vec3 Kt;                                                                        // "Kt", transmittance of the material (rgb-spectral factor for light passing through the material)
    // glm::vec3 Ke;                                                                        // "Ke", emissive coeficient, for radio-active surfaces emitting light
    // float Ni;                                                                            // "Ni", optical density of the surface or its index of refraction (determines how light is bent as it passes through the surface)
    // std::string disp;                                                                    // "disp" : heightmap scalar texture used to deform the surface of an object, requires tesselation or complicated geometry shader
};

struct mesh_t
{
    std::string name;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<glm::uvec3> indices;
    std::vector<std::pair<int, int>> material_ids;                                          // per-vertex group material IDs
};

bool load_obj(std::vector<mesh_t>& meshes, std::vector<material_t>& materials, std::string& error_str, const std::string& file_name, const std::string& mtl_base_path);
void load_mtl(std::map<std::string, int>& material_map, std::vector<material_t>& materials, std::istream& input_stream);

//===================================================================================================================================
// Auxiliary structure, needed at the time of obj loading only
//===================================================================================================================================

struct ptn_index
{
    unsigned int p, t, n;

    ptn_index(unsigned int p, unsigned int t, unsigned int n) : p(p), t(t), n(n) {};

    friend bool operator < (const ptn_index& lhs, const ptn_index& rhs)                     // for std::map construction to work
    {
        if (lhs.p < rhs.p) return true;
        if (lhs.p > rhs.p) return false;
        if (lhs.t < rhs.t) return true;
        if (lhs.t > rhs.t) return false;
        if (lhs.n < rhs.n) return true;
        return false;
    }
};

//===================================================================================================================================
// Static methods
//===================================================================================================================================

static unsigned int vertex_update(std::map<ptn_index, unsigned int>& vcache,
             std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& uvs, 
             const std::vector<glm::vec3>& positions_in, const std::vector<glm::vec3>& normals_in, const std::vector<glm::vec2>& uvs_in, const ptn_index& index)
{
    const std::map<ptn_index, unsigned int>::iterator it = vcache.find(index);
    if (it != vcache.end()) return it->second;

    unsigned int idx = positions.size();
    positions.push_back(positions_in[index.p]);
    if (index.t != -1)
        uvs.push_back(uvs_in[index.t]);
    if (index.n != -1)
        normals.push_back(normals_in[index.n]);

    vcache[index] = idx;
    return idx;
}

static void default_material(material_t& material)
{                
    material.name = "";

    material.Ka = glm::vec3(1.0f);
    material.Kd = glm::vec3(1.0f);
    material.Ks = glm::vec3(1.0f);
    material.Ns = 5.0f;
    material.d = 1.0f;

    material.map_Ka = "";  
    material.map_Kd = "";  
    material.map_Ks = "";  
    material.map_Ns = "";  
    material.map_d = "";   
    material.map_bump = "";
}

static bool export_group(mesh_t& mesh, std::map<ptn_index, unsigned int> vcache,
    const std::vector<glm::vec3>& positions_in, const std::vector<glm::vec3>& normals_in, const std::vector<glm::vec2>& uvs_in,
    const std::vector<std::vector<ptn_index>>& faces, const std::string& name)
{
    if (faces.empty()) return false;
    mesh.name = name;    
    for (size_t i = 0; i < faces.size(); i++)                                           // Flatten vertices and indices
    {
        const std::vector<ptn_index>& face = faces[i];

        ptn_index i0 = face[0];
        ptn_index i2 = face[1];

        for (size_t k = 2; k < face.size(); k++)                                         // Polygon -> triangle fan conversion
        {
            ptn_index i1 = i2;
            i2 = face[k];
            unsigned int v0 = vertex_update(vcache, mesh.positions, mesh.normals, mesh.uvs, positions_in, normals_in, uvs_in, i0);
            unsigned int v1 = vertex_update(vcache, mesh.positions, mesh.normals, mesh.uvs, positions_in, normals_in, uvs_in, i1);
            unsigned int v2 = vertex_update(vcache, mesh.positions, mesh.normals, mesh.uvs, positions_in, normals_in, uvs_in, i2);
            mesh.indices.push_back(glm::uvec3(v0, v1, v2));
        }
    }

    if (mesh.normals.empty())
    {
        mesh.normals.resize(mesh.positions.size(), glm::vec3(0.0f));
        for (size_t t = 0; t < mesh.indices.size(); ++t)
        {
            glm::vec3 v1 = mesh.positions[mesh.indices[t].x],
                      v2 = mesh.positions[mesh.indices[t].y],
                      v3 = mesh.positions[mesh.indices[t].z];

            glm::vec3 normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));

            mesh.normals[mesh.indices[t].x] += normal;
            mesh.normals[mesh.indices[t].y] += normal;
            mesh.normals[mesh.indices[t].z] += normal;
        }
        for (size_t n = 0; n < mesh.normals.size(); ++n)
            mesh.normals[n] = glm::normalize(mesh.normals[n]);
    }
    return true;
}

std::istream& operator >> (std::istream& is, glm::vec3& v)
{
    is >> v.x >> v.y >> v.z;
    return is;
};

std::istream& operator >> (std::istream& is, glm::vec2& v)
{
    is >> v.x >> v.y;
    return is;
};

void load_mtl(std::map<std::string, int>& material_map, std::vector<material_t>& materials, std::istream& input_stream)
{
    material_t material;                                                                                        // Create a default material anyway.
    default_material(material);

    int l = -1;

    for (std::string buffer; getline(input_stream, buffer);)
    {      
        ++l;
        if (buffer.empty()) continue;
        std::stringstream line(buffer);
        std::string token;
        line >> token;

        if (token.empty()) continue;
        if (token == "newmtl")
        {
            if (!material.name.empty())
            {
                material_map.insert(std::pair<std::string, int>(material.name, static_cast<int>(materials.size())));
                materials.push_back(material);
            }
            default_material(material);                                                                         // initial temporary material
            line >> material.name;
            continue;
        }

      #define CHECK_TOKEN(name, dest) if (token == name) { if (line >> dest) continue; else debug_msg("Error parsing material file at line %d.", l); }
                                                                                                                        
        CHECK_TOKEN("Ka", material.Ka);
        CHECK_TOKEN("Kd", material.Kd);
        CHECK_TOKEN("Ks", material.Ks);
        CHECK_TOKEN("Ns", material.Ns);
        CHECK_TOKEN("d",  material.d);

        CHECK_TOKEN("map_Ka", material.map_Ka);
        CHECK_TOKEN("map_Kd", material.map_Kd);
        CHECK_TOKEN("map_Ks", material.map_Ks);
        CHECK_TOKEN("map_Ns", material.map_Ns);
        CHECK_TOKEN("map_d",  material.map_d);

        CHECK_TOKEN("map_bump", material.map_bump);
        CHECK_TOKEN("bump",     material.map_bump);

        // CHECK_TOKEN("illum", material.illum);
        // CHECK_TOKEN("Kt",    material.Kt);
        // CHECK_TOKEN("Ke",    material.Ke);
        // CHECK_TOKEN("Ni",    material.Ni);
        // CHECK_TOKEN("disp",  material.disp);

      #undef CHECK_TOKEN
        
        // still here? ... everything else is a crap that we silently ignore ...
        // ... maybe we should qDebug("Token %s is not supported.", token.c_str());
    }
    
    material_map.insert(std::pair<std::string, int>(material.name, static_cast<int>(materials.size())));        // flush last material
    materials.push_back(material);
}



bool load_obj(std::vector<mesh_t>& meshes, std::vector<material_t>& materials, const std::string& file_name, const std::string& mtl_base_path)
{
    std::ifstream input_stream(file_name);
    if (!input_stream)
    {
        debug_msg("Cannot open object file %s.", file_name.c_str());
        return false;
    }

    std::string name;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;

    std::vector<std::vector<ptn_index>> faces;

    std::map<std::string, int> material_map;                                                                    // materials
    std::map<ptn_index, unsigned int> vcache;
    int material_id = -1;

    mesh_t mesh;

    for (std::string buffer; getline(input_stream, buffer);)
    {        
        if (buffer.empty()) continue;
        std::stringstream line(buffer);
        std::string token;
        line >> token;
        
        if (token == "v")
        {
            glm::vec3 position;
            line >> position;
            positions.push_back(position);
            continue;
        }
        if (token == "vn")
        {
            glm::vec3 normal;
            line >> normal;
            normals.push_back(normal);
            continue;
        }
        if (token == "vt")
        {
            glm::vec2 uv;
            line >> uv;
            uvs.push_back(uv);
            continue;
        }
        
        if (token == "f")
        {
            std::vector<ptn_index> face;
            std::string triple;
            while(line >> triple)                                              // Parse triples: p, p//n, p/t, p/t/n
            {
                std::stringstream triple_ss(triple);
                int p, t = -1, n = -1;
                triple_ss >> p;
                if (p > 0) --p;
                if (p < 0) p = positions.size() + p;

                char slash;
                triple_ss >> slash; 
                if (slash != '/')                                               // we got only position index : p
                {
                    face.push_back(ptn_index(p, t, n));
                    continue;
                }
    
                if (triple_ss.peek() != '/')                                    // two slashes in a row, so the input must be of the form p//n
                {
                    triple_ss >> t;
                        if (t > 0) --t;
                    if (t < 0) t = uvs.size() + t;

                    triple_ss >> slash; 
                    if (slash != '/')                                           // at this point we have p/t, push it
                    {
                        face.push_back(ptn_index(p, t, n));
                        continue;
                    }
                }
                triple_ss >> n;
                if (n > 0) --n;
                if (n < 0) n = normals.size() + n;

                face.push_back(ptn_index(p, t, n));
            }
            faces.push_back(face);
            continue;
        }

        if (token == "mtllib")                                     // load mtl
        {
            std::string mtl_file_name;
            line >> mtl_file_name;
            mtl_file_name = mtl_base_path + mtl_file_name;
            std::ifstream material_stream(mtl_file_name);
            if (!material_stream)
            {
                debug_msg("ERROR : Material file %s not found.", mtl_file_name.c_str());
                return false;
            }
            load_mtl(material_map, materials, material_stream);
            continue;
        }
    
        if (token == "usemtl")
        {   
            std::string mtl_name;
            line >> mtl_name;
            std::map<std::string, int>::iterator it = material_map.find(mtl_name); 
            int new_material_id = (it == material_map.end()) ? -1 : it->second;                                     // TODO : trigger something if material has not been found (new_material_id == -1)

            if (new_material_id != material_id)                                                                      // Create per-face material
            {
                if (export_group(mesh, vcache, positions, normals, uvs, faces, name))
                    mesh.material_ids.push_back(std::pair<int, int>(material_id, mesh.indices.size()));
                vcache.clear();
                faces.clear();
                material_id = new_material_id;
            }
            continue;
        }
    
        if ((token == "g") || (token == "o"))                                 // group name
        {                                                                                                       // flush previous face group.
            if (export_group(mesh, vcache, positions, normals, uvs, faces, name))
            {
                mesh.material_ids.push_back(std::pair<int, int>(material_id, mesh.indices.size()));
                meshes.push_back(mesh);
            }
            vcache.clear();
            mesh = mesh_t();
            faces.clear();
            line >> name;
            continue;
        }
    }

    if (export_group(mesh, vcache, positions, normals, uvs, faces, name))
    {
        mesh.material_ids.push_back(std::pair<int, int>(material_id, mesh.indices.size()));
        meshes.push_back(mesh);
    }
    return true;
}


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
        ovrMatrix4f ovrPerspectiveProjection = ovrMatrix4f_Projection(erd.Fov, 0.01f, 100000.0f, ovrProjection_ClipRangeOpenGL);
        _eyeProjections[eye] = toGlm(ovrPerspectiveProjection);
        _viewScaleDesc.HmdToEyeOffset[eye] = erd.HmdToEyeOffset;

        ovrFovPort & fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
        auto eyeSize = ovr_GetFovTextureSize(session, eye, fov, 1.0f);
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
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, _renderTargetSize.x, _renderTargetSize.y);
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
    // Load standard Blinn-Phong shader
    //===================================================================================================================================================================================================================
    glsl_program blinn_phong(glsl_shader(GL_VERTEX_SHADER,   "glsl/blinn-phong.vs"),
                             glsl_shader(GL_FRAGMENT_SHADER, "glsl/blinn-phong.fs"));

    blinn_phong.enable();
    glUniform1i(blinn_phong.uniform_id("map_Kd"), 0);
    glUniform1i(blinn_phong.uniform_id("map_bump"), 1);
    glUniform1i(blinn_phong.uniform_id("map_d"), 2);

    GLint uniform_camera_ws = blinn_phong.uniform_id("camera_ws");
    GLint uniform_light_ws  = blinn_phong.uniform_id("light_ws");
    GLint uniform_Ka        = blinn_phong.uniform_id("Ka");
    GLint uniform_Kd        = blinn_phong.uniform_id("Kd");
    GLint uniform_Ks        = blinn_phong.uniform_id("Ks");
    GLint uniform_Ns        = blinn_phong.uniform_id("Ns");
    GLint uniform_d         = blinn_phong.uniform_id("d");
    GLint uniform_mask_texture = blinn_phong.uniform_id("mask_texture");

    //===================================================================================================================================================================================================================
    // Global OpenGL state : since there are no depth writes, depth buffer needs not be cleared
    //===================================================================================================================================================================================================================
    glClearColor(0.03f, 0.0f, 0.09f, 1.0f);
    glEnable(GL_DEPTH_TEST);   
    glEnable(GL_CULL_FACE);

    //===============================================================================================================================
    // create UBO for common uniforms, bind it to UBO target 0 and connect with shader uniform blocks
    //===============================================================================================================================
    struct
    {
        glm::mat4 projection_view_matrix;
        glm::mat4 projection_matrix;
        glm::mat4 view_matrix;
        glm::mat4 camera_matrix;
    } matrices;

    GLuint ubo_id;
    glGenBuffers(1, &ubo_id);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(matrices), 0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_id);

    blinn_phong.bind_uniform_buffer("matrices", 0);

    //===============================================================================================================================
    // create VBOs from a set of vectors
    //===============================================================================================================================

    std::vector<mesh_t> meshes;
    std::vector<material_t> materials;

    std::vector<GLuint> vao_id;
    std::vector<GLuint> pbo_id;
    std::vector<GLuint> nbo_id;
    std::vector<GLuint> tbo_id;
    std::vector<GLuint> ibo_id;

    std::string objPath = "crytek-sponza\\sponza.obj";
    std::string mtlPath = "crytek-sponza\\";

    if (!load_obj(meshes, materials, objPath, mtlPath))
        exit_msg("Error loading object file %s. Exiting", objPath.c_str());

    size_t meshes_size = meshes.size();

    vao_id.resize(meshes_size);
    pbo_id.resize(meshes_size);
    nbo_id.resize(meshes_size);
    tbo_id.resize(meshes_size);
    ibo_id.resize(meshes_size);

    glGenVertexArrays(meshes_size, vao_id.data());
    glGenBuffers(meshes_size, pbo_id.data());
    glGenBuffers(meshes_size, nbo_id.data());
    glGenBuffers(meshes_size, tbo_id.data());
    glGenBuffers(meshes_size, ibo_id.data());

    for (size_t i = 0; i < meshes_size; ++i)
    {

        glBindVertexArray(vao_id[i]);
        glBindBuffer(GL_ARRAY_BUFFER, pbo_id[i]);
        glBufferData(GL_ARRAY_BUFFER, meshes[i].positions.size() * sizeof(glm::vec3), glm::value_ptr(meshes[i].positions[0]), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, nbo_id[i]);
        glBufferData(GL_ARRAY_BUFFER, meshes[i].normals.size() * sizeof(glm::vec3), glm::value_ptr(meshes[i].normals[0]), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, tbo_id[i]);
        glBufferData(GL_ARRAY_BUFFER, meshes[i].uvs.size() * sizeof(glm::vec2), glm::value_ptr(meshes[i].uvs[0]), GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshes[i].indices.size() * sizeof(glm::uvec3), glm::value_ptr(meshes[i].indices[0]), GL_STATIC_DRAW);
    }

    
    std::map<std::string, GLuint> textures;    

    for (size_t i = 0; i < materials.size(); ++i)
    {

      #define TEXTURE_LOAD(name) if (!name.empty()) { std::string path = mtlPath + name; \
                                                      std::map<std::string, GLuint>::iterator it = textures.find(name); \
                                                      if (it == textures.end()) \
                                                          textures[name] = texture::texture2d_png(path.c_str()); \
                                                    }

        TEXTURE_LOAD(materials[i].map_Ka);   
        TEXTURE_LOAD(materials[i].map_Kd);   
        TEXTURE_LOAD(materials[i].map_Ks);   
        TEXTURE_LOAD(materials[i].map_Ns);   
        TEXTURE_LOAD(materials[i].map_d);    
        TEXTURE_LOAD(materials[i].map_bump); 

      #undef TEXTURE_LOAD
    }


    //===================================================================================================================================================================================================================
    // Log the information about loaded object
    //===================================================================================================================================================================================================================
    debug_msg("# of meshes : %ld.\n# of materials : %ld.\n", meshes.size(), materials.size());
    for (size_t i = 0; i < meshes.size(); i++)
    {
        debug_msg("mesh[%ld].name = %s.\t\t#vertices = %ld, normals = %ld, uvs = %ld, #indices = %ld, #materials = %ld.", 
                  i, meshes[i].name.c_str(), meshes[i].positions.size(), meshes[i].normals.size(), meshes[i].uvs.size(), meshes[i].indices.size(), meshes[i].material_ids.size());
    }

    for (size_t i = 0; i < materials.size(); i++)
    {
        debug_msg("material[%ld].name = %s.", i, materials[i].name.c_str());
        debug_msg("\tmaterial.Ka = %s.", glm::to_string(materials[i].Ka).c_str());
        debug_msg("\tmaterial.Kd = %s.", glm::to_string(materials[i].Kd).c_str());
        debug_msg("\tmaterial.Ks = %s.", glm::to_string(materials[i].Ks).c_str());
        debug_msg("\tmaterial.Ns = %f.", materials[i].Ns);
        debug_msg("\tmaterial.d  = %f.", materials[i].d);

        debug_msg("\tmaterial.map_Ka = %s", materials[i].map_Ka.c_str());
        debug_msg("\tmaterial.map_Kd = %s", materials[i].map_Kd.c_str());
        debug_msg("\tmaterial.map_Ks = %s", materials[i].map_Ks.c_str());
        debug_msg("\tmaterial.map_Ns = %s", materials[i].map_Ns.c_str());
        debug_msg("\tmaterial.map_d = %s", materials[i].map_d.c_str());
        debug_msg("\tmaterial.map_bump = %s", materials[i].map_bump.c_str());
    }

    glActiveTexture(GL_TEXTURE0);
    const float light_radius = 100.0f;


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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = glfwGetTime();
        glm::vec4 light_ws = glm::vec4(light_radius * cos(5.5f * time), light_radius * sin(5.5f * time), -0.66f * light_radius, 1.0f);

        for (ovrEyeType eye = ovrEyeType::ovrEye_Left; eye < ovrEyeType::ovrEye_Count; eye = static_cast<ovrEyeType>(eye + 1))
        {
            const auto& vp = _sceneLayer.Viewport[eye];
            glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
            _sceneLayer.RenderPose[eye] = eyePoses[eye];
            blinn_phong.enable();

            glm::mat4 camera_ws1 = camera.view_matrix;
            matrices.projection_matrix = _eyeProjections[eye];
            matrices.view_matrix = toGlm(eyePoses[eye]) * camera_ws1;
            matrices.projection_view_matrix = matrices.projection_matrix * matrices.view_matrix;
            matrices.camera_matrix = glm::inverse(matrices.view_matrix);

            float time = glfwGetTime();
            glm::vec3 light_ws = glm::vec3(light_radius * cos(time), 150.0f, light_radius * sin(time));
            glm::vec3 camera_ws = -glm::vec3(matrices.view_matrix[3]);

            glUniform3fv(uniform_light_ws, 1, glm::value_ptr(light_ws));
            glUniform3fv(uniform_camera_ws, 1, glm::value_ptr(camera_ws));



            //===========================================================================================================================================================================================================
            // Write common shader data to shared uniform buffer
            //===========================================================================================================================================================================================================
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
            GLvoid* buf_ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
            memcpy(buf_ptr, &matrices, sizeof(matrices));
            glUnmapBuffer(GL_UNIFORM_BUFFER);
            
            for (size_t i = 0; i < meshes.size(); ++i)
            {
            
                mesh_t& mesh = meshes[i];
                glBindVertexArray(vao_id[i]);
                GLuint index = 0;
                for (size_t j = 0; j < mesh.material_ids.size(); ++j)
                {
                    std::pair<int, int> material_index = mesh.material_ids[j];
            
                    material_t& material = materials[material_index.first];
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, textures[material.map_Kd]);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, textures[material.map_bump]);
            
                    glUniform3fv(uniform_Ka, 1, glm::value_ptr(material.Ka));
                    glUniform3fv(uniform_Kd, 1, glm::value_ptr(material.Kd));
                    glUniform3fv(uniform_Ks, 1, glm::value_ptr(material.Ks));
                    glUniform1f (uniform_Ns, material.Ns);
            
                    if (material.map_d.empty())
                    {
                        glUniform1i (uniform_mask_texture, 0);
                    }
                    else
                    {
                        glUniform1i (uniform_mask_texture, 1);
                        glUniform1f (uniform_d, material.d);
                        glActiveTexture(GL_TEXTURE2);
                        glBindTexture(GL_TEXTURE_2D, textures[material.map_d]);
                    }
                    glDrawElements(GL_TRIANGLES, 3 * (material_index.second - index), GL_UNSIGNED_INT, (const GLvoid*) (sizeof(glm::uvec3) * index));
                    index = material_index.second;
                }       
            }

        };

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
