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

//=======================================================================================================================================================================================================================
// Auxiliary structures
//=======================================================================================================================================================================================================================
struct stages_info_t
{
    GLint vertex;
    GLint tess_control;
    GLint tess_evaluation;
    GLint geometry;
    GLint fragment;
    GLint compute;

    void print()
    {
        printf("\t\tShader stages : ");
        if (vertex) printf("%s", "VERTEX");
        if (tess_control) printf("%s", "TESS_CONTROL");
        if (tess_evaluation) printf("%s", "TESS_EVALUATION");
        if (geometry) printf("%s", "GEOMETRY");
        if (fragment) printf("%s", "FRAGMENT");
        if (compute) printf("%s", "COMPUTE");
        printf("\n");
    }

    // static constexpr GLenum tokens[] = { GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER };
};

struct buffer_info_t
{
    GLint active_variables;
    GLint buffer_binding;
    GLint num_active_variables;

    void print()
    {
        printf("\t\tBuffer info : binding = %d, number of active variables = %d\n", buffer_binding, num_active_variables);
    }

    // static GLenum tokens[] = { GL_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES };
};

struct io_attribute_info_t
{
    GLint type;
    GLint location;
    GLint location_component;
    GLint array_size;


    void print()
    {
        printf("\t\tAttribute info : type = %d, location = %d, location component = %d, array_size = %d\n", type, location, location_component, array_size);
    }

    // static GLenum tokens[] = { GL_TYPE, GL_LOCATION, GL_LOCATION_COMPONENT, GL_ARRAY_SIZE };
};

struct variable_info_t
{
    GLint type;
    GLint offset;
    GLint array_size;

    void print()
    {
        printf("\t\tVariable info : type = %d, offset = %d, array_size = %d\n", type, offset, array_size);
    }

    // static GLenum tokens[] = { GL_TYPE, GL_OFFSET, GL_ARRAY_SIZE };
};

struct variable_layout_info_t
{
    GLint array_stride;
    GLint block_index;
    GLint is_row_major;
    GLint matrix_stride;

    void print()
    {
        printf("\t\tVariable layout info : array_stride = %d, block_index = %d, is_row_major = %d, matrix_stride = %d\n", array_stride, block_index, is_row_major, matrix_stride);
    }

    // static GLenum tokens[] = { GL_ARRAY_STRIDE, GL_BLOCK_INDEX, GL_IS_ROW_MAJOR, GL_MATRIX_STRIDE };
};


struct top_level_array_info_t
{
    GLint size;
    GLint stride;

    void print()
    {
        printf("\t\tTop level array info : size = %d, stride = %d\n", size, stride);
    }

    // static GLenum tokens[] = { GL_TOP_LEVEL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_STRIDE };
};


//=======================================================================================================================================================================================================================
// Interface info structures
//=======================================================================================================================================================================================================================

struct uniform_t                            /* GL_UNIFORM */
{
    GLint name_length;
    variable_info_t variable_info;
    variable_layout_info_t variable_layout_info;
    GLint location;
    GLint atomic_counter_buffer_index;
    stages_info_t stages_info;

    int name_len()
        { return name_length; }

    void print()
    {
        variable_info.print();
        variable_layout_info.print();
        printf("\t\tLocation = %d", location);
        printf("\t\tAtomic counter buffer index = %d", atomic_counter_buffer_index);
        stages_info.print();
    }

    static GLenum tokens[];
};

GLenum uniform_t::tokens[] =
{
    GL_NAME_LENGTH,
    GL_TYPE, GL_OFFSET, GL_ARRAY_SIZE,
    GL_ARRAY_STRIDE, GL_BLOCK_INDEX, GL_IS_ROW_MAJOR, GL_MATRIX_STRIDE,
    GL_LOCATION,
    GL_ATOMIC_COUNTER_BUFFER_INDEX,
    GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER
};

struct uniform_block_t                      /* GL_UNIFORM_BLOCK */
{
    GLint name_length;
    buffer_info_t buffer_info;
    GLint buffer_data_size;
    stages_info_t stages_info;

    int name_len()
        { return name_length; }

    void print()
    {
        buffer_info.print();
        printf("\t\tBuffer data size = %d", buffer_data_size);
        stages_info.print();
    }

    static GLenum tokens[];
};

