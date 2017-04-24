#ifndef _shader_included_245196303876410928396854735684527895739527664352362354 
#define _shader_included_245196303876410928396854735684527895739527664352362354

//=======================================================================================================================================================================================================================
// A simple-minded wrap-up for glsl shaders and programs 
// OpenGL has its state data exposed and easily corruptible by inappropriate use.    
// Our code continues this tradition and therefore we have structures, not classes. 
//=======================================================================================================================================================================================================================

#include <vector>
#include <GL/glew.h>

//=======================================================================================================================================================================================================================
// glsl_shader structure
//=======================================================================================================================================================================================================================

struct glsl_shader
{
    GLint id;
    GLenum type;
    // The shader type must be one of GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER, or GL_COMPUTE_SHADER. 
    glsl_shader();
    glsl_shader(GLenum shader_type, const char* filename);
    ~glsl_shader();
    bool is_valid();
    static GLint compile_from_string(GLenum shader_type, const char* source_code);

};   

//=======================================================================================================================================================================================================================
// glsl_program structure
//=======================================================================================================================================================================================================================

struct glsl_program
{
    GLuint id;
    std::vector<GLint> shader_id;

    glsl_program();
    glsl_program(const glsl_shader& vs, const glsl_shader& fs);
    glsl_program(const glsl_shader& vs, const glsl_shader& gs, const glsl_shader& fs);
    glsl_program(const glsl_shader& vs, const glsl_shader& tcs, const glsl_shader& tes, const glsl_shader& gs, const glsl_shader& fs);

	void init(const glsl_shader& vs, const glsl_shader& fs);
    void init(const glsl_shader& vs, const glsl_shader& gs, const glsl_shader& fs);
    void init(const glsl_shader& vs, const glsl_shader& tcs, const glsl_shader& tes, const glsl_shader& gs, const glsl_shader& fs);


    ~glsl_program(); 

    void attach(GLint shader_id);
    void link();
    GLint uniform_id (const char* name);
	GLuint uniform_block_index(const char* name);

    void enable();
    void disable();
    
};

#endif // _shader_included_245196303876410928396854735684527895739527664352362354