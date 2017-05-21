//========================================================================================================================================================================================================================
// DEMO 067 : HBAO
//========================================================================================================================================================================================================================
#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "log.hpp"
#include "gl_info.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "geometry.hpp"
#include "obj1.hpp"
#include "shader.hpp"
#include "surface1.hpp"
#include "model1.hpp"
#include "mtl.hpp"
#include "fbo2d.hpp"
#include "mesh2geometry.hpp"
#include "material2surface.hpp"

const int WIDTH = 1920;
const int HEIGHT = 1080;
const int RES_RATIO = 2;
const int AO_WIDTH = WIDTH / RES_RATIO;
const int AO_HEIGHT = HEIGHT / RES_RATIO;
const int AO_DIRS = 6;
const int AO_SAMPLES = 3;

const float AO_RADIUS = 0.3f;
const float AO_STRENGTH = 2.75;
const float AO_MAX_RADIUS_PIXELS = 50.0;

#define NOISE_RES 4

GLuint generateNoiseTexture(int width, int height)
{
    GLuint noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);

    float *noise = new float[width * height * 4];
    for(int y = 0; y < height; ++y)
    {
        for(int x = 0; x < width; ++x)
        {
            glm::vec2 xy = glm::circularRand(1.0f);
            float z = glm::linearRand(0.0f, 1.0f);
            float w = glm::linearRand(0.0f, 1.0f);

            int offset = 4 * (y * width + x);
            noise[offset + 0] = xy[0];
            noise[offset + 1] = xy[1];
            noise[offset + 2] = z;
            noise[offset + 3] = w;
        }
    }

    GLint internalFormat = GL_RGBA16F;
    GLint format = GL_RGBA;
    GLint type = GL_FLOAT;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, noise);
    return noiseTexture;
}

Model* mdl;
Geometry* model;

double timeStamps[7];
unsigned int queryID[7];

Surface * surface0;