GLenum uniform_block_t::tokens[] =
{
    GL_NAME_LENGTH,
    GL_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES,
    GL_BUFFER_DATA_SIZE,
    GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER
};


struct atomic_counter_buffer_t              /* GL_ATOMIC_COUNTER_BUFFER */
{
    buffer_info_t buffer_info;
    GLint buffer_data_size;
    stages_info_t stages_info;

    int name_len()
        { return 0; }


    void print()
    {
        buffer_info.print();
        printf("\t\tBuffer data size = %d", buffer_data_size);
        stages_info.print();
    }

    static GLenum tokens[];
};

GLenum atomic_counter_buffer_t::tokens[] =
{
    GL_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES,
    GL_BUFFER_DATA_SIZE,
    GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER
};

struct program_input_t                      /* GL_PROGRAM_INPUT */
{
    GLint name_length;
    io_attribute_info_t io_attribute_info;
    GLint is_per_patch;
    stages_info_t stages_info;

    int name_len()
        { return name_length; }

    void print()
    {
        io_attribute_info.print();
        printf("\t\tPer patch input = %d", is_per_patch);
        stages_info.print();
    }

    static GLenum tokens[];
};

GLenum program_input_t::tokens[] =
{
    GL_NAME_LENGTH,
    GL_TYPE, GL_LOCATION, GL_LOCATION_COMPONENT, GL_ARRAY_SIZE,
    GL_IS_PER_PATCH,
    GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER
};

struct program_output_t                     /* GL_PROGRAM_OUTPUT */
{
    GLint name_length;
    io_attribute_info_t io_attribute_info;
    GLint location_index;
    GLint is_per_patch;
    stages_info_t stages_info;

    int name_len()
        { return name_length; }

    void print()
    {
        io_attribute_info.print();
        printf("\t\tLocation index = %d", location_index);
        printf("\t\tPer patch input = %d", is_per_patch);
        stages_info.print();
    }

    static GLenum tokens[];
};

GLenum program_output_t::tokens[] =
{
    GL_NAME_LENGTH,
    GL_TYPE, GL_LOCATION, GL_LOCATION_COMPONENT, GL_ARRAY_SIZE,
    GL_LOCATION_INDEX,
    GL_IS_PER_PATCH,
    GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER
};


struct subroutine_t                         /* GL_VERTEX_SUBROUTINE, GL_TESS_CONTROL_SUBROUTINE, GL_TESS_EVALUATION_SUBROUTINE, GL_GEOMETRY_SUBROUTINE, GL_FRAGMENT_SUBROUTINE, GL_COMPUTE_SUBROUTINE */
{
    GLint name_length;

    int name_len()
        { return name_length; }

    void print()
    {
    }

    static GLenum tokens[];
};

GLenum subroutine_t::tokens[] =
{
    GL_NAME_LENGTH
};


struct subroutine_uniform_t                 /* GL_VERTEX_SUBROUTINE_UNIFORM, GL_TESS_CONTROL_SUBROUTINE_UNIFORM, GL_TESS_EVALUATION_SUBROUTINE_UNIFORM, GL_GEOMETRY_SUBROUTINE_UNIFORM, GL_FRAGMENT_SUBROUTINE_UNIFORM, GL_COMPUTE_SUBROUTINE_UNIFORM */
{
    GLint name_length;
    GLint location;
    GLint compatible_subroutines;
    GLint num_compatible_subroutines;
    GLint array_size;

    int name_len()
        { return name_length; }

    void print()
    {
        printf("\t\tLocation = %d", location);
        printf("\t\tCompatible subroutines = %d", compatible_subroutines);
        printf("\t\tNumber of compatible subroutines = %d", num_compatible_subroutines);
        printf("\t\tArray size = %d", array_size);
    }

    static GLenum tokens[];
};

GLenum subroutine_uniform_t::tokens[] =
{
    GL_NAME_LENGTH,
    GL_LOCATION,
    GL_COMPATIBLE_SUBROUTINES,
    GL_NUM_COMPATIBLE_SUBROUTINES,
    GL_ARRAY_SIZE
};


struct transform_feedback_varying_t         /* GL_TRANSFORM_FEEDBACK_VARYING */
{
    GLint name_length;
    variable_info_t variable_info;

    GLint transform_feedback_buffer_index;

    int name_len()
        { return name_length; }


