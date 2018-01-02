//========================================================================================================================================================================================================================
// DEMO 096 : Program interface and binary
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS
#define GLM_FORCE_NO_CTOR_INIT

#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "gl_aux.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "vao.hpp"
#include "vertex.hpp"

struct demo_window_t : public glfw_window_t
{
    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
    }

    void on_mouse_move() override
    {
    }
};

//=======================================================================================================================================================================================================================
// GLSL Program resource interface tokens ::
//      void glGetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint* params);
//=======================================================================================================================================================================================================================
// GL_UNIFORM                             | The query is targeted at the set of active uniforms within program.
// GL_UNIFORM_BLOCK                       | The query is targeted at the set of active uniform blocks within program.
// GL_ATOMIC_COUNTER_BUFFER               | The query is targeted at the set of active atomic counter buffer binding points within program.
//
// GL_PROGRAM_INPUT                       | The query is targeted at the set of active input variables used by the first shader stage of program.
//                                        | If program contains multiple shader stages then input variables from any stage other than the first will not be enumerated.
// GL_PROGRAM_OUTPUT                      | The query is targeted at the set of active output variables produced by the last shader stage of program.
//                                        | If program contains multiple shader stages then output variables from any stage other than the last will not be enumerated.
//
// GL_VERTEX_SUBROUTINE                   | The query is targeted at the set of active subroutines for the vertex,
// GL_TESS_CONTROL_SUBROUTINE             | tessellation control,
// GL_TESS_EVALUATION_SUBROUTINE          | tessellation evaluation,
// GL_GEOMETRY_SUBROUTINE                 | geometry,
// GL_FRAGMENT_SUBROUTINE                 | fragment and
// GL_COMPUTE_SUBROUTINE                  | compute shader stages of program, respectively.
//
// GL_VERTEX_SUBROUTINE_UNIFORM           | The query is targeted at the set of active subroutine uniform variables used by the vertex,
// GL_TESS_CONTROL_SUBROUTINE_UNIFORM     | tessellation control,
// GL_TESS_EVALUATION_SUBROUTINE_UNIFORM  | tessellation evaluation,
// GL_GEOMETRY_SUBROUTINE_UNIFORM         | geometry,
// GL_FRAGMENT_SUBROUTINE_UNIFORM         | fragment and
// GL_COMPUTE_SUBROUTINE_UNIFORM          | compute shader stages of program, respectively.
//
// GL_TRANSFORM_FEEDBACK_VARYING          | The query is targeted at the set of output variables from the last non-fragment stage of program that
//                                        | would be captured if transform feedback were active.
// GL_TRANSFORM_FEEDBACK_BUFFER           | The query is targeted at the set of active buffer binding points to which output variables in the GL_TRANSFORM_FEEDBACK_VARYING
//                                        | interface are written.
// GL_BUFFER_VARIABLE                     | The query is targeted at the set of active buffer variables used by program.
//
// GL_SHADER_STORAGE_BLOCK                | The query is targeted at the set of active shader storage blocks used by program.
//
//=======================================================================================================================================================================================================================
// GLSL Program resource interface :: properties and supported interfaces
//=======================================================================================================================================================================================================================
//
// -- GL_UNIFORM
// { GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_STRIDE, GL_BLOCK_INDEX, GL_IS_ROW_MAJOR, GL_MATRIX_STRIDE, GL_ATOMIC_COUNTER_BUFFER_INDEX, GL_OFFSET, GL_ARRAY_SIZE, GL_LOCATION
//   GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER }
//
// -- GL_UNIFORM_BLOCK
// { GL_NAME_LENGTH, GL_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_DATA_SIZE,
//   GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER }
//
// -- GL_ATOMIC_COUNTER_BUFFER
// { ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_DATA_SIZE,
//   GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER }
//
// -- GL_PROGRAM_INPUT
// { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_IS_PER_PATCH, GL_LOCATION_COMPONENT, GL_ARRAY_SIZE
//   GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER }
//
// -- GL_PROGRAM_OUTPUT
// { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_IS_PER_PATCH, GL_LOCATION_COMPONENT, GL_LOCATION_INDEX, GL_ARRAY_SIZE
//   GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER }
//
// -- GL_VERTEX_SUBROUTINE { GL_NAME_LENGTH, }
// -- GL_TESS_CONTROL_SUBROUTINE { GL_NAME_LENGTH, }
// -- GL_TESS_EVALUATION_SUBROUTINE { GL_NAME_LENGTH, }
// -- GL_GEOMETRY_SUBROUTINE { GL_NAME_LENGTH, }
// -- GL_FRAGMENT_SUBROUTINE { GL_NAME_LENGTH, }
// -- GL_COMPUTE_SUBROUTINE { GL_NAME_LENGTH, }
//
// -- GL_VERTEX_SUBROUTINE_UNIFORM
// { GL_NAME_LENGTH, GL_LOCATION, GL_COMPATIBLE_SUBROUTINES, GL_NUM_COMPATIBLE_SUBROUTINES, GL_ARRAY_SIZE }
// -- GL_TESS_CONTROL_SUBROUTINE_UNIFORM
// { GL_NAME_LENGTH, GL_LOCATION, GL_COMPATIBLE_SUBROUTINES, GL_NUM_COMPATIBLE_SUBROUTINES, GL_ARRAY_SIZE }
// -- GL_TESS_EVALUATION_SUBROUTINE_UNIFORM
// { GL_NAME_LENGTH, GL_LOCATION, GL_COMPATIBLE_SUBROUTINES, GL_NUM_COMPATIBLE_SUBROUTINES, GL_ARRAY_SIZE }
// -- GL_GEOMETRY_SUBROUTINE_UNIFORM
// { GL_NAME_LENGTH, GL_LOCATION, GL_COMPATIBLE_SUBROUTINES, GL_NUM_COMPATIBLE_SUBROUTINES, GL_ARRAY_SIZE }
// -- GL_FRAGMENT_SUBROUTINE_UNIFORM
// { GL_NAME_LENGTH, GL_LOCATION, GL_COMPATIBLE_SUBROUTINES, GL_NUM_COMPATIBLE_SUBROUTINES, GL_ARRAY_SIZE }
// -- GL_COMPUTE_SUBROUTINE_UNIFORM
// { GL_NAME_LENGTH, GL_LOCATION, GL_COMPATIBLE_SUBROUTINES, GL_NUM_COMPATIBLE_SUBROUTINES, GL_ARRAY_SIZE }
//
//
// -- GL_TRANSFORM_FEEDBACK_VARYING
// { GL_NAME_LENGTH, GL_TYPE, GL_TRANSFORM_FEEDBACK_BUFFER_INDEX, GL_OFFSET, GL_ARRAY_SIZE }
//
// -- GL_TRANSFORM_FEEDBACK_BUFFER
// { GL_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES, GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE }
//
// -- GL_BUFFER_VARIABLE
// { GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_STRIDE, GL_BLOCK_INDEX, GL_IS_ROW_MAJOR, GL_MATRIX_STRIDE, GL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_STRIDE, GL_OFFSET,
//   GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER }
//
// -- GL_SHADER_STORAGE_BLOCK
// { GL_NAME_LENGTH, ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_DATA_SIZE,
//   GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER }
//
//

