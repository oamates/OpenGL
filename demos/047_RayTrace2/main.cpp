#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "camera.hpp"
#include "matrix44.hpp"
#include "vector4.hpp"


struct GLFWwindow;

struct Application
{
    static Application* s_instance;

    Application(const Application&);
    Application& operator = (const Application&);

    Application() : m_window(0) {}

    ~Application() {}

    static Application* GetInstance();

    void Render(double currentTime);
    void OnResize(int w, int h);
    void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods);
    void OnMouseButton(int button, int action);
    void OnMouseMove(GLFWwindow*, double x, double y);
    void OnMouseWheel(int pos);
    void GetMousePosition(int& x, int& y);

    struct APPINFO
    {
        char title[128];
        int windowWidth;
        int windowHeight;
        int majorVersion;
        int minorVersion;
        int samples;
        union
        {
            struct
            {
                unsigned int fullscreen : 1;
                unsigned int vsync : 1;
                unsigned int cursor : 1;
                unsigned int stereo : 1;
                unsigned int debug : 1;
            };
            unsigned int all;
        } flags;
    };

    void Init();
    GLuint LoadShaderFromFile(std::string path, GLenum shaderType);
    void Trace();
    void DoInput();
    GLuint QuadFullScreenVao();
    GLuint CreateComputeProgram();
    void InitComputeProgram();
    GLuint CreateQuadProgram();
    void InitQuadProgram();
    GLuint CreateFramebufferTexture();


    APPINFO         m_info;
    GLFWwindow*   m_window;

    GLint m_workGroupSizeX;
    GLint m_workGroupSizeY;

    GLuint m_vao;
    GLuint m_tex;
    GLuint m_computeProgram;
    GLuint m_quadProgram;

    GLuint m_eyeUniform;
    GLuint m_ray00Uniform;
    GLuint m_ray10Uniform;
    GLuint m_ray01Uniform;
    GLuint m_ray11Uniform;
    
    double m_mouseX;
    double m_mouseY; 
    double m_prevX; 
    double m_prevY;
    
    Camera m_camera;
};  

typedef Dg::Vector4<float> vec4;
typedef Dg::Matrix44<float> mat4;

Application* Application::s_instance = 0;

static void OnKeyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{
    Application::GetInstance()->OnKey(a_window, a_key, a_scancode, a_action, a_mods);
}

static void OnMouseMoveCallback(GLFWwindow* a_window, double a_x, double a_y)
{
    Application::GetInstance()->OnMouseMove(a_window, a_x, a_y);
}

Application* Application::GetInstance()
{
    if (!s_instance)
        s_instance = new Application();
    return s_instance;
}

// Returns the ID of a compiled shader of the specified type from the specified file 
// Reports error to console if file could not be found or compiled
GLuint Application::LoadShaderFromFile(std::string a_path, GLenum a_shaderType)
{
    GLuint shaderID = 0;
    std::string shaderString;
    std::ifstream sourceFile(a_path.c_str());                   // Open file

    if (sourceFile)                                             // Source file loaded, get shader source
    {
        shaderString.assign(std::istreambuf_iterator<char> (sourceFile), std::istreambuf_iterator<char>());
        shaderID = glCreateShader(a_shaderType);                // Create shader ID
        const GLchar* shaderSource = shaderString.c_str();      // Set shader source
        glShaderSource(shaderID, 1, (const GLchar**) &shaderSource, 0);
        glCompileShader(shaderID);                              // Compile shader source

        GLint shaderCompiled = GL_FALSE;                        // Check shader for errors
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);
        if (shaderCompiled != GL_TRUE)
        {
            printf("Unable to compile shader %d!\n\nSource:\n%s\n", shaderID, shaderSource);
            glDeleteShader(shaderID);
            shaderID = 0;
        }
    }
    else
        printf("Unable to open file %s\n", a_path.c_str());

    return shaderID;
}

