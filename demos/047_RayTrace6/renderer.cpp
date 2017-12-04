// Main application class implementation
#include <sys/time.h>


// Includes autres
#include "log.hpp"
#include "renderer.hpp"
#include "helper.hpp"
#include "defines.hpp"


// Fonction qui récupère l'heure système
double getTime()
{
    timeval tv;
    gettimeofday (&tv, NULL);
    return double (tv.tv_sec) + 0.000001 * tv.tv_usec;
}

// Fonction d'error callback
static void error_callback(int error, const char* description)
{
    debug_color_msg(DEBUG_RED_COLOR, "GLFW Error #%d : %s", error, description);
}

// Fonction de key callback
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    Renderer::Instance().HandleKey(key, action);
}
// Fonction de cursor callback
static void cursor_callback(GLFWwindow *window, double xpos, double ypos)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float ratio = (float) width / (float) height;
    float x = ratio*(2*xpos/(float)width - 1);
    float y = 2*-ypos/(float)height + 1;
    Renderer::Instance().HandleMouse(x,-y);
}

// Constructeur du renderer
Renderer::Renderer()
: FIsRendering(false)
, FVertexArrayID(0)
, FVertexbuffer(0)
, FInitDone(false)
, FLastTime(-1)
, FForward(false)
, FBackward(false)
, FLeftward(false)
, FRightward(false)
, FOreille(false)
{
    
}
// Destructeur du renderer
Renderer::~Renderer()
{
    
}

void Renderer::HandleMouse(float parX, float parY)
{
    // Si l'init est fait
    if(FInitDone)
    {
        // Calcul des déplacements relatifs
        float xMove = parX - FOldX;
        float yMove = parY - FOldY;
        FOldX = parX;
        FOldY = parY;
        // Rotations
        FCamera.Yaw(xMove);
        FCamera.Pitch(yMove);
        FCamera.UpdateValues(FComputeShader);
    }
    else
    {
        // Première étape
        // Sauvegarde des position initiales
        FOldX = parX;
        FOldY = parY;
        FInitDone = true;
    }
}

// Prise en compte des entrées clavier
void Renderer::HandleKey(int parKey, int parAction)
{
    if (parAction == GLFW_PRESS)
    {
        if (parKey == GLFW_KEY_A) FLeftward  = true;
        if (parKey == GLFW_KEY_D) FRightward = true;
        if (parKey == GLFW_KEY_W) FForward    = true;
        if (parKey == GLFW_KEY_S) FBackward  = true;
        return;
    }

    if (parKey == GLFW_KEY_A) FLeftward  = false;
    if (parKey == GLFW_KEY_D) FRightward = false;
    if (parKey == GLFW_KEY_W) FForward   = false;
    if (parKey == GLFW_KEY_S) FBackward  = false;
}

