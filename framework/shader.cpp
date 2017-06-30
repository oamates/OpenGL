//=======================================================================================================================================================================================================================
// shader + program structure methods implementation
//=======================================================================================================================================================================================================================
#include <memory>

#include "shader.hpp"
#include "utils.hpp"
#include "log.hpp"

//=======================================================================================================================================================================================================================
// shader methods implementation
//=======================================================================================================================================================================================================================

glsl_shader_t::glsl_shader_t() : id(0) {}

glsl_shader_t::glsl_shader_t(GLenum shader_type, const char* file_name)
{
    type = shader_type;
    debug_msg("Creating shader : source file = %s", file_name);
    char* source_code = utils::file_read(file_name);
    id = compile_from_string(shader_type, source_code);
    free(source_code);
}

glsl_shader_t::glsl_shader_t(const char* shader_source, GLenum shader_type)
{
    type = shader_type;
    id = compile_from_string(shader_type, shader_source);
}

glsl_shader_t::~glsl_shader_t()
    { glDeleteShader(id); } 

GLint glsl_shader_t::compile_from_string(GLenum shader_type, const char* source_code)
{
    debug_msg("Compiling shader : type = %d. source code = \n", shader_type);
    put_msg(source_code);

    GLint shader_id = glCreateShader(shader_type);
    glShaderSource (shader_id, 1, &source_code, 0);
    glCompileShader (shader_id);
    GLint compile_status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_status);

    if (GL_TRUE == compile_status) 
    {
        debug_msg("Shader id#%d of type %d successfully compiled.", shader_id, shader_type);
        GLint error_msg_length;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &error_msg_length);

        if (error_msg_length)
        {
            char* error_msg = static_cast<char*> (malloc((size_t) error_msg_length));
            glGetShaderInfoLog (shader_id, error_msg_length, 0, error_msg);
            debug_msg("Compiler message : ");
            put_msg(error_msg);
            free(error_msg);
        }

        return shader_id;
    }

    debug_msg("Error compiling shader id#%d, type = %d", shader_id, shader_type);
    GLint error_msg_length;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &error_msg_length);
    
    if (error_msg_length)
    {
        char* error_msg = static_cast<char*> (malloc((size_t) error_msg_length));
        glGetShaderInfoLog (shader_id, error_msg_length, 0, error_msg);
        debug_msg("Compiler message : ");
        put_msg(error_msg);
        free(error_msg);
    }

    glDeleteShader(shader_id);
    return 0;
}

//=======================================================================================================================================================================================================================
// program methods implementation
//=======================================================================================================================================================================================================================

glsl_program_t::glsl_program_t() 
    { }

glsl_program_t::glsl_program_t(const glsl_shader_t& cs)
    { link(cs); }

glsl_program_t::glsl_program_t(const glsl_shader_t& vs, const glsl_shader_t& fs)
    { link(vs, fs); }

glsl_program_t::glsl_program_t(const glsl_shader_t& vs, const glsl_shader_t& gs, const glsl_shader_t& fs)
    { link (vs, gs, fs); }

glsl_program_t::glsl_program_t(const glsl_shader_t& vs, const glsl_shader_t& tcs, const glsl_shader_t& tes, const glsl_shader_t& fs)
    { link(vs, tcs, tes, fs); }

glsl_program_t::glsl_program_t(const glsl_shader_t& vs, const glsl_shader_t& tcs, const glsl_shader_t& tes, const glsl_shader_t& gs, const glsl_shader_t& fs)
    { link(vs, tcs, tes, gs, fs); }


void glsl_program_t::link(const glsl_shader_t& cs)
    { id = glCreateProgram(); attach(cs.id); link(); };

void glsl_program_t::link(const glsl_shader_t& vs, const glsl_shader_t& fs)
    { id = glCreateProgram(); attach(vs); attach(fs); link(); };

void glsl_program_t::link(const glsl_shader_t& vs, const glsl_shader_t& gs,  const glsl_shader_t& fs)
    { id = glCreateProgram(); attach(vs); attach(gs); attach(fs); link(); };

void glsl_program_t::link(const glsl_shader_t& vs, const glsl_shader_t& tcs, const glsl_shader_t& tes, const glsl_shader_t& fs)
    { id = glCreateProgram(); attach(vs); attach(tcs); attach(tes); attach(fs); link(); };

