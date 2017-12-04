// Main application class

#ifndef CSR_RENDERER
#define CSR_RENDERER

#include "parser.hpp"
#include "shadermanager.hpp"
#include "camera.hpp"
#include "defines.hpp"
#include "singleton.hpp"
#include "octree.hpp"
#include "resourcemanager.hpp"

struct Renderer : public singleton_t <Renderer>
{
    Renderer();
    ~Renderer();

    bool Init(const char* scene, bool parOreille);
    void InitShaders();

    void Run();

    void RayTracing();
    void RenderResultToScreen();
    GLuint GetComputeProgID() { return FComputeShader; }

    void HandleKey(int parKey, int parAction);
    void HandleMouse(float parX, float parY);

    void CreateRenderQuad();
    void LoadScene(const std::string& parFilename);
    void InjectScene();
    void UpdateDisplacement();
        
    bool FIsRendering;
    GLFWwindow* FWindow;
    float FOldX;
    float FOldY;
    bool FInitDone;
        
    bool FOreille;
        
    double FLastTime;
    ShaderManager FManager;
    Camera FCamera;

    // Loading scene
    Parser FParser;
    Scene* FScene;
        
    // Octree;
    octree_t* octree;
    ObjFile * FEarModel;
    
    // RenderToQuad
    GLuint FVertexArrayID;
    GLuint FVertexbuffer;
    GLuint FVertexCoord;
    GLuint FPipelineShaderID;

    GLuint FComputeShader;
    GLuint FRenderTexture;
    GLuint FTriangleTex;
    GLuint FQuadTex;
    GLuint FPrimitiveTex;
    GLuint FMateriauTex;
    
    GLuint FNoeudTex;
        
    bool FFoward;
    bool FBackward;
    bool FLeftward;
    bool FRightward;
                        
};

#endif // CSR_RENDERER