GLuint Application::QuadFullScreenVao()
{
    glGenVertexArrays(1, &m_vao);
    GLuint vbo(0);
    glGenBuffers(1, &vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    float screenVertices[12] = {-1.0f, -1.0f,
                                 1.0f, -1.0f,
                                 1.0f,  1.0f, 
                                 1.0f,  1.0f, 
                                -1.0f,  1.0f, 
                                -1.0f, -1.0f };

    glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), screenVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    return m_vao;
}

GLuint Application::CreateQuadProgram()
{
    GLuint quadProgram = glCreateProgram();
    GLuint vshader = LoadShaderFromFile("glsl/quad.vs", GL_VERTEX_SHADER);
    GLuint fshader = LoadShaderFromFile("glsl/quad.fs", GL_FRAGMENT_SHADER);
    glAttachShader(quadProgram, vshader);
    glAttachShader(quadProgram, fshader);
    glBindAttribLocation(quadProgram, 0, "vertex");
    glBindFragDataLocation(quadProgram, 0, "color");
    glLinkProgram(quadProgram);
    GLint linked;
    glGetProgramiv(quadProgram, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLchar buf[2048] = {};
        GLsizei length;
        glGetProgramInfoLog(quadProgram, 2048, &length, buf);
        printf("%s", buf);
    }
    return quadProgram;
}

void Application::InitQuadProgram()
{
    glUseProgram(m_quadProgram);
    GLint texUniform = glGetUniformLocation(m_quadProgram, "tex");
    glUniform1i(texUniform, 0);
    glUseProgram(0);
}


GLuint Application::CreateComputeProgram()
{
    GLuint computeProgram = glCreateProgram();
    GLuint cshader = LoadShaderFromFile("glsl/raytracer_cs.glsl", GL_COMPUTE_SHADER);
    glAttachShader(computeProgram, cshader);
    glLinkProgram(computeProgram);
    GLint linked;
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLchar buf[2048] = {};
        GLsizei length;
        glGetProgramInfoLog(computeProgram, 2048, &length, buf);
        printf("%s", buf);
    }
    return computeProgram;
}


void Application::InitComputeProgram()
{
    glUseProgram(m_computeProgram);
    GLint workGroupSize[3] = {};
    glGetProgramiv(m_computeProgram, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
    m_workGroupSizeX = workGroupSize[0];
    m_workGroupSizeY = workGroupSize[1];
    m_eyeUniform = glGetUniformLocation(m_computeProgram, "eye");
    m_ray00Uniform = glGetUniformLocation(m_computeProgram, "ray00");
    m_ray10Uniform = glGetUniformLocation(m_computeProgram, "ray10");
    m_ray01Uniform = glGetUniformLocation(m_computeProgram, "ray01");
    m_ray11Uniform = glGetUniformLocation(m_computeProgram, "ray11");
    glUseProgram(0);
}


void Application::Init()
{
    // Init defaults
    strcpy(m_info.title, "Raytracer example");
    m_info.windowWidth = 800;
    m_info.windowHeight = 600;
    m_info.majorVersion = 4;
    m_info.minorVersion = 4;
    m_info.samples = 0;
    m_info.flags.all = 0;
    m_info.flags.cursor = 1;
    m_info.flags.fullscreen = 0;

    m_mouseX = 0.0;
    m_mouseY = 0.0;
    m_prevX = 0.0;
    m_prevY = 0.0;
  
    m_window = nullptr;

    // Init GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return;
    }

    // Init Window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, m_info.majorVersion);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, m_info.minorVersion);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, m_info.samples);
    glfwWindowHint(GLFW_STEREO, m_info.flags.stereo ? GL_TRUE : GL_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    if (m_info.flags.fullscreen)
    {
        if (m_info.windowWidth == 0 || m_info.windowHeight == 0)
        {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            m_info.windowWidth = mode->width;
            m_info.windowHeight = mode->height;
        }
        m_window = glfwCreateWindow(m_info.windowWidth, m_info.windowHeight, m_info.title, glfwGetPrimaryMonitor(), 0);
        glfwSwapInterval((int)m_info.flags.vsync);
    }
    else
        m_window = glfwCreateWindow(m_info.windowWidth, m_info.windowHeight, m_info.title, 0, 0);

    if (!m_window)
    {
        fprintf(stderr, "Failed to open m_window\n");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    // Set up the camera
    m_camera.SetScreen((float(m_info.windowWidth) / float(m_info.windowHeight)), 1.0f);

    // Set up the mouse
    glfwSetCursorPos(m_window, 0.0, 0.0);
    m_mouseX = 0.0;
    m_mouseY = 0.0;
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set up keys
    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);


    glfwSetKeyCallback(m_window, ::OnKeyCallback);
    glfwSetCursorPosCallback(m_window, ::OnMouseMoveCallback);
    m_info.flags.stereo = (glfwGetWindowAttrib(m_window, GLFW_STEREO) ? 1 : 0);

    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();

    // get version m_info
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

    // Create all needed GL resources
    m_tex = CreateFramebufferTexture();
    m_vao = QuadFullScreenVao();
    m_computeProgram = CreateComputeProgram();
    InitComputeProgram();
    m_quadProgram = CreateQuadProgram();
    InitQuadProgram();
}


GLuint Application::CreateFramebufferTexture()
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GLchar * black(nullptr);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_info.windowWidth, m_info.windowHeight, 0, GL_RGBA, GL_FLOAT, black);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