void glsl_program_t::link(const glsl_shader_t& vs, const glsl_shader_t& tcs, const glsl_shader_t& tes, const glsl_shader_t& gs, const glsl_shader_t& fs)
    { id = glCreateProgram(); attach(vs); attach(tcs); attach(tes); attach(gs); attach(fs); link(); };


void glsl_program_t::attach(const glsl_shader_t& shader)
    { glAttachShader(id, shader.id); };

void glsl_program_t::attach(GLint shader_id)
    { glAttachShader(id, shader_id); }

void glsl_program_t::link()
{
    debug_msg("Linking shader :: id = [%d].", id);
    glLinkProgram(id);
    GLint linkage_status;
    glGetProgramiv(id, GL_LINK_STATUS, &linkage_status);
    if (GL_TRUE == linkage_status) 
    {
        debug_msg("Program [%d] successfully linked.", id);
        return;
    };
    GLint error_msg_length;
    glGetProgramiv (id, GL_INFO_LOG_LENGTH, &error_msg_length);

    debug_msg("Program [%d] link not successful. Log message length = %d", id, error_msg_length);
    if (error_msg_length)
    {
        char* error_msg = static_cast<char*>(malloc ((size_t) error_msg_length));
        glGetProgramInfoLog (id, error_msg_length, 0, error_msg);
        debug_msg("Program linkage error : ");
        put_msg(error_msg);
        free(error_msg);
    }
    glDeleteProgram(id);
    exit_msg("Aborting program ...");
}

glsl_program_t::~glsl_program_t() 
    { glDeleteProgram(id); }

uniform_t glsl_program_t::operator[] (const char* name)
    { return uniform_t(*this, name); }

GLint glsl_program_t::uniform_id(const char * name) 
    { return glGetUniformLocation(id, name); }

GLuint glsl_program_t::subroutine_index(GLenum shader_type, const char* name)
{
    GLuint index = glGetSubroutineIndex(id, shader_type, name);
    debug_msg("Program [%i] subroutine [%s] in shader of type = [%d] has index = [%d]", id, name, shader_type, index);
    return index;
}

GLuint glsl_program_t::subroutine_location(GLenum shader_type, const char* name)
{
    GLuint location = glGetSubroutineUniformLocation(id, shader_type, name);
    debug_msg("Program [%i] subroutine [%s] uniform in shader of type = [%d] has location = [%d]", id, name, shader_type, location);
    return location;
}

void glsl_program_t::bind_ubo(const char* block_name, GLuint target)
{
    GLuint ubi = glGetUniformBlockIndex(id, block_name);
    glUniformBlockBinding(id, ubi, target);
}

void glsl_program_t::enable()
    { glUseProgram(id); }

void glsl_program_t::disable()
    { glUseProgram(0); }

GLint glsl_program_t::get_param(GLint param_name)
{
    GLint value;
    glGetProgramiv(id, param_name, &value);
    return value;
}

void glsl_program_t::dump_param(GLint param_name, const char* description)
{
    debug_msg("%s : %d", description, get_param(param_name));
}

void glsl_program_t::dump_info()
{   
    dump_param(GL_DELETE_STATUS,                         "Program flagged for deletion");
    dump_param(GL_LINK_STATUS,                           "Last link operation");
    dump_param(GL_VALIDATE_STATUS,                       "Last validation operation");
    dump_param(GL_INFO_LOG_LENGTH,                       "Length of the log information");
    dump_param(GL_ATTACHED_SHADERS,                      "The number of shader objects attached to program");
    dump_param(GL_ACTIVE_ATOMIC_COUNTER_BUFFERS,         "The number of active attribute atomic counter buffers used by program");
    dump_param(GL_ACTIVE_ATTRIBUTES,                     "The number of active attribute variables for program");
    dump_param(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,           "The longest active attribute name for program");
    dump_param(GL_ACTIVE_UNIFORMS,                       "The number of active uniform variables for program");
    dump_param(GL_ACTIVE_UNIFORM_MAX_LENGTH,             "The length of the longest active uniform variable name for program");
    dump_param(GL_PROGRAM_BINARY_LENGTH,                 "The length of the program binary, in bytes");
    dump_param(GL_TRANSFORM_FEEDBACK_BUFFER_MODE,        "The buffer mode used when transform feedback is active (GL_SEPARATE_ATTRIBS or GL_INTERLEAVED_ATTRIBS)");
    dump_param(GL_TRANSFORM_FEEDBACK_VARYINGS,           "The number of varying variables to capture in transform feedback mode for the program");
    dump_param(GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH, "The longest variable name to be used for transform feedback");
    dump_param(GL_GEOMETRY_VERTICES_OUT,                 "The maximum number of vertices that the geometry shader in program will output");
    dump_param(GL_GEOMETRY_INPUT_TYPE,                   "Geometry shader input primitive type");
    dump_param(GL_GEOMETRY_OUTPUT_TYPE,                  "Geometry shader output primitive type");
    // GL_COMPUTE_WORK_GROUP_SIZE "The local work group size (x, y, z) of the compute program"
}

