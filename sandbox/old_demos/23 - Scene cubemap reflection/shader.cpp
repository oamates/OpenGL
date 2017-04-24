
#include "shader.hpp"
#include "log.hpp"

//==============================================================================================================================================================
// A simple-minded wrap-up for glsl shaders and programs 
// OpenGL has its state data exposed and easily corruptible by inappropriate use.    
// Our code continues this tradition and therefore we have structures, not classes. 
//==============================================================================================================================================================

// should be moved to some auxiliary module
char* file_read(const char* filename)
{
    FILE* input = fopen(filename, "rb");
    if (!input) 
    {
        debug_msg("File %s failed to open.", filename);
        return 0;
    };
    if (fseek(input, 0, SEEK_END) == -1) 
    {
        debug_msg("End of the file %s not found.", filename);
        return 0;
    };
    long int size = ftell(input);
    if (size == -1) 
    {
        debug_msg("File %s is empty.", filename);
        return 0;
    };

    if (fseek(input, 0, SEEK_SET) == -1) 
    {
        debug_msg("File %s reading error.", filename);
        return 0;
    };
  	
    char * content = (char*) malloc ((size_t) size + 1);
    if (!content) return 0;
    fread(content, 1, (size_t)size, input);
    if (ferror(input)) 
    {
        debug_msg("File %s reading error.", filename);
        free(content);
        return 0;
    }
    fclose(input);
    content[size] = '\0';
    return content;
}


//==============================================================================================================================================================
// shader
//==============================================================================================================================================================

glsl_shader::glsl_shader() : id(0) {};
glsl_shader::glsl_shader(GLenum shader_type, const char* filename)
{
    type = shader_type;
    char* source_code = file_read(filename);
    id = compile_from_string(shader_type, source_code);
    free(source_code);
};
glsl_shader::~glsl_shader()
    { glDeleteShader(id); };	

bool glsl_shader::is_valid() 
    { return id; };

GLint glsl_shader::compile_from_string(GLenum shader_type, const char* source_code)
{
	debug_msg("Compiling shader : type = %d, source_code = \n\n%s", shader_type, source_code);							// compile fragment shader
    GLint shader_id = glCreateShader(shader_type);
	glShaderSource (shader_id, 1, &source_code, 0);
	glCompileShader (shader_id);
    GLint compile_status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_status);											// check if the fragment shader was successfully compiled
    if (GL_TRUE == compile_status) 
    {
        debug_msg("Shader id#%d of type %d successfully compiled.", shader_id, shader_type);
        return shader_id;
    }
	// GL_FALSE == compile_status
    debug_msg("Error compiling shader id#%d, type = %d", shader_id, shader_type);
    GLint error_msg_length;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &error_msg_length);
    if (error_msg_length)
	{
        char * error_msg = (char*) malloc ((size_t) error_msg_length);
        glGetShaderInfoLog (shader_id, error_msg_length, 0, error_msg);
        debug_msg("Compiler message : %s", error_msg);
        free (error_msg);

    };
    glDeleteShader(shader_id);
    return 0;
};

//==============================================================================================================================================================
// program = vertex + fragment + geometry 
//==============================================================================================================================================================

glsl_program::glsl_program() : id(glCreateProgram()) {};

glsl_program::glsl_program(const glsl_shader& vs, const glsl_shader& fs)
{
    id = glCreateProgram();
    attach(vs.id);         
    attach(fs.id); 
    link();         
}

glsl_program::glsl_program(const glsl_shader& vs, const glsl_shader& gs, const glsl_shader& fs)
{
    id = glCreateProgram();
    attach(vs.id); 
    attach(gs.id); 
    attach(fs.id); 
    link();         
}

glsl_program::glsl_program(const glsl_shader& vs, const glsl_shader& tcs, const glsl_shader& tes, const glsl_shader& gs, const glsl_shader& fs)
{
    id = glCreateProgram();
    attach(vs.id);       
    attach(tcs.id); 
    attach(tes.id); 
    attach(gs.id);  
    attach(fs.id);  
    link();         
};

void glsl_program::attach(GLint sid)
{
    shader_id.push_back(sid);
    glAttachShader(id, sid); 
}

void glsl_program::link()
{
    glLinkProgram(id);
    GLint linkage_status;
    glGetProgramiv(id, GL_LINK_STATUS, &linkage_status);
    if (GL_TRUE == linkage_status) 
    {
        for (unsigned int i = 0; i < shader_id.size(); ++i) glDetachShader(id, shader_id[i]);
        debug_msg("Program %d successfully linked.", id);
        return;
    };
    GLint error_msg_length;
    glGetProgramiv (id, GL_INFO_LOG_LENGTH, &error_msg_length);
    

    debug_msg("error_msg_length = %d", error_msg_length);
    if (error_msg_length)
	{
        char* error_msg = new char [error_msg_length];
        GLsizei length;
        glGetProgramInfoLog (id, error_msg_length, 0, error_msg);
        debug_msg("Program linkage error : %s", error_msg);
        delete error_msg;
    }
    glDeleteProgram(id);
    exit_msg("Aborting program ...");
}

glsl_program::~glsl_program() 
    { glDeleteProgram(id); };

GLint glsl_program::uniform_id (const char* name)
{ 
    GLint uid = glGetUniformLocation(id, name); 
    debug_msg("Program #%i uniform %s id = %i", id, name, uid);
    return uid;
};

void glsl_program::enable()
    { glUseProgram(id); };

void glsl_program::disable()
    { glUseProgram(0); };