namespace glsl_interface {

struct stages_info_t
{
    GLint vertex;
    GLint tess_control;
    GLint tess_evaluation;
    GLint geometry;
    GLint fragment;
    GLint compute;

    static constexpr GLenum tokens[] = { GL_REFERENCED_BY_VERTEX_SHADER,
                                     GL_REFERENCED_BY_TESS_CONTROL_SHADER,
                                     GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
                                     GL_REFERENCED_BY_GEOMETRY_SHADER,
                                     GL_REFERENCED_BY_FRAGMENT_SHADER,
                                     GL_REFERENCED_BY_COMPUTE_SHADER };
};

struct buffer_info_t
{
    GLint active_variables;
    GLint buffer_binding;
    GLint num_active_variables;

    static constexpr GLenum tokens[] =
    {
        GL_ACTIVE_VARIABLES,
        GL_BUFFER_BINDING,
        GL_NUM_ACTIVE_VARIABLES
    };
};

struct io_attribute_info_t
{
    GLint type;
    GLint location;
    GLint location_component;
    GLint array_size;

    static constexpr GLenum tokens[] =
    {
        GL_TYPE,
        GL_LOCATION,
        GL_LOCATION_COMPONENT,
        GL_ARRAY_SIZE
    };
};

struct variable_info_t
{
    GLint type;
    GLint offset;
    GLint array_size;

    static constexpr GLenum tokens[] =
    {
        GL_TYPE,
        GL_OFFSET,
        GL_ARRAY_SIZE
    };
};

struct variable_layout_info_t
{
    GLint array_stride;
    GLint block_index;
    GLint is_row_major;
    GLint matrix_stride;

    static constexpr GLenum tokens[] =
    {
        GL_ARRAY_STRIDE,
        GL_BLOCK_INDEX,
        GL_IS_ROW_MAJOR,
        GL_MATRIX_STRIDE
    };

    static constexpr std::array<GLenum, 4> tokens1 =
    {
        GL_ARRAY_STRIDE,
        GL_BLOCK_INDEX,
        GL_IS_ROW_MAJOR,
        GL_MATRIX_STRIDE
    };

};


struct top_level_array_info_t
{
    GLint size;
    GLint stride;

