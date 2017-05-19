
#include "types.hpp"
#include "log1.hpp"
#include "geometry.hpp"
#include "obj1.hpp"
#include "shader1.hpp"
#include "camera1.hpp"
#include "surface1.hpp"
#include "model1.hpp"
#include "mtl.hpp"
#include "fbo2d.hpp"
#include "mesh2geometry.hpp"
#include "material2surface.hpp"
#include "gl_info.hpp"

#define WIDTH 1280
#define HEIGHT 720

#define RES_RATIO 2
#define AO_WIDTH (WIDTH/RES_RATIO)
#define AO_HEIGHT (HEIGHT/RES_RATIO)
#define AO_RADIUS 0.3
#define AO_DIRS 6
#define AO_SAMPLES 3
#define AO_STRENGTH 2.5;
#define AO_MAX_RADIUS_PIXELS 50.0

#define NOISE_RES 4

#define MOVE_SPEED 1.5
#define MOUSE_SPEED 1.5

void setupGL();
void calcFPS(float &dt);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void init();
void cleanUp();

void modifyCamera(float dt);
void generateNoiseTexture(int width, int height);

bool running = true;
bool blur = false;
bool fullres = true;

GLFWwindow* window;

Model * mdl;
Geometry* model;
Geometry* fsquad;
Camera * cam;

Shader* geometryShader;
Shader* geometryBackShader;
Shader* compositShader;
Shader* hbaoHalfShader;
Shader* hbaoFullShader;
Shader* blurXShader;
Shader* blurYShader;
Shader* downsampleShader;
Shader* upsampleShader;
//Shader* ssaoShader;

Framebuffer2D * fboFullRes;
Framebuffer2D * fboHalfRes;

double timeStamps[7];
unsigned int queryID[7];

Surface * surface0;

std::map<std::string, Surface*> surfaces;
std::vector<Geometry> models;

GLuint noiseTexture;

int main()
{
	init();
    GLenum buffer[1];
    float dt;

    cam->translate(vec3(0, 0, 2));

	while(running)
	{
        calcFPS(dt);

        glGenQueries(7, queryID);
        glQueryCounter(queryID[0], GL_TIMESTAMP);

		modifyCamera(dt);
        cam->setup();

        // RENDER GEOMETRY PASS
        glEnable(GL_DEPTH_TEST);

        fboFullRes->bind();
        glClearColor(0.2, 0.3, 0.5, 1.0);
        glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        geometryShader->bind();

        glUniformMatrix4fv(geometryShader->getViewMatrixLocation(), 1, false, glm::value_ptr(cam->getViewMatrix()));
        glUniformMatrix4fv(geometryShader->getProjMatrixLocation(), 1, false, glm::value_ptr(cam->getProjMatrix()));

        mdl->draw();
        //model->draw();

        glQueryCounter(queryID[1], GL_TIMESTAMP);

        glDisable(GL_DEPTH_TEST);

    //glColorMask(1, 0, 0, 0);
    //glDepthFunc(GL_ALWAYS);

        if(!fullres)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_DEPTH));
            fboHalfRes->bind();                                                       // Downsample depth
            buffer[0] = GL_COLOR_ATTACHMENT0;
            glDrawBuffers(1, buffer);
            downsampleShader->bind();
            fsquad->draw();
        }
    
        glQueryCounter(queryID[2], GL_TIMESTAMP);
    
        // AO pass    
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
    
        if(!fullres)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboHalfRes->getBufferHandle(FBO_AUX0));
            buffer[0] = GL_COLOR_ATTACHMENT1;
            glDrawBuffers(1, buffer);
            hbaoHalfShader->bind();
            //ssaoShader->bind();
            fsquad->draw();
        }
        else
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_DEPTH));
            buffer[0] = GL_COLOR_ATTACHMENT1;
            glDrawBuffers(1, buffer);
            hbaoFullShader->bind();
            fsquad->draw();
        }
    
        glQueryCounter(queryID[3], GL_TIMESTAMP);
    
        // Upsample
    
        if(!fullres)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_DEPTH));
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, fboHalfRes->getBufferHandle(FBO_AUX1));
            fboFullRes->bind();
            buffer[0] = GL_COLOR_ATTACHMENT1;
            glDrawBuffers(1, buffer);
            upsampleShader->bind();
            fsquad->draw();
        }
    
        glQueryCounter(queryID[4], GL_TIMESTAMP);
    
        if(blur)
        {
            // BLUR 
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX1));
    
            // X
            buffer[0] = GL_COLOR_ATTACHMENT2;
            glDrawBuffers(1, buffer);
            blurXShader->bind();
            fsquad->draw();
    
            // Y
            buffer[0] = GL_COLOR_ATTACHMENT1;
            glDrawBuffers(1, buffer);
            blurYShader->bind();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX2));
            fsquad->draw();
        }
    
        //timeStamps[5] = glfwGetTime();
        glQueryCounter(queryID[5], GL_TIMESTAMP);
    
        fboFullRes->unbind();
    
        // RENDER TO SCREEN PASS
    
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX0));
    
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX1));
    
        //glClearColor(1.0, 1.0, 1.0, 0.0);
        //glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, WIDTH, HEIGHT);
    
        compositShader->bind();
        fsquad->draw();
		glfwSwapBuffers(window);
        glfwPollEvents();
        
        if (glfwWindowShouldClose(window))
			running = false;

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
	}

	cleanUp();
	return 0;
}