bool Renderer::Init(const char* scene, bool parOreille)
{
    FOreille = parOreille;
    debug_color_msg(DEBUG_ORANGE_COLOR, "Ear model used: %u", FOreille ? 1 : 0);
    if(!glfwInit())
    {
        debug_color_msg(DEBUG_RED_COLOR, "GLFW initialization failed.");
        return false;
    }
    else
    {
        debug_color_msg(DEBUG_GREEN_COLOR, "GLFW successfully initialized");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

    // Open a window and create its OpenGL context
    FWindow = glfwCreateWindow(1280, 720, "Compute Shader Ray Tracer", 0, 0);
    if(!FWindow)
    {
        debug_color_msg(DEBUG_RED_COLOR, "GLFW window creation failed");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(FWindow);
    glfwSetErrorCallback(error_callback);
    glfwSetKeyCallback(FWindow, key_callback);
    glfwSetCursorPosCallback(FWindow, cursor_callback);
    
    glewExperimental = GL_TRUE;
    GLenum glewReturn = glewInit();
    if(glewReturn)
    {
        debug_color_msg(DEBUG_RED_COLOR, "GLEW initialization failed: %s", glewGetErrorString(glewReturn));
        return false;
    }
    
    debug_color_msg(DEBUG_ORANGE_COLOR, "Renderer: %s", glGetString (GL_RENDERER));
    debug_color_msg(DEBUG_ORANGE_COLOR, "GL Version: %s", glGetString (GL_VERSION));
    
    // Everything went ok let's render
    FIsRendering = true;
    CheckGLState("Vidage buffer");
    
    // Loading the scene file
    LoadScene(scene);
    
    // Creation de l'octree
    octree = new octree_t(FScene);
    
    // Initialisation des shaders
    InitShaders();
    
    // Creating the render to quad
    CreateRenderQuad();

    // Injecting the scene to the shader
    InjectScene();
    
    debug_color_msg(DEBUG_GREEN_COLOR, "The renderer has been successfully created.");
    
    FLastTime = glfwGetTime();
    glfwSetInputMode(FWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    return true;
}


void Renderer::CreateRenderQuad()
{
    glGenVertexArrays (1, &FVertexArrayID);
    glBindVertexArray (FVertexArrayID);
    
    glGenBuffers(1, &FVertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, FVertexbuffer);

    const GLfloat mainQuadArray[] = 
    {
        -1.0f, -1.0f,  0.0,   0.0, 0.0,
        -1.0f,  1.0f,  0.0,   0.0, 1.0,
         1.0f, -1.0f,  0.0,   1.0, 0.0,
         1.0f,  1.0f,  0.0,   1.0, 1.0
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(mainQuadArray), mainQuadArray, GL_STATIC_DRAW);
    GLuint posAtt = glGetAttribLocation(FPipelineShaderID, "Vertex_Pos");
    GLuint texAtt = glGetAttribLocation(FPipelineShaderID, "Vertex_TexCoord");
    glEnableVertexAttribArray (posAtt);
    glEnableVertexAttribArray (texAtt);
    glVertexAttribPointer (posAtt, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const GLvoid *) 0);
    glVertexAttribPointer (texAtt, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const GLvoid *) 12);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray (0);
}

void Renderer::InitShaders()
{

    //Création du shader de la pipline fixe
    FPipelineShaderID = ShaderManager::Instance().CreateProgramVF();

    #ifndef SIMPLE
    //Création du shader de calcul
    FComputeShader = ShaderManager::Instance().CreateProgramC(3, FScene->m_triangles.size(), 1, FScene->m_quadrics.size(), octree->m_nodes.size(), FScene->m_primitives.size(), FScene->m_materiaux.size(), FOreille);
    // Texture creation
    FRenderTexture = ShaderManager::Instance().GenerateTexture(512, 512);
    FTriangleTex = ShaderManager::Instance().CreateTexTriangle(FScene->m_triangles);
    FQuadTex = ShaderManager::Instance().CreateTexQuad(FScene->m_quadrics);
    FPrimitiveTex = ShaderManager::Instance().CreateTexPrimitive(FScene->m_primitives, FScene->m_materiaux.size());
    FMateriauTex = ShaderManager::Instance().CreateTexMat(FScene->m_materiaux);
    #endif
    // Creation de la texture des noeuds de l'octree, mit en commentaire car n'a pas été debuggé
    //FNoeudTex = ShaderManager::Instance().CreateTexNoeud(octree->m_nodes, octree->m_nb_prim_max);

    //Mappage de la texture pour dessin
    ShaderManager::Instance().InjectTex(FPipelineShaderID,FRenderTexture,"bling",0);


    #ifndef SIMPLE
    //Mappage de la texture pour écriture
    ShaderManager::Instance().InjectTex(FComputeShader,FRenderTexture,"renderCanvas",0);
    // Mappage des autres texture pour lecture
    ShaderManager::Instance().InjectTex(FComputeShader,FTriangleTex,"listTriangles",1);
    ShaderManager::Instance().InjectTex(FComputeShader,FPrimitiveTex,"listPrimitives",2);
    ShaderManager::Instance().InjectTex(FComputeShader,FMateriauTex,"listMateriaux",3);
    ShaderManager::Instance().InjectTex(FComputeShader,FQuadTex,"listQuadriques",4);
    // Si oreille définie, on injecte les textures de l'oreille
    
    if(FOreille)
    {
        ShaderManager::Instance().InjectTex(FComputeShader,FEarModel->albTex->id,"listTex[0]",5);
        ShaderManager::Instance().InjectTex(FComputeShader,FEarModel->rugTex->id,"listTex[1]",6);
        ShaderManager::Instance().InjectTex(FComputeShader,FEarModel->specTex->id,"listTex[2]",7);
        ShaderManager::Instance().InjectTex(FComputeShader,FEarModel->normalTex->id,"listTex[3]",8);
    }
    // Injection de la texture de l'octree ne fonctionne pas car n'a pas été debuggé
    //ShaderManager::Instance().InjectTex(FComputeShader,FNoeudTex,"listNoeuds",9);
    #endif
}

void Renderer::RayTracing()
{
    ShaderManager::Instance().BindProgram(FComputeShader);
    glDispatchCompute(512 / 16, 512 / 16, 1);
    ShaderManager::Instance().BindProgram(0);
}

void Renderer::RenderResultToScreen()
{
    ShaderManager::Instance().BindProgram(FPipelineShaderID);   // bind quad program
    glBindVertexArray (FVertexArrayID);                         // full screen quad
    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray (0);
    ShaderManager::Instance().BindProgram(0);                   // unbind program
}

void Renderer::UpdateDisplacement()
{
    if(FForward)   FCamera.Translate(glm::dvec3( 0.0, 0.0,-1.0));
    if(FBackward)  FCamera.Translate(glm::dvec3( 0.0, 0.0, 1.0));
    if(FLeftward)  FCamera.Translate(glm::dvec3( 1.0, 0.0, 0.0));
    if(FRightward) FCamera.Translate(glm::dvec3(-1.0, 0.0, 0.0));
    FCamera.UpdateValues(FComputeShader);       
}

void Renderer::Run()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);               // Update uniform camera parameters

    FCamera.UpdateValues(FComputeShader);
    while (!glfwWindowShouldClose (FWindow)) 
    {
//        double timeA = getTime();

        glClear (GL_COLOR_BUFFER_BIT);
        RayTracing();                               // Lancer de rayon
        RenderResultToScreen();                     // Rendu a l'ecran
        UpdateDisplacement();                       // Maj des deplacements
        glfwPollEvents ();
        glfwSwapBuffers (FWindow);
//        double timeB = getTime();
//        debug_color_msg(DEBUG_ORANGE_COLOR, "FPS = %f", 1.0 / (timeB - timeA));
    }
}


