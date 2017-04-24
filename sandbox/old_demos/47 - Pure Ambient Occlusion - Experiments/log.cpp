// ======================================================================================================================================================================================================================
// logging and opengl bebugging implementation
// ======================================================================================================================================================================================================================

#include "log.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>														                                                // windows and event management library

void log_write(const char *format, ...)
{
	va_list ap;
	FILE *output = fopen("debug.log", "a+");
	if (!output) return;
	va_start(ap, format);
	vfprintf(output, format, ap);
	va_end(ap);
	fclose(output);
};

// ======================================================================================================================================================================================================================
// log opengl information for the current implementation
// ======================================================================================================================================================================================================================

void glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const GLvoid *userParam)
{
	static const char * UNDEFINED = "undefined";
	const char *SEVERITY, *SOURCE, *TYPE;
	switch (source)
	{
		case GL_DEBUG_SOURCE_API:             SOURCE = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   SOURCE = "Window system"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: SOURCE = "Shader compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     SOURCE = "Third party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     SOURCE = "Application"; break;
	    default:							  SOURCE = UNDEFINED;	  	
	};
	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:               TYPE = "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	TYPE = "Deprecated behavior"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  TYPE = "Undefined behavior";	break;
		case GL_DEBUG_TYPE_PORTABILITY:         TYPE = "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         TYPE = "Performance"; break;
		default:								TYPE = UNDEFINED;	  	
	};
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:   SEVERITY = "High"; break;
		case GL_DEBUG_SEVERITY_MEDIUM: SEVERITY = "Medium"; break;
		case GL_DEBUG_SEVERITY_LOW:    SEVERITY = "Low"; break;
		default:					   SEVERITY = UNDEFINED;	  	
	};
	debug_msg("OpenGL debug message : source = %s, type = %s, id = %u, severity = %s.\n\t\t msg : %s\n\t\ttime = %f.", SOURCE, TYPE, id, SEVERITY, msg, glfwGetTime());
};