    void print()
    {
        variable_info.print();
    }

    static GLenum tokens[];
};

GLenum transform_feedback_varying_t::tokens[] =
{
    GL_NAME_LENGTH,
    GL_TYPE, GL_OFFSET, GL_ARRAY_SIZE,
    GL_TRANSFORM_FEEDBACK_BUFFER_INDEX
};

struct transform_feedback_buffer_t          /* GL_TRANSFORM_FEEDBACK_BUFFER */
{
    buffer_info_t buffer_info;
    GLint transform_feedback_buffer_stride;

    int name_len()
        { return 0; }

    void print()
    {
        buffer_info.print();
        printf("Transform feedback buffer stride = %d", transform_feedback_buffer_stride);
    }

    static GLenum tokens[];
};

GLenum transform_feedback_buffer_t::tokens[] =
{
    GL_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES,
    GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE
};

struct buffer_variable_t                    /* GL_BUFFER_VARIABLE */
{
    GLint name_length;
    variable_info_t variable_info;
    variable_layout_info_t variable_layout_info;
    top_level_array_info_t top_level_array_info;
    stages_info_t stages_info;

    int name_len()
        { return name_length; }

    void print()
    {
        variable_info.print();
        variable_layout_info.print();
        top_level_array_info.print();
        stages_info.print();
    }

    static GLenum tokens[];
};

GLenum buffer_variable_t::tokens[] =
{
    GL_NAME_LENGTH,
    GL_TYPE, GL_OFFSET, GL_ARRAY_SIZE,
    GL_ARRAY_STRIDE, GL_BLOCK_INDEX, GL_IS_ROW_MAJOR, GL_MATRIX_STRIDE,
    GL_TOP_LEVEL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_STRIDE,
    GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER
};


struct shader_storage_block_t               /* GL_SHADER_STORAGE_BLOCK -- identical to GL_UNIFORM_BLOCK */
{
    GLint name_length;
    buffer_info_t buffer_info;
    GLint buffer_data_size;
    stages_info_t stages_info;

    int name_len()
        { return name_length; }

    void print()
    {
        buffer_info.print();
        printf("Buffer data size = %d", buffer_data_size);
        stages_info.print();
    }

    static GLenum tokens[];
};

GLenum shader_storage_block_t::tokens[] =
{
    GL_NAME_LENGTH,
    GL_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES,
    GL_BUFFER_DATA_SIZE,
    GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER
};

template<GLenum token, typename interface_t> void print_interface_info(const char* interface_name, const glsl_program_t& program)
{
    const int MAX_SHADER_RESOURCE_NAME_LENGTH = 256;
    char name[MAX_SHADER_RESOURCE_NAME_LENGTH];

    GLuint id = program.id;

    GLint resource_count;
    glGetProgramInterfaceiv(id, token, GL_ACTIVE_RESOURCES, &resource_count);

    printf("\n\t%s :: %u\n", interface_name, resource_count);

    if (resource_count == 0)
        return;

    GLsizei prop_count = sizeof(interface_t) / sizeof(GLint);

    for (GLint i = 0; i != resource_count; ++i)
    {
        GLsizei length;
        interface_t interface;
        glGetProgramResourceiv(id, token, i, prop_count, interface_t::tokens, sizeof(interface_t), &length, (GLint *) &interface);

        int name_length = interface.name_len();
        if (name_length > 0)
        {
            GLsizei bufSize = std::min(name_length + 1, MAX_SHADER_RESOURCE_NAME_LENGTH);
            glGetProgramResourceName(id, token, i, bufSize, 0, name);
            printf("%s : ", name);
        }
        else
            printf("#%u : ", i);

        interface.print();
    }
}