    static constexpr GLenum tokens[] =
    {
        GL_TOP_LEVEL_ARRAY_SIZE,
        GL_TOP_LEVEL_ARRAY_STRIDE
    };
};

//----------------------------------------------------------------------------
struct uniform_t                            /* GL_UNIFORM */
{
    GLint name_length;
    variable_info_t variable_info;
    variable_layout_info_t variable_layout_info;
    GLint location;
    GLint atomic_counter_buffer_index;
    stages_info_t stages_info;

    static constexpr GLenum tokens[] =
    {
        GL_NAME_LENGTH,
        variable_info_t::tokens,
        variable_layout_info_t::tokens,
        GL_LOCATION,
        GL_ATOMIC_COUNTER_BUFFER_INDEX,
        stages_info_t::tokens
    };
};

struct uniform_block_t                      /* GL_UNIFORM_BLOCK */
{
    GLint name_length;
    buffer_info_t buffer_info;
    GLint buffer_data_size;
    stages_info_t stages_info;

    static constexpr GLenum tokens[] =
    {
        GL_NAME_LENGTH,
        buffer_info_t::tokens,
        GL_BUFFER_DATA_SIZE,
        stages_info_t::tokens
    };
};


struct atomic_counter_buffer_t              /* GL_ATOMIC_COUNTER_BUFFER */
{
    buffer_info_t buffer_info;
    GLint buffer_data_size;
    stages_info_t stages_info;

    static constexpr GLenum tokens[] =
    {
        buffer_info_t::tokens,
        GL_BUFFER_DATA_SIZE,
        stages_info_t::tokens
    };
};

struct program_input_t                      /* GL_PROGRAM_INPUT */
{
    GLint name_length;
    io_attribute_info_t io_attribute_info;
    GLint is_per_patch;
    stages_info_t stages_info;

    static constexpr GLenum tokens[] =
    {
        GL_NAME_LENGTH,
        io_attribute_info_t::tokens,
        GL_IS_PER_PATCH,
        stages_info_t::tokens
    };
};

struct program_output_t                     /* GL_PROGRAM_OUTPUT */
{
    GLint name_length;
    io_attribute_info_t io_attribute_info;
    GLint location_index;
    GLint is_per_patch;
    stages_info_t stages_info;

    static constexpr GLenum tokens[] =
    {
        GL_NAME_LENGTH,
        io_attribute_info_t::tokens,
        GL_LOCATION_INDEX,
        GL_IS_PER_PATCH,
        stages_info_t::tokens
    };
};

struct subroutine_t                         /* GL_VERTEX_SUBROUTINE, GL_TESS_CONTROL_SUBROUTINE, GL_TESS_EVALUATION_SUBROUTINE, GL_GEOMETRY_SUBROUTINE, GL_FRAGMENT_SUBROUTINE, GL_COMPUTE_SUBROUTINE */
{
    GLint name_length;
    static constexpr GLenum tokens[] =
    {
        GL_NAME_LENGTH
    };
};

struct subroutine_uniform_t                 /* GL_VERTEX_SUBROUTINE_UNIFORM, GL_TESS_CONTROL_SUBROUTINE_UNIFORM, GL_TESS_EVALUATION_SUBROUTINE_UNIFORM, GL_GEOMETRY_SUBROUTINE_UNIFORM, GL_FRAGMENT_SUBROUTINE_UNIFORM, GL_COMPUTE_SUBROUTINE_UNIFORM */
{
    GLint name_length;
    GLint location;
    GLint compatible_subroutines;
    GLint num_compatible_subroutines;
    GLint array_size;

    static constexpr GLenum tokens[] =
    {
        GL_NAME_LENGTH,
        GL_LOCATION,
        GL_COMPATIBLE_SUBROUTINES,
        GL_NUM_COMPATIBLE_SUBROUTINES,
        GL_ARRAY_SIZE
    };
};

struct transform_feedback_varying_t         /* GL_TRANSFORM_FEEDBACK_VARYING */
{
    GLint name_length;
    variable_info_t variable_info;

    GLint transform_feedback_buffer_index;

    static constexpr GLenum tokens[] =
    {
        GL_NAME_LENGTH,
        variable_info_t::tokens,
        GL_TRANSFORM_FEEDBACK_BUFFER_INDEX
    };

};

struct transform_feedback_buffer_t          /* GL_TRANSFORM_FEEDBACK_BUFFER */
{
    buffer_info_t buffer_info;
    GLint transform_feedback_buffer_stride;