void init()
{
	setupGL();
    Surface::init();

	cam = new Camera();
    mdl = new Model();

    Mesh mesh = loadMeshFromObj("mesh/dragon.obj", 0.1);
    Geometry dragon = createGeometryFromMesh(mesh);

    Surface *surface = new Surface();
    //surface->loadDiffuseTexture("resources/meshes/textures/sponza_floor_a_spec.tga");
    surfaces.insert(std::pair<std::string, Surface*> (std::string("default"), surface) );

    mdl->addGeometryAndSurface(&dragon, surface);

    // Geometry floor;

    // Geometry::sVertex v;
    // v.position = vec3(-1, 0,-1); v.texCoord = vec2(0,0); floor.addVertex(v);
    // v.position = vec3( 1, 0,-1); v.texCoord = vec2(1,0); floor.addVertex(v);
    // v.position = vec3( 1, 0, 1); v.texCoord = vec2(1,1); floor.addVertex(v);
    // v.position = vec3(-1, 0, 1); v.texCoord = vec2(0,1); floor.addVertex(v);

    // floor.addTriangle(uvec3(0,2,1));
    // floor.addTriangle(uvec3(0,3,2));

    // mdl->addGeometryAndSurface(&floor, surface);

    model = new Geometry();

    mesh = loadMeshFromObj("mesh/sponza.obj", 0.01f);
    *model = createGeometryFromMesh(mesh);
    model->createStaticBuffers();

    std::vector<Mesh> meshes = loadMeshesFromObj("mesh/sponza.obj", 0.01f);
    std::vector<Geometry> geometries = createGeometryFromMesh(meshes);

    std::vector<Material> materials = loadMaterialsFromMtl("mesh/sponza.mtl");
    surfaces = createSurfaceFromMaterial(materials, "mesh/");

    for(unsigned int i=0; i<geometries.size(); ++i)
    {
        mdl->addGeometryAndSurface(&geometries[i], surfaces[geometries[i].material]);
    }

    mdl->prepare();

    fsquad = new Geometry();

    Geometry::sVertex v;
    v.position = vec3(-1,-1, 0); v.texCoord = vec2(0,0); fsquad->addVertex(v);
    v.position = vec3( 1,-1, 0); v.texCoord = vec2(1,0); fsquad->addVertex(v);
    v.position = vec3( 1, 1, 0); v.texCoord = vec2(1,1); fsquad->addVertex(v);
    v.position = vec3(-1, 1, 0); v.texCoord = vec2(0,1); fsquad->addVertex(v);

    fsquad->addTriangle(uvec3(0, 1, 2));
    fsquad->addTriangle(uvec3(0, 2, 3));
    fsquad->createStaticBuffers();

	geometryShader = new Shader("glsl/geometry.vs", "glsl/geometry.fs");
    geometryBackShader = new Shader("glsl/geometry.vs", "glsl/geometry_back.fs");

    hbaoHalfShader = new Shader("glsl/fullscreen.vs", "glsl/hbao.fs");
    hbaoFullShader = new Shader("glsl/fullscreen.vs", "glsl/hbao_full.fs");
	compositShader = new Shader("glsl/fullscreen.vs", "glsl/composite.fs");
    blurXShader = new Shader("glsl/fullscreen.vs", "glsl/blur_x.fs");
    blurYShader = new Shader("glsl/fullscreen.vs", "glsl/blur_y.fs");
    downsampleShader = new Shader("glsl/fullscreen.vs", "glsl/downsample_depth.fs");
    upsampleShader = new Shader("glsl/fullscreen.vs", "glsl/upsample_aoz.fs");

    // Full res deferred base
    fboFullRes = new Framebuffer2D(WIDTH, HEIGHT);
    fboFullRes->attachBuffer(FBO_DEPTH, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    fboFullRes->attachBuffer(FBO_AUX0, GL_RGBA8, GL_RGBA, GL_FLOAT);
    fboFullRes->attachBuffer(FBO_AUX1, GL_RG16F, GL_RG, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    fboFullRes->attachBuffer(FBO_AUX2, GL_RG16F, GL_RG, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    //fboFullRes->attachBuffer(FBO_AUX3, GL_R8, GL_RED, GL_FLOAT);

    // Half res buffer for AO
    fboHalfRes = new Framebuffer2D(AO_WIDTH, AO_HEIGHT);
    //fboHalfRes->attachBuffer(FBO_DEPTH, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT);
    fboHalfRes->attachBuffer(FBO_AUX0, GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    fboHalfRes->attachBuffer(FBO_AUX1, GL_R8, GL_RED, GL_FLOAT, GL_LINEAR, GL_LINEAR);


    float fovRad = cam->getFov();

    vec2 FocalLen, InvFocalLen, UVToViewA, UVToViewB, LinMAD;

    FocalLen[0]      = 1.0f / tanf(fovRad * 0.5f) * ((float)AO_HEIGHT / (float)AO_WIDTH);
    FocalLen[1]      = 1.0f / tanf(fovRad * 0.5f);
    InvFocalLen[0]   = 1.0f / FocalLen[0];
    InvFocalLen[1]   = 1.0f / FocalLen[1];

    UVToViewA[0] = -2.0f * InvFocalLen[0];
    UVToViewA[1] = -2.0f * InvFocalLen[1];
    UVToViewB[0] =  1.0f * InvFocalLen[0];
    UVToViewB[1] =  1.0f * InvFocalLen[1];

    float near = cam->getNear(), far = cam->getFar();
    LinMAD[0] = (near-far)/(2.0f*near*far);
    LinMAD[1] = (near+far)/(2.0f*near*far);

    hbaoHalfShader->bind();
    int pos;
    pos = hbaoHalfShader->getUniformLocation("FocalLen");
    glUniform2f(pos, FocalLen[0], FocalLen[1]);
    pos = hbaoHalfShader->getUniformLocation("UVToViewA");
    glUniform2f(pos, UVToViewA[0], UVToViewA[1]);
    pos = hbaoHalfShader->getUniformLocation("UVToViewB");
    glUniform2f(pos, UVToViewB[0], UVToViewB[1]);
    pos = hbaoHalfShader->getUniformLocation("LinMAD");
    glUniform2f(pos, LinMAD[0], LinMAD[1]);

    pos = hbaoHalfShader->getUniformLocation("AORes");
    glUniform2f(pos, (float)AO_WIDTH, (float)AO_HEIGHT);
    pos = hbaoHalfShader->getUniformLocation("InvAORes");
    glUniform2f(pos, 1.0f/(float)AO_WIDTH, 1.0f/(float)AO_HEIGHT);

    pos = hbaoHalfShader->getUniformLocation("R");
    glUniform1f(pos, AO_RADIUS);
    pos = hbaoHalfShader->getUniformLocation("R2");
    glUniform1f(pos, AO_RADIUS*AO_RADIUS);
    pos = hbaoHalfShader->getUniformLocation("NegInvR2");
    glUniform1f(pos, -1.0f / (AO_RADIUS*AO_RADIUS));
    pos = hbaoHalfShader->getUniformLocation("MaxRadiusPixels");
    glUniform1f(pos, AO_MAX_RADIUS_PIXELS / (float)RES_RATIO);

    pos = hbaoHalfShader->getUniformLocation("NoiseScale");
    glUniform2f(pos, (float)AO_WIDTH/(float)NOISE_RES, (float)AO_HEIGHT/(float)NOISE_RES);
    pos = hbaoHalfShader->getUniformLocation("NumDirections");
    glUniform1i(pos, AO_DIRS);
    pos = hbaoHalfShader->getUniformLocation("NumSamples");
    glUniform1i(pos, AO_SAMPLES);

    hbaoFullShader->bind();
    pos = hbaoFullShader->getUniformLocation("FocalLen");
    glUniform2f(pos, FocalLen[0], FocalLen[1]);
    pos = hbaoFullShader->getUniformLocation("UVToViewA");
    glUniform2f(pos, UVToViewA[0], UVToViewA[1]);
    pos = hbaoFullShader->getUniformLocation("UVToViewB");
    glUniform2f(pos, UVToViewB[0], UVToViewB[1]);
    pos = hbaoFullShader->getUniformLocation("LinMAD");
    glUniform2f(pos, LinMAD[0], LinMAD[1]);

    pos = hbaoFullShader->getUniformLocation("AORes");
    glUniform2f(pos, (float)WIDTH, (float)HEIGHT);
    pos = hbaoFullShader->getUniformLocation("InvAORes");
    glUniform2f(pos, 1.0f/(float)WIDTH, 1.0f/(float)HEIGHT);

    pos = hbaoFullShader->getUniformLocation("R");
    glUniform1f(pos, AO_RADIUS);
    pos = hbaoFullShader->getUniformLocation("R2");
    glUniform1f(pos, AO_RADIUS*AO_RADIUS);
    pos = hbaoFullShader->getUniformLocation("NegInvR2");
    glUniform1f(pos, -1.0f / (AO_RADIUS*AO_RADIUS));
    pos = hbaoFullShader->getUniformLocation("MaxRadiusPixels");
    glUniform1f(pos, AO_MAX_RADIUS_PIXELS);

    pos = hbaoFullShader->getUniformLocation("NoiseScale");
    glUniform2f(pos, (float)WIDTH/(float)NOISE_RES, (float)HEIGHT/(float)NOISE_RES);
    pos = hbaoFullShader->getUniformLocation("NumDirections");
    glUniform1i(pos, AO_DIRS);
    pos = hbaoFullShader->getUniformLocation("NumSamples");
    glUniform1i(pos, AO_SAMPLES);

    blurXShader->bind();
    pos = blurXShader->getUniformLocation("AORes");
    glUniform2f(pos, AO_WIDTH, AO_HEIGHT);
    pos = blurXShader->getUniformLocation("InvAORes");
    glUniform2f(pos, 1.0f/AO_WIDTH, 1.0f/AO_HEIGHT);
    pos = blurXShader->getUniformLocation("FullRes");
    glUniform2f(pos, WIDTH, HEIGHT);
    pos = blurXShader->getUniformLocation("InvFullRes");
    glUniform2f(pos, 1.0f/WIDTH, 1.0f/HEIGHT);
    pos = blurXShader->getUniformLocation("LinMAD");
    glUniform2f(pos, LinMAD[0], LinMAD[1]);

    blurYShader->bind();
    pos = blurYShader->getUniformLocation("AORes");
    glUniform2f(pos, AO_WIDTH, AO_HEIGHT);
    pos = blurYShader->getUniformLocation("InvAORes");
    glUniform2f(pos, 1.0f/AO_WIDTH, 1.0f/AO_HEIGHT);
    pos = blurYShader->getUniformLocation("FullRes");
    glUniform2f(pos, WIDTH, HEIGHT);
    pos = blurYShader->getUniformLocation("InvFullRes");
    glUniform2f(pos, 1.0f/WIDTH, 1.0f/HEIGHT);
    pos = blurYShader->getUniformLocation("LinMAD");
    glUniform2f(pos, LinMAD[0], LinMAD[1]);

    downsampleShader->bind();
    pos = downsampleShader->getUniformLocation("LinMAD");
    glUniform2f(pos, LinMAD[0], LinMAD[1]);
    pos = downsampleShader->getUniformLocation("ResRatio");
    glUniform1i(pos, RES_RATIO);

    upsampleShader->bind();
    pos = upsampleShader->getUniformLocation("LinMAD");
    glUniform2f(pos, LinMAD[0], LinMAD[1]);

    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    generateNoiseTexture(NOISE_RES, NOISE_RES);
}

void cleanUp()
{
	delete model;
    delete fsquad;
    delete surface0;
	delete cam;
	delete geometryShader;
	delete hbaoHalfShader;
    delete hbaoFullShader;
    delete compositShader;
    delete blurXShader;
    delete blurYShader;
    delete downsampleShader;
    delete fboFullRes;
    delete fboHalfRes;

    for(std::map<std::string, Surface*>::iterator it = surfaces.begin(); it != surfaces.end(); ++it)
        delete it->second;

    glDeleteTextures(1, &noiseTexture);
    Surface::cleanUp();
}

void setupGL()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "HBAO", 0, 0);
    if (!window) 
    {
        glfwTerminate();
        exit_msg("Failed to create window");
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

	logErrorsGL();

	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit())
    {
        glfwTerminate();
        exit_msg("GLEW init error");
    }

	logErrorsGL();

    gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
	glfwSwapInterval(0);
    glActiveTexture(GL_TEXTURE0);
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetKeyCallback(window, keyCallback);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key == GLFW_KEY_ESCAPE)
        running = false;
    else if(key == GLFW_KEY_B && action == GLFW_PRESS)
        blur = !blur;
    else if(key == GLFW_KEY_F && action == GLFW_PRESS)
        fullres = !fullres;
}

void calcFPS(float &dt)
{
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
            (int)fullres, fps, geom, down, ao, up, blur, comp, tot);

        glfwSetWindowTitle(window, title);
        t1 = 0.0;
        frames = 0;
    }
    ++frames;
}