void Application::OnKey(GLFWwindow* m_window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, GL_TRUE);
}

void Application::OnMouseMove(GLFWwindow* m_window, double x, double y)
{
    double dx = x - m_mouseX;
    double dy = y - m_mouseY;
    m_camera.UpdateYPR(-0.01 * dx, -0.01 * dy, 0.0);
    m_mouseX = x;
    m_mouseY = y;
}


void Application::DoInput()
{
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) m_camera.MoveForward( 0.1);
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) m_camera.MoveForward(-0.1);
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) m_camera.MoveLeft( 0.1);
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) m_camera.MoveLeft(-0.1);
    if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS) m_camera.MoveWorldUp( 0.1);
    if (glfwGetKey(m_window, GLFW_KEY_F) == GLFW_PRESS) m_camera.MoveWorldUp(-0.1);
}

void Application::Trace()
{
    glUseProgram(m_computeProgram);

    vec4 ray00, ray01, ray10, ray11, eye;
    m_camera.GetCornerRays(ray00, ray01, ray10, ray11, eye);
    glUniform3f(m_eyeUniform, eye[0], eye[1], eye[2]);
    glUniform3f(m_ray00Uniform, ray00[0], ray00[1], ray00[2]);
    glUniform3f(m_ray01Uniform, ray01[0], ray01[1], ray01[2]);
    glUniform3f(m_ray10Uniform, ray10[0], ray10[1], ray10[2]);
    glUniform3f(m_ray11Uniform, ray11[0], ray11[1], ray11[2]);

    // Bind level 0 of framebuffer texture as writable image in the shader.
    glBindImageTexture(0, m_tex, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Compute appropriate invocation dimension.
    int worksizeX = Dg::NextPower2(m_info.windowWidth);
    int worksizeY = Dg::NextPower2(m_info.windowHeight);

    /* Invoke the compute shader. */
    glDispatchCompute(worksizeX / m_workGroupSizeX, worksizeY / m_workGroupSizeY, 1);

    /* Reset image binding. */
    glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glUseProgram(0);

    // Draw the rendered image on the screen using textured full-screen quad.
    glUseProgram(m_quadProgram);
    glBindVertexArray(m_vao);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

int main()
{
    Application& application = *(Application::GetInstance());
    application.Init();

    while (glfwWindowShouldClose(application.m_window) == GL_FALSE) 
    {
        glfwPollEvents();
        glViewport(0, 0, application.m_info.windowWidth, application.m_info.windowHeight);

        application.DoInput();
        application.Trace();

        glfwSwapBuffers(application.m_window);
    }

    glfwDestroyWindow(application.m_window);
    glfwTerminate();
    return 0;
}