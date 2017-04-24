#ifndef _util_included_78501350607812561208375160374856384971584568743165874365
#define _util_included_78501350607812561208375160374856384971584568743165874365

#include <array>
#include <string>
#include <windows.h>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace util
{
	char* read_file(const char* file_name);

	// A debug callback function
	void APIENTRY glDebugCallback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei len, const GLchar *msg, const GLvoid *user);

	bool loadOBJ(const char * file_name, GLuint &vbo, GLuint &ebo, size_t &nElems);

	void gl_info();

};

#endif //_util_included_78501350607812561208375160374856384971584568743165874365
