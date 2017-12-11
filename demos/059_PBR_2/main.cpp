#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/ext.hpp>

#include <memory>

#include "timer.hpp"
#include "scene.hpp"
#include "sphere.hpp"
#include "phongmaterial.hpp"
#include "earth.hpp"
#include "moon.hpp"
#include "axis.hpp"
#include "animator.hpp"

#include "glfw_window.hpp"
#include "log.hpp"
#include "gl_aux.hpp"

static int  s_WindowWidth  = 800;
static int  s_WindowHeight = 600;
static bool s_bEnableVSync = true;
static bool s_bWireframe   = false;
static bool s_bEarthScene = true;
static bool s_bFullScreen = false;

static std::shared_ptr<Scene> scene = std::make_shared<Scene>();

void draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene->draw();
}

void key(GLFWwindow* /*window*/, int key, int /*s*/, int action, int /*mods*/)
{
	scene->camera()->keyEvent(key, action);

	if (action == GLFW_RELEASE)
	{
		switch (key)
		{
		case GLFW_KEY_L:
			//scene->toggleLightAnimation();
			break;
		case GLFW_KEY_K:
			//scene->toggleLightDirection();
			break;
		case GLFW_KEY_O:
			s_bWireframe = !s_bWireframe;
			glPolygonMode(GL_FRONT_AND_BACK, s_bWireframe ? GL_LINE : GL_FILL);			
			std::cout << "Wireframe: " << s_bWireframe << std::endl;
			break;		
		default:
			break;
		}		
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/)
{
	if (button != GLFW_MOUSE_BUTTON_LEFT)
    {
		return;
    }

	if (action == GLFW_PRESS)
	{
        double cursorX, cursorY;
        
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwGetCursorPos(window, &cursorX, &cursorY);
        
        scene->camera()->mouseButtonEvent(cursorX, cursorY);
	}
	else
    {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void mouseMotionCallback(GLFWwindow* window, double x, double y)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
	{
        scene->camera()->mouseMotionEvent(x, y);
    }
}

void mouseScrollCallback(GLFWwindow* /*window*/, double x, double y)
{
    scene->camera()->mouseScrollEvent(x, y);
}

void reshape(GLFWwindow* /*window*/, int width, int height)
{
	if (width > 0 && height > 0)
	{
		scene->camera()->setViewportSize(width, height);
		glViewport(0, 0, (GLint)width, (GLint)height);
	}    
}

static void createScene()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);

	auto phongMaterial = std::make_shared<PhongMaterial>();
	auto sphereMesh = std::make_shared<SphereMesh>(200);

	auto earth = std::make_shared<Earth>(glm::vec3{ 0.0f }, 6.371f, sphereMesh);
	auto moon = std::make_shared<Moon>(glm::vec3{ 384.400f, 0.0f, 0.0f }, 10.737f, sphereMesh);

	auto axis = std::make_shared<Axis>();

	const float MoonRotationSpeed = 0.3f;

	auto moonAnimator = std::make_shared<Animator>(moon->transform);
	moonAnimator->RotationSpeed.z = MoonRotationSpeed;
	moonAnimator->WorldRotationSpeed.y = -MoonRotationSpeed;

	auto sunAnimator = std::make_shared<Animator>(scene->light->transform);
	sunAnimator->WorldRotationSpeed.y = -0.03f;

	scene->addAnimator(moonAnimator);
	scene->addAnimator(sunAnimator);

	// Order is important here. Earth must be the last
	scene->addDrawable(moon);
	scene->addDrawable(earth);
	scene->addDrawable(axis);
}

static void init()
{
    gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);   
	
	createScene();
}

int main()
{
    GLFWwindow* window = 0;
    
    if(!glfw::init())
        exit_msg("Failed to initialize GLFW")
    
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
    glfwWindowHint(GLFW_SAMPLES, 16);

    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, 1);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    std::string windowTitleBase = "Physically Based Rendering with OpenGL";
    
	if (s_bFullScreen)
	{
		int monitorsCount;
		GLFWmonitor** monitors = glfwGetMonitors(&monitorsCount);
		const int activeMonitorIdx = 1;

		const GLFWvidmode* mode = glfwGetVideoMode(monitors[activeMonitorIdx]);
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		window = glfwCreateWindow(mode->width, mode->height, windowTitleBase.data(), monitors[activeMonitorIdx], nullptr);
	}
	else
	{
		window = glfwCreateWindow(s_WindowWidth, s_WindowHeight, windowTitleBase.data(), nullptr, nullptr);
	}
    
    if (!window)
    {
        glfw::terminate();
        exit_msg("Failed to open GLFW window");
    }
    
    // Set callback functions
    glfwSetFramebufferSizeCallback(window, reshape);
    glfwSetKeyCallback(window, key);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, mouseMotionCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(s_bEnableVSync);

    //===============================================================================================================================================================================================================
    // GLEW library initialization
    //===============================================================================================================================================================================================================
    glewExperimental = true;                                                                                                // needed in core profile 
    GLenum result = glewInit();                                                                                             // initialise GLEW
    if (result != GLEW_OK) 
    {
        glfw::terminate();
        exit_msg("Failed to initialize GLEW : %s", glewGetErrorString(result));
    }
    debug_msg("GLEW library initialization done ... ");
    
    glfwGetFramebufferSize(window, &s_WindowWidth, &s_WindowHeight);
    reshape(window, s_WindowWidth, s_WindowHeight);

    init();
    
    FPSTimer fpsTimer;
    Timer frameTimer;
    
    // Main loop
    while( !glfwWindowShouldClose(window) )
    {        
        draw();
        
        // Update animation
        scene->update(frameTimer.elapsedSeconds());
        
        frameTimer.start();
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }
    
    glfw::terminate();
    return 0;
}