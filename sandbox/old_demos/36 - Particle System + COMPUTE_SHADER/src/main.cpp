#ifndef _GLEW_
#define _GLEW_
	#include <GL\glew.h>
	#include <GLFW\glfw3.h>
#endif

#include <iostream>
#include <string>
#include <stdexcept>

//#include <boost\lexical_cast.hpp>

#include "GLFWWindow.h"
#include "ParticleSystem.h"

int main()
{
	GLFWWindow* wnd;

	wnd = new GLFWWindow(1920, 1080, "OpenGL43 PARTICLE SYSTEM", true);

	glewExperimental = GL_TRUE;

	if(glewInit() != GLEW_OK)
	{
		std::cout << "Could not initialize GLEW!" <<std::endl;
		std::cin.get();
		delete wnd;
		return -1;
	}
	else
	{
		std::cout << "GLEW Version: " <<glewGetString(GLEW_VERSION) <<std::endl;
	};
	
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

                              			 // max particles, iniRadius, quadLength, max fps, translation_velocity, rotation_velocity
	ParticleSystem* ps = new ParticleSystem(40000, 50, 0.00999999978f, 60, 20.0f, 80.0f);


	std::cout << "Executing Particle System" <<std::endl;
	ps->run(wnd);

	delete ps;
	delete wnd;
	return 0;
};