    static constexpr GLenum tokens[] =
    {
        buffer_info_t::tokens,
        GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE
    };
};

struct buffer_variable_t                    /* GL_BUFFER_VARIABLE */
{
    GLint name_length;
    variable_info_t variable_info;
    variable_layout_info_t variable_layout_info;
    top_level_array_info_t top_level_array_info;
    stages_info_t stages_info;

    static constexpr GLenum tokens[] =
    {
        GL_NAME_LENGTH,
        variable_info_t::tokens,
        variable_layout_info_t::tokens,
        top_level_array_info_t::tokens,
        stages_info_t::tokens
    };
};

struct shader_storage_block_t               /* GL_SHADER_STORAGE_BLOCK -- identical to GL_UNIFORM_BLOCK */
{
    GLint name_length;
    buffer_info_t buffer_info;
    GLint buffer_data_size;
    stages_info_t stages_info;

    static constexpr GLenum tokens[] =
    {
        GL_NAME_LENGTH,
        buffer_info_t::tokens,
        GL_BUFFER_DATA_SIZE,
        stages_info_t::tokens
    };
};


} // namespace glsl_interface

/*

struct glsl_program_interface_t
{
    std::vector<> input;
    std::vector<> output;

    glsl_program_interface_t(const glsl_program_t& program)
    {
        GLuint id = program.id;

        // GL_UNIFORM :: The query is targeted at the set of active uniforms within program.

        GLint uniform_count;
        glGetProgramInterfaceiv(id, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniform_count);

        const GLenum uniform_props[] = {
            GL_NAME_LENGTH,
            GL_TYPE,
            GL_ARRAY_SIZE,
            GL_OFFSET,
            GL_BLOCK_INDEX,
            GL_ARRAY_STRIDE,
            GL_MATRIX_STRIDE,
            GL_IS_ROW_MAJOR,
            GL_ATOMIC_COUNTER_BUFFER_INDEX,
            GL_REFERENCED_BY_VERTEX_SHADER,
            GL_REFERENCED_BY_TESS_CONTROL_SHADER,
            GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
            GL_REFERENCED_BY_GEOMETRY_SHADER,
            GL_REFERENCED_BY_FRAGMENT_SHADER,
            GL_REFERENCED_BY_COMPUTE_SHADER,
            GL_LOCATION
        }




              GL_LOCATION};


        const int MAX_SHADER_RESOURCE_NAME_LENGTH = 128;
        GLchar name[MAX_SHADER_RESOURCE_NAME_LENGTH];

        for (GLint u = 0; u != uniform_count; ++u)
        {
            glGetProgramResourceiv(id, GL_UNIFORM, u, )
            glGetProgramResourceName(program, GL_UNIFORM, i, MAX_SHADER_RESOURCE_NAME_LENGTH, 0, name);
            glGetProgramResourceiv(program, GL_UNIFORM, i, 2, props, 2, NULL, params);
        type_name = name;
        //std::cout << "Index " << i << std::endl;
        std::cout <<  "(" <<  type_name  << ")" << " locatoin: " << params[1] << std::endl;
    }


        glGetProgramResourceiv(program)

    }
}

 */

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, for program interface queries we need OpenGL 4.3 context
    // screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Program interface and binary", 4, 4, 3, 1920, 1080);

    //===================================================================================================================================================================================================================
    // Shader and uniform variables initialization
    //===================================================================================================================================================================================================================
    glsl_program_t point_sphere(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/sphere.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/sphere.fs"));

    point_sphere.enable();
    uniform_t uni_sphere_pv_matrix = point_sphere["projection_view_matrix"];
    uniform_t uni_sphere_spheres   = point_sphere["spheres"];
    uniform_t uni_sphere_time      = point_sphere["time"];

    //===================================================================================================================================================================================================================
    // Shader program binary retrieval step
    //===================================================================================================================================================================================================================

    //===================================================================================================================================================================================================================
    // Shader program interface query step
    //===================================================================================================================================================================================================================

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup :
    // * background color -- dark blue
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.0f, 0.08f, 0.0f);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        //===============================================================================================================================================================================================================
        // clear back buffer, process events and update timer
        //===============================================================================================================================================================================================================
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        //===============================================================================================================================================================================================================
        // done : increment frame counter and show back buffer
        //===============================================================================================================================================================================================================
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}

/*


void glsl_program_t::dump_interface()
{
    debug_msg()std::cout << "--------------------" << name << " Interface------------------------" << std::endl;
    GLint outputs = 0;
    glGetProgramInterfaceiv(program, GL_PROGRAM_INPUT,  GL_ACTIVE_RESOURCES, &outputs);
    static const GLenum props[] = {GL_TYPE, GL_LOCATION};
    GLint params[2];
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
*/
