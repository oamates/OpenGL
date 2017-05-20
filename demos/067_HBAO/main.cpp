//========================================================================================================================================================================================================================
// DEMO 067 : HBAO
//========================================================================================================================================================================================================================
#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
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
#include "shader1.hpp"
#include "surface1.hpp"
#include "model1.hpp"
#include "mtl.hpp"
#include "fbo2d.hpp"
#include "mesh2geometry.hpp"
#include "material2surface.hpp"

#define WIDTH 1024
#define HEIGHT 768

#define RES_RATIO 2
#define AO_WIDTH (WIDTH / RES_RATIO)
#define AO_HEIGHT (HEIGHT / RES_RATIO)
#define AO_RADIUS 0.3
#define AO_DIRS 6
#define AO_SAMPLES 3
#define AO_STRENGTH 17.5;
#define AO_MAX_RADIUS_PIXELS 50.0

#define NOISE_RES 4

#define MOVE_SPEED 1.5
#define MOUSE_SPEED 1.5

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
Geometry* fsquad;

Framebuffer2D* fboFullRes;
Framebuffer2D* fboHalfRes;

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

#include <glm/gtx/string_cast.hpp>

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
    glfwSwapInterval(0);
    glActiveTexture(GL_TEXTURE0);
    glClearColor(1.0, 1.0, 1.0, 0.0);

    //===================================================================================================================================================================================================================
    // data and shaders initialization
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

    fsquad = new Geometry();

    Geometry::sVertex v;
    v.position = glm::vec3(-1.0f, -1.0f, 0.0f); v.texCoord = glm::vec2(0.0f, 0.0f); fsquad->addVertex(v);
    v.position = glm::vec3( 1.0f, -1.0f, 0.0f); v.texCoord = glm::vec2(1.0f, 0.0f); fsquad->addVertex(v);
    v.position = glm::vec3( 1.0f,  1.0f, 0.0f); v.texCoord = glm::vec2(1.0f, 1.0f); fsquad->addVertex(v);
    v.position = glm::vec3(-1.0f,  1.0f, 0.0f); v.texCoord = glm::vec2(0.0f, 1.0f); fsquad->addVertex(v);

    fsquad->addTriangle(glm::uvec3(0, 1, 2));
    fsquad->addTriangle(glm::uvec3(0, 2, 3));
    fsquad->createStaticBuffers();


    //glsl_program_t geometryShader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/geometry.vs"),
    //                              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/geometry.FS"));

    //geometryShader.enable();


    Shader geometryShader("glsl/geometry.vs", "glsl/geometry.fs");
    Shader hbaoHalfShader("glsl/fullscreen.vs", "glsl/hbao.fs");
    Shader hbaoFullShader("glsl/fullscreen.vs", "glsl/hbao_full.fs");
    Shader compositShader("glsl/fullscreen.vs", "glsl/composite.fs");
    Shader blurXShader("glsl/fullscreen.vs", "glsl/blur_x.fs");
    Shader blurYShader("glsl/fullscreen.vs", "glsl/blur_y.fs");
    Shader downsampleShader("glsl/fullscreen.vs", "glsl/downsample_depth.fs");
    Shader upsampleShader("glsl/fullscreen.vs", "glsl/upsample_aoz.fs");

    // Full res deferred base
    fboFullRes = new Framebuffer2D(WIDTH, HEIGHT);
    fboFullRes->attachBuffer(FBO_DEPTH, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    fboFullRes->attachBuffer(FBO_AUX0, GL_RGBA8, GL_RGBA, GL_FLOAT);
    fboFullRes->attachBuffer(FBO_AUX1, GL_RG16F, GL_RG, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    fboFullRes->attachBuffer(FBO_AUX2, GL_RG16F, GL_RG, GL_FLOAT, GL_LINEAR, GL_LINEAR);

    // Half res buffer for AO
    fboHalfRes = new Framebuffer2D(AO_WIDTH, AO_HEIGHT);
    fboHalfRes->attachBuffer(FBO_AUX0, GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    fboHalfRes->attachBuffer(FBO_AUX1, GL_R8, GL_RED, GL_FLOAT, GL_LINEAR, GL_LINEAR);

    glm::mat4& projection_matrix = window.camera.projection_matrix;

    glm::vec2 FocalLen = glm::vec2(projection_matrix[0][0], projection_matrix[1][1]);
    glm::vec2 InvFocalLen = 1.0f / FocalLen;
    glm::vec2 UVToViewA = -2.0f * InvFocalLen;
    glm::vec2 UVToViewB = InvFocalLen;
    glm::vec2 LinMAD = glm::vec2(1.0 / (projection_matrix[3][2]), projection_matrix[2][2] / (projection_matrix[3][2]));


    hbaoHalfShader.bind();
    int pos;
    pos = hbaoHalfShader.getUniformLocation("FocalLen");
    glUniform2f(pos, FocalLen[0], FocalLen[1]);
    pos = hbaoHalfShader.getUniformLocation("UVToViewA");
    glUniform2f(pos, UVToViewA[0], UVToViewA[1]);
    pos = hbaoHalfShader.getUniformLocation("UVToViewB");
    glUniform2f(pos, UVToViewB[0], UVToViewB[1]);
    pos = hbaoHalfShader.getUniformLocation("LinMAD");
    glUniform2f(pos, LinMAD[0], LinMAD[1]);

    pos = hbaoHalfShader.getUniformLocation("AORes");
    glUniform2f(pos, (float)AO_WIDTH, (float)AO_HEIGHT);
    pos = hbaoHalfShader.getUniformLocation("InvAORes");
    glUniform2f(pos, 1.0f/(float)AO_WIDTH, 1.0f/(float)AO_HEIGHT);

    pos = hbaoHalfShader.getUniformLocation("R");
    glUniform1f(pos, AO_RADIUS);
    pos = hbaoHalfShader.getUniformLocation("R2");
    glUniform1f(pos, AO_RADIUS*AO_RADIUS);
    pos = hbaoHalfShader.getUniformLocation("NegInvR2");
    glUniform1f(pos, -1.0f / (AO_RADIUS*AO_RADIUS));
    pos = hbaoHalfShader.getUniformLocation("MaxRadiusPixels");
    glUniform1f(pos, AO_MAX_RADIUS_PIXELS / (float)RES_RATIO);

    pos = hbaoHalfShader.getUniformLocation("NoiseScale");
    glUniform2f(pos, (float)AO_WIDTH/(float)NOISE_RES, (float)AO_HEIGHT/(float)NOISE_RES);
    pos = hbaoHalfShader.getUniformLocation("NumDirections");
    glUniform1i(pos, AO_DIRS);
    pos = hbaoHalfShader.getUniformLocation("NumSamples");
    glUniform1i(pos, AO_SAMPLES);

    hbaoFullShader.bind();
    pos = hbaoFullShader.getUniformLocation("FocalLen");
    glUniform2f(pos, FocalLen[0], FocalLen[1]);
    pos = hbaoFullShader.getUniformLocation("UVToViewA");
    glUniform2f(pos, UVToViewA[0], UVToViewA[1]);
    pos = hbaoFullShader.getUniformLocation("UVToViewB");
    glUniform2f(pos, UVToViewB[0], UVToViewB[1]);
    pos = hbaoFullShader.getUniformLocation("LinMAD");
    glUniform2f(pos, LinMAD[0], LinMAD[1]);

    pos = hbaoFullShader.getUniformLocation("AORes");
    glUniform2f(pos, (float)WIDTH, (float)HEIGHT);
    pos = hbaoFullShader.getUniformLocation("InvAORes");
    glUniform2f(pos, 1.0f/(float)WIDTH, 1.0f/(float)HEIGHT);

    pos = hbaoFullShader.getUniformLocation("R");
    glUniform1f(pos, AO_RADIUS);
    pos = hbaoFullShader.getUniformLocation("R2");
    glUniform1f(pos, AO_RADIUS*AO_RADIUS);
    pos = hbaoFullShader.getUniformLocation("NegInvR2");
    glUniform1f(pos, -1.0f / (AO_RADIUS*AO_RADIUS));
    pos = hbaoFullShader.getUniformLocation("MaxRadiusPixels");
    glUniform1f(pos, AO_MAX_RADIUS_PIXELS);

    pos = hbaoFullShader.getUniformLocation("NoiseScale");
    glUniform2f(pos, (float)WIDTH/(float)NOISE_RES, (float)HEIGHT/(float)NOISE_RES);
    pos = hbaoFullShader.getUniformLocation("NumDirections");
    glUniform1i(pos, AO_DIRS);
    pos = hbaoFullShader.getUniformLocation("NumSamples");
    glUniform1i(pos, AO_SAMPLES);

    blurXShader.bind();
    pos = blurXShader.getUniformLocation("AORes");
    glUniform2f(pos, AO_WIDTH, AO_HEIGHT);
    pos = blurXShader.getUniformLocation("InvAORes");
    glUniform2f(pos, 1.0f/AO_WIDTH, 1.0f/AO_HEIGHT);
    pos = blurXShader.getUniformLocation("FullRes");
    glUniform2f(pos, WIDTH, HEIGHT);
    pos = blurXShader.getUniformLocation("InvFullRes");
    glUniform2f(pos, 1.0f/WIDTH, 1.0f/HEIGHT);
    pos = blurXShader.getUniformLocation("LinMAD");
    glUniform2f(pos, LinMAD[0], LinMAD[1]);

    blurYShader.bind();
    pos = blurYShader.getUniformLocation("AORes");
    glUniform2f(pos, AO_WIDTH, AO_HEIGHT);
    pos = blurYShader.getUniformLocation("InvAORes");
    glUniform2f(pos, 1.0f/AO_WIDTH, 1.0f/AO_HEIGHT);
    pos = blurYShader.getUniformLocation("FullRes");
    glUniform2f(pos, WIDTH, HEIGHT);
    pos = blurYShader.getUniformLocation("InvFullRes");
    glUniform2f(pos, 1.0f/WIDTH, 1.0f/HEIGHT);
    pos = blurYShader.getUniformLocation("LinMAD");
    glUniform2f(pos, LinMAD[0], LinMAD[1]);

    downsampleShader.bind();
    pos = downsampleShader.getUniformLocation("LinMAD");
    glUniform2f(pos, LinMAD[0], LinMAD[1]);
    pos = downsampleShader.getUniformLocation("ResRatio");
    glUniform1i(pos, RES_RATIO);

    upsampleShader.bind();
    pos = upsampleShader.getUniformLocation("LinMAD");
    glUniform2f(pos, LinMAD[0], LinMAD[1]);

    GLuint noiseTexture = generateNoiseTexture(NOISE_RES, NOISE_RES);

    float dt;

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while (!window.should_close())
    {
        window.new_frame();

        //===============================================================================================================================================================================================================
        // calculate FPS
        //===============================================================================================================================================================================================================

        static float t0 = 0.0;
        static float t1 = 0.0;
        static char title[256];
        static int frames = 0;

        float t = (float)glfwGetTime();

        dt = t - t0;
        t0 = t;
        t1 += dt;

        if(t1 > 0.25)
        {
            float fps = (float)frames / t1;

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

            sprintf(title, "Fullres: %i, FPS: %2.1f, time(ms): geom %2.2f, down %2.2f, ao %2.2f, up %2.2f, blur %2.2f, comp %2.2f tot %2.2f",
                (int)window.fullres, fps, geom, down, ao, up, blur, comp, tot);

            window.set_title(title);
            t1 = 0.0;
            frames = 0;
        }

        ++frames;        

        glGenQueries(7, queryID);
        glQueryCounter(queryID[0], GL_TIMESTAMP);

        //===============================================================================================================================================================================================================
        // RENDER GEOMETRY PASS
        //===============================================================================================================================================================================================================
        glEnable(GL_DEPTH_TEST);

        fboFullRes->bind();
        glClearColor(0.2, 0.3, 0.5, 1.0);
        glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        geometryShader.bind();

        glUniformMatrix4fv(geometryShader.getViewMatrixLocation(), 1, false, glm::value_ptr(window.camera.view_matrix));
        glUniformMatrix4fv(geometryShader.getProjMatrixLocation(), 1, false, glm::value_ptr(projection_matrix));

        mdl->draw();

        glQueryCounter(queryID[1], GL_TIMESTAMP);

        glDisable(GL_DEPTH_TEST);

        if(!window.fullres)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_DEPTH));
            fboHalfRes->bind();                                                       // Downsample depth
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            downsampleShader.bind();
            fsquad->draw();
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
            glBindTexture(GL_TEXTURE_2D, fboHalfRes->getBufferHandle(FBO_AUX0));
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            hbaoHalfShader.bind();
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_DEPTH));
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            hbaoFullShader.bind();
        }
        fsquad->draw();
    
        glQueryCounter(queryID[3], GL_TIMESTAMP);
    
        //===============================================================================================================================================================================================================
        // Upsample
        //===============================================================================================================================================================================================================    
        if(!window.fullres)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_DEPTH));
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, fboHalfRes->getBufferHandle(FBO_AUX1));
            fboFullRes->bind();
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            upsampleShader.bind();
            fsquad->draw();
        }
    
        glQueryCounter(queryID[4], GL_TIMESTAMP);
    
        //===============================================================================================================================================================================================================
        // Blur pass
        //===============================================================================================================================================================================================================    
        if(window.blur)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX1));
    
            glDrawBuffer(GL_COLOR_ATTACHMENT2);
            blurXShader.bind();
            fsquad->draw();
    
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            blurYShader.bind();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX2));
            fsquad->draw();
        }
    
        glQueryCounter(queryID[5], GL_TIMESTAMP);    
        fboFullRes->unbind();
    
        //===============================================================================================================================================================================================================
        // RENDER TO SCREEN PASS
        //===============================================================================================================================================================================================================
    
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX0));    
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX1));
    
        glViewport(0, 0, WIDTH, HEIGHT);
    
        compositShader.bind();
        fsquad->draw();
        
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
    delete fsquad;
    delete surface0;
    delete fboFullRes;
    delete fboHalfRes;

    for(std::map<std::string, Surface*>::iterator it = surfaces.begin(); it != surfaces.end(); ++it)
        delete it->second;

    glDeleteTextures(1, &noiseTexture);
    Surface::cleanUp();

    glfw::terminate();
    return 0;
}