void program_interfaces_info(const glsl_program_t& program)
{
    printf("Shader program %u interface :: \n\n", program.id);

    print_interface_info<GL_UNIFORM, uniform_t>("Uniform variables", program);
    print_interface_info<GL_UNIFORM_BLOCK, uniform_block_t>("Uniform blocks", program);
    print_interface_info<GL_ATOMIC_COUNTER_BUFFER, atomic_counter_buffer_t>("Atomic counter buffers", program);
    print_interface_info<GL_PROGRAM_INPUT, program_input_t>("Program inputs", program);
    print_interface_info<GL_PROGRAM_OUTPUT, program_output_t>("Program outputs", program);

    print_interface_info<GL_VERTEX_SUBROUTINE, subroutine_t>("Vertex shader subroutines", program);
    print_interface_info<GL_TESS_CONTROL_SUBROUTINE, subroutine_t>("Tesselation control shader subroutines", program);
    print_interface_info<GL_TESS_EVALUATION_SUBROUTINE, subroutine_t>("Tesselation evaluation shader subroutines", program);
    print_interface_info<GL_GEOMETRY_SUBROUTINE, subroutine_t>("Geometry shader subroutines", program);
    print_interface_info<GL_FRAGMENT_SUBROUTINE, subroutine_t>("Fragment shader subroutines", program);
    print_interface_info<GL_COMPUTE_SUBROUTINE, subroutine_t>("Compute shader subroutines", program);

    print_interface_info<GL_VERTEX_SUBROUTINE_UNIFORM, subroutine_uniform_t>("Vertex shader subroutine uniforms", program);
    print_interface_info<GL_TESS_CONTROL_SUBROUTINE_UNIFORM, subroutine_uniform_t>("Tesselation control shader subroutine uniforms", program);
    print_interface_info<GL_TESS_EVALUATION_SUBROUTINE_UNIFORM, subroutine_uniform_t>("Tesselation evaluation shader subroutine uniforms", program);
    print_interface_info<GL_GEOMETRY_SUBROUTINE_UNIFORM, subroutine_uniform_t>("Geometry shader subroutine uniforms", program);
    print_interface_info<GL_FRAGMENT_SUBROUTINE_UNIFORM, subroutine_uniform_t>("Fragment shader subroutine uniforms", program);
    print_interface_info<GL_COMPUTE_SUBROUTINE_UNIFORM, subroutine_uniform_t>("Compute shader subroutine uniforms", program);

    print_interface_info<GL_TRANSFORM_FEEDBACK_VARYING, transform_feedback_varying_t>("Transform feedback varyings", program);
    print_interface_info<GL_TRANSFORM_FEEDBACK_BUFFER, transform_feedback_buffer_t>("Transform feedback buffers", program);
    print_interface_info<GL_BUFFER_VARIABLE, buffer_variable_t>("Buffer variables", program);
    print_interface_info<GL_SHADER_STORAGE_BLOCK, shader_storage_block_t>("Shader storage blocks", program);
}


} // namespace glsl_interface

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

    glsl_shader_t vs(GL_VERTEX_SHADER, "glsl/sphere.vs");

    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vs.id);

    glProgramParameteri(program_id, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);                   // Set the binary retrievable hint and link the program
    glProgramParameteri(program_id, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glLinkProgram(program_id);

    GLint binary_size = 0;                                                                          // Get the expected size of the program binary
    glGetProgramiv(program_id, GL_PROGRAM_BINARY_LENGTH, &binary_size);

    void* program_binary = malloc(binary_size);                                                     // Allocate some memory to store the program binary

    GLenum binary_format = GL_NONE;                                                                 // Now retrieve the binary from the program object
    glGetProgramBinary(program_id, binary_size, 0, &binary_format, program_binary);

    printf("\n\nGot program binary :: size = %u, format = %u", binary_size, binary_format);

    FILE* f = fopen("program.bin", "wb");
    fwrite(program_binary, binary_size, 1, f);
    fclose(f);


    GLint num_shader_binary_formats;
    GLint num_program_binary_formats;

    glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &num_shader_binary_formats);
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num_program_binary_formats);

    printf("\t\t# supported shader binary formats : %u\n", num_shader_binary_formats);
    printf("\t\t# supported program binary formats : %u\n", num_program_binary_formats);

    GLint* binary_formats = (GLint*) malloc(num_program_binary_formats * sizeof(GLint));
    glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, binary_formats);

    for (GLint f = 0; f < num_program_binary_formats; ++f)
    {
        printf("\t\t\t%d\n", binary_formats[f]);
    }


    //===================================================================================================================================================================================================================
    // Shader program interface query step
    //===================================================================================================================================================================================================================

    glsl_interface::program_interfaces_info(point_sphere);

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup :
    // * background color -- dark blue
    //===================================================================================================================================================================================================================
    glClearColor(0.01f, 0.0f, 0.08f, 0.0f);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
/*
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
*/
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}