void modifyCamera(float dt)
{
    double x,y;

    glfwGetCursorPos(window, &x, &y);

    vec3 camrot = cam->getOrientation();

    camrot.x -= (float)(y - HEIGHT / 2) * MOUSE_SPEED * dt;
    camrot.y -= (float)(x - WIDTH / 2)  * MOUSE_SPEED * dt;
    
    if(camrot.x > 90.0f) camrot.x = 90.0f;
    if(camrot.x < -90.0f) camrot.x = -90.0f;
    if(camrot.y > 360.0f) camrot.y -= 360.0f;
    if(camrot.y < -360.0f) camrot.y += 360.0f;
    
    cam->setOrientation(camrot);
    glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);

    glm::vec3 movevec = glm::vec3(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W)) movevec.z += MOVE_SPEED;
    if (glfwGetKey(window, GLFW_KEY_S)) movevec.z -= MOVE_SPEED;
    if (glfwGetKey(window, GLFW_KEY_A)) movevec.x -= MOVE_SPEED;
    if (glfwGetKey(window, GLFW_KEY_D)) movevec.x += MOVE_SPEED;

    cam->move(movevec * dt);
}

void generateNoiseTexture(int width, int height)
{
    float *noise = new float[width * height * 4];
    for(int y = 0; y < height; ++y)
    {
        for(int x = 0; x < width; ++x)
        {
            vec2 xy = glm::circularRand(1.0f);
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
}