std::map<std::string, Surface*> surfaces;
std::vector<Geometry> models;

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    bool blur = false;
    bool fullres = true;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
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

        if((key == GLFW_KEY_B) && (action == GLFW_RELEASE))
            blur = !blur;
        else if ((key == GLFW_KEY_F) && (action == GLFW_RELEASE))
            fullres = !fullres;

    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("HBAO", 4, 3, 3, WIDTH, HEIGHT, false);

    gl_error_msg();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glActiveTexture(GL_TEXTURE0);
    glClearColor(0.17f, 0.23f, 0.57f, 1.0f);
    glClearDepth(1.0f);

    //===================================================================================================================================================================================================================
    // data initialization
    //===================================================================================================================================================================================================================

    Surface::init();
    mdl = new Model();

    Mesh mesh = loadMeshFromObj("../../../resources/models/obj/dragon_low_poly.obj", 0.1);
    Geometry dragon = createGeometryFromMesh(mesh);

    Surface *surface = new Surface();
    surfaces.insert(std::pair<std::string, Surface*> (std::string("default"), surface));

    mdl->addGeometryAndSurface(&dragon, surface);

    std::vector<Mesh> meshes = loadMeshesFromObj("../../../resources/models/obj/crytek-sponza/sponza.obj", 0.01f);
    std::vector<Geometry> geometries = createGeometryFromMesh(meshes);
    std::vector<Material> materials = loadMaterialsFromMtl("../../../resources/models/obj/crytek-sponza/sponza.mtl");
    surfaces = createSurfaceFromMaterial(materials, "../../../resources/models/obj/crytek-sponza/");

    for(unsigned int i = 0; i < geometries.size(); ++i)
        mdl->addGeometryAndSurface(&geometries[i], surfaces[geometries[i].material]);

    mdl->prepare();

    glm::mat4& projection_matrix = window.camera.projection_matrix;
    glm::vec2 FocalLen = glm::vec2(projection_matrix[0][0], projection_matrix[1][1]);
    glm::vec2 InvFocalLen = 1.0f / FocalLen;
    glm::vec2 UVToViewA = -2.0f * InvFocalLen;
    glm::vec2 UVToViewB = InvFocalLen;
    glm::vec2 LinMAD = glm::vec2(1.0 / (projection_matrix[3][2]), projection_matrix[2][2] / (projection_matrix[3][2]));

    //===================================================================================================================================================================================================================
    // shaders initialization
    //===================================================================================================================================================================================================================

    glsl_program_t geometryShader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/geometry.vs"),
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/geometry.fs"));
    geometryShader.enable();
    geometryShader["projection_matrix"] = projection_matrix;
    uniform_t uni_gs_view_matrix = geometryShader["view_matrix"];


    glsl_shader_t quad_vs(GL_VERTEX_SHADER, "glsl/fullscreen.vs");

    glsl_program_t compositeShader(quad_vs, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/composite.fs"));
    compositeShader.enable();
    compositeShader["texture0"] = 0;
    compositeShader["texture1"] = 1;
    compositeShader["texture2"] = 2;
    compositeShader["texture3"] = 3;


    glsl_program_t blurXShader(quad_vs, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blur_x.fs"));
    blurXShader.enable();
    blurXShader["texture0"] = 0;
    blurXShader["FullRes"] = glm::vec2(window.res_x, window.res_y);
    blurXShader["InvFullRes"] = 1.0f / glm::vec2(window.res_x, window.res_y);
    blurXShader["LinMAD"] = LinMAD;


    glsl_program_t blurYShader(quad_vs, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blur_y.fs"));
    blurYShader.enable();
    blurYShader["texture0"] = 0;
    blurYShader["FullRes"] = glm::vec2(window.res_x, window.res_y);
    blurYShader["InvFullRes"] = 1.0f / glm::vec2(window.res_x, window.res_y);
    blurYShader["LinMAD"] = LinMAD;


    glsl_program_t downsampleShader(quad_vs, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/downsample_depth.fs"));
    downsampleShader.enable();
    downsampleShader["texture0"] = 0;
    downsampleShader["LinMAD"] = LinMAD;
    downsampleShader["ResRatio"] = RES_RATIO;


    glsl_program_t upsampleShader(quad_vs, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/upsample_aoz.fs"));
    upsampleShader.enable();
    upsampleShader["texture0"] = 0;
    upsampleShader["texture1"] = 1;
    upsampleShader["LinMAD"] = LinMAD;


    glsl_program_t hbaoHalfShader(quad_vs, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/hbao.fs"));
    hbaoHalfShader.enable();
    hbaoHalfShader["texture0"] = 0;
    hbaoHalfShader["texture1"] = 1;
    hbaoHalfShader["FocalLen"] = FocalLen;
    hbaoHalfShader["UVToViewA"] = UVToViewA;
    hbaoHalfShader["UVToViewB"] = UVToViewB;
    hbaoHalfShader["LinMAD"] = LinMAD;
    hbaoHalfShader["AORes"] = glm::vec2(AO_WIDTH, AO_HEIGHT);
    hbaoHalfShader["InvAORes"] = 1.0f / glm::vec2(AO_WIDTH, AO_HEIGHT);
    hbaoHalfShader["R"] = AO_RADIUS;
    hbaoHalfShader["R2"] = AO_RADIUS * AO_RADIUS;
    hbaoHalfShader["NegInvR2"] = -1.0f / (AO_RADIUS * AO_RADIUS);
    hbaoHalfShader["AOStrength"] = AO_STRENGTH;
    hbaoHalfShader["MaxRadiusPixels"] = AO_MAX_RADIUS_PIXELS / (float) RES_RATIO;
    hbaoHalfShader["NoiseScale"] = glm::vec2(AO_WIDTH, AO_HEIGHT) / NOISE_RES;
    hbaoHalfShader["NumDirections"] = AO_DIRS;
    hbaoHalfShader["NumSamples"] = AO_SAMPLES;


    glsl_program_t hbaoFullShader(quad_vs, glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/hbao_full.fs"));
    hbaoFullShader.enable();
    hbaoFullShader["texture0"] = 0;
    hbaoFullShader["texture1"] = 1;
    hbaoFullShader["FocalLen"] = FocalLen;
    hbaoFullShader["UVToViewA"] = UVToViewA;
    hbaoFullShader["UVToViewB"] = UVToViewB;
    hbaoFullShader["LinMAD"] = LinMAD;
    hbaoFullShader["AORes"] = glm::vec2(WIDTH, HEIGHT);
    hbaoFullShader["InvAORes"] = 1.0f / glm::vec2(WIDTH, HEIGHT);
    hbaoFullShader["R"] = AO_RADIUS;
    hbaoFullShader["R2"] = AO_RADIUS * AO_RADIUS;
    hbaoFullShader["NegInvR2"] = -1.0f / (AO_RADIUS * AO_RADIUS);
    hbaoFullShader["AOStrength"] = AO_STRENGTH;
    hbaoFullShader["MaxRadiusPixels"] = (float) AO_MAX_RADIUS_PIXELS;
    hbaoFullShader["NoiseScale"] = glm::vec2(WIDTH, HEIGHT) / NOISE_RES;
    hbaoFullShader["NumDirections"] = AO_DIRS;
    hbaoFullShader["NumSamples"] = AO_SAMPLES;


    Framebuffer2D fboFullRes(WIDTH, HEIGHT);
    fboFullRes.attachBuffer(FBO_DEPTH, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    fboFullRes.attachBuffer(FBO_AUX0, GL_RGBA8, GL_RGBA, GL_FLOAT);
    fboFullRes.attachBuffer(FBO_AUX1, GL_RG16F, GL_RG, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    fboFullRes.attachBuffer(FBO_AUX2, GL_RG16F, GL_RG, GL_FLOAT, GL_LINEAR, GL_LINEAR);

    Framebuffer2D fboHalfRes(AO_WIDTH, AO_HEIGHT);
    fboHalfRes.attachBuffer(FBO_AUX0, GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    fboHalfRes.attachBuffer(FBO_AUX1, GL_R8, GL_RED, GL_FLOAT, GL_LINEAR, GL_LINEAR);

    GLuint noiseTexture = generateNoiseTexture(NOISE_RES, NOISE_RES);

    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while (!window.should_close())
    {
        //===============================================================================================================================================================================================================
        // calculate FPS
        //===============================================================================================================================================================================================================
        window.new_frame();
        GLuint64 timeStamp[7];

        for(int i = 0; i < 7; ++i)
            glGetQueryObjectui64v(queryID[i], GL_QUERY_RESULT, &timeStamp[i]);

        double geom = (timeStamp[1] - timeStamp[0]) / 1000000.0;
        double down = (timeStamp[2] - timeStamp[1]) / 1000000.0;
        double ao   = (timeStamp[3] - timeStamp[2]) / 1000000.0;
        double up   = (timeStamp[4] - timeStamp[3]) / 1000000.0;
        double blur = (timeStamp[5] - timeStamp[4]) / 1000000.0;
        double comp = (timeStamp[6] - timeStamp[5]) / 1000000.0;
        double tot  = (timeStamp[6] - timeStamp[0]) / 1000000.0;

        static char title[256];
        sprintf(title, "%s, FPS: %2.1f, time(ms): geom %2.2f, down %2.2f, ao %2.2f, up %2.2f, blur %2.2f, comp %2.2f tot %2.2f",
            window.fullres ? "FullRes" : "HalfRes", window.fps(), geom, down, ao, up, blur, comp, tot);
        window.set_title(title);

        glGenQueries(7, queryID);
        glQueryCounter(queryID[0], GL_TIMESTAMP);

        //===============================================================================================================================================================================================================
        // RENDER GEOMETRY PASS
        //===============================================================================================================================================================================================================
        glEnable(GL_DEPTH_TEST);
        fboFullRes.bind();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        geometryShader.enable();
        uni_gs_view_matrix = window.camera.view_matrix;
        mdl->draw();
        glQueryCounter(queryID[1], GL_TIMESTAMP);

        //===============================================================================================================================================================================================================
        // Enable fullscreen quad fake vao and downsample depth id necessary
        //===============================================================================================================================================================================================================
        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(vao_id);

        if(!window.fullres)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboFullRes.getBufferHandle(FBO_DEPTH));
            fboHalfRes.bind();                                                       // Downsample depth
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            downsampleShader.enable();
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    
        glQueryCounter(queryID[2], GL_TIMESTAMP);
    
        //===============================================================================================================================================================================================================
        // AO pass
        //===============================================================================================================================================================================================================
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glActiveTexture(GL_TEXTURE0);

        if(!window.fullres)
        {
            glBindTexture(GL_TEXTURE_2D, fboHalfRes.getBufferHandle(FBO_AUX0));
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            hbaoHalfShader.enable();
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, fboFullRes.getBufferHandle(FBO_DEPTH));
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            hbaoFullShader.enable();
        }
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);    
        glQueryCounter(queryID[3], GL_TIMESTAMP);
    
        //===============================================================================================================================================================================================================
        // Upsample
        //===============================================================================================================================================================================================================    
        if(!window.fullres)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboFullRes.getBufferHandle(FBO_DEPTH));
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, fboHalfRes.getBufferHandle(FBO_AUX1));
            fboFullRes.bind();
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            upsampleShader.enable();
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
        glQueryCounter(queryID[4], GL_TIMESTAMP);
    
        //===============================================================================================================================================================================================================
        // Blur pass
        //===============================================================================================================================================================================================================    
        if(window.blur)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboFullRes.getBufferHandle(FBO_AUX1));
    
            glDrawBuffer(GL_COLOR_ATTACHMENT2);
            blurXShader.enable();
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            blurYShader.enable();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboFullRes.getBufferHandle(FBO_AUX2));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
        glQueryCounter(queryID[5], GL_TIMESTAMP);    
    
        //===============================================================================================================================================================================================================
        // RENDER TO SCREEN PASS
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fboFullRes.getBufferHandle(FBO_AUX0));    
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fboFullRes.getBufferHandle(FBO_AUX1));
        glViewport(0, 0, WIDTH, HEIGHT);
        compositeShader.enable();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glQueryCounter(queryID[6], GL_TIMESTAMP);

        GLint stopTimerAvailable = 0;
        bool stop = false;

        while (!stop)
        {
            stop = true;
            for(int i = 0; i < 7; ++i)
            {
                glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
                if(!stopTimerAvailable) stop = false;
            }
        }

        window.end_frame();
    }

    delete model;
    delete surface0;

    for(std::map<std::string, Surface*>::iterator it = surfaces.begin(); it != surfaces.end(); ++it)
        delete it->second;

    glDeleteTextures(1, &noiseTexture);
    Surface::cleanUp();

    glfw::terminate();
    return 0;
}