/*

// Create a simple program containing only a vertex shader
static const GLchar source[] = { ... };

// First create and compile the shader
GLuint shader;
shader = glCreateShader(GL_VERTEX_SHADER);
glShaderSource(shader, 1, suorce, NULL);
glCompileShader(shader);

// Create the program and attach the shader to it
GLuint program;
program = glCreateProgram();
glAttachShader(program, shader);

// Set the binary retrievable hint and link the program
glProgramParameteri(program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
glLinkProgram(program);

// Get the expected size of the program binary
GLint binary_size = 0;
glGetProgramiv(program, GL_PROGRAM_BINARY_SIZE, &binary_size);

// Allocate some memory to store the program binary
unsigned char * program_binary = new unsigned char [binary_size];

// Now retrieve the binary from the program object
GLenum binary_format = GL_NONE;
glGetProgramBinary(program, binary_size, NULL, &binary_format, program_binary);

*/
/*
void glsl_program_t::dump_interface()
{
    std::cout << "--------------------" << name << " Interface------------------------" << std::endl; 
    GLint outputs = 0;
    glGetProgramInterfaceiv(program, GL_PROGRAM_INPUT,  GL_ACTIVE_RESOURCES, &outputs);
    static const GLenum props[] = {GL_TYPE, GL_LOCATION};
    GLint params[2];
    GLchar name[64];
    const char *type_name;

    if (outputs > 0)
       std::cout << "----------Input-----------" << std::endl;
    std::cout << std::endl;
    for (int i = 0; i != outputs; ++i)
    {
        glGetProgramResourceName(program, GL_PROGRAM_INPUT, i, sizeof(name), NULL, name);
        glGetProgramResourceiv(program, GL_PROGRAM_INPUT, i, 2, props, 2, NULL, params);
        type_name = name;
        //std::cout << "Index " << i << std::endl;
        std::cout <<  "(" <<  type_name  << ")" << " locatoin: " << params[1] << std::endl;
    }

    glGetProgramInterfaceiv(program, GL_PROGRAM_OUTPUT,  GL_ACTIVE_RESOURCES, &outputs);
    if (outputs > 0)
       std::cout << "----------Onput-----------" << std::endl;
    std::cout << std::endl;

    for (int i = 0; i != outputs; ++i)
    {
        glGetProgramResourceName(program, GL_PROGRAM_OUTPUT, i, sizeof(name), NULL, name);
        glGetProgramResourceiv(program, GL_PROGRAM_OUTPUT, i, 2, props, 2, NULL, params);

        type_name = name;
        //std::cout << "Index " << i << std::endl;
        std::cout  <<  "(" <<  type_name  << ")" << " locatoin: " << params[1] << std::endl;
    }
    
    glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK,  GL_ACTIVE_RESOURCES, &outputs);
    if (outputs > 0)
      std::cout << "------Uniform Block-------" << std::endl;
    std::cout << std::endl;
    for (int i = 0; i != outputs; ++i)
    {
        glGetProgramResourceName(program, GL_UNIFORM_BLOCK, i, sizeof(name), NULL, name);
        glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, i, 2, props, 2, NULL, params);

        type_name = name;
        //std::cout << "Index " << i << std::endl;
        std::cout  <<  "(" <<  type_name  << ")" << " locatoin: " << params[1] << std::endl;
    }


    glGetProgramInterfaceiv(program, GL_UNIFORM,  GL_ACTIVE_RESOURCES, &outputs);
    if (outputs > 0)
        std::cout << "----------Uniform---------" << std::endl;

    if (outputs > 10)
        return ;
    for (int i = 0; i != outputs; ++i)
    {
        glGetProgramResourceName(program, GL_UNIFORM, i, sizeof(name), NULL, name);
        glGetProgramResourceiv(program, GL_UNIFORM, i, 2, props, 2, NULL, params);

        type_name = name;
        //std::cout << "Index " << i << std::endl;
        std::cout  <<  "(" <<  type_name  << ")" << " locatoin: " << params[1] << std::endl;
    }

}
*/