void Renderer::LoadScene(const std::string& parFilename)
{
    debug_color_msg(DEBUG_GREEN_COLOR, "Loading requested scene file: %s ...", parFilename.c_str());
    FScene = FParser.GetSceneFromFile(parFilename);
    if (!FScene)
        debug_color_msg(DEBUG_RED_COLOR, "Scene file %s not found", parFilename.c_str());
    else
        debug_color_msg(DEBUG_GREEN_COLOR, "Scene file %s found. Scene loaded...", parFilename.c_str());
    
    // L'oreille a été demandée
    if(FOreille)
    {
        // on la charge
        debug_color_msg(DEBUG_ORANGE_COLOR, "Loading ear model ... ");
        FEarModel = ResourceManager::Instance().LoadModel("model/ear.obj", "model/diff.bmp","model/rugo.bmp", "model/spec.bmp","model/normal.bmp");
        FScene->AddMateriau(FEarModel->material);
        foreach(triangle, FEarModel->listTriangle)
            { FScene->AddTriangle(*triangle); }     // uses the last material added to the scene
    }
    else
        debug_color_msg(DEBUG_ORANGE_COLOR, "Ear not used ...");
}

void Renderer::InjectScene()
{   
    int index = 0;      // Setting light uniforms
    foreach(light, FScene->m_lights)
    {
        ShaderManager::Instance().InjectLight(FComputeShader, *light, index++);
    }
}
