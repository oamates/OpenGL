#version 430 core


layout(location = 0) in vec4 position_ms;


layout (std430, binding = 0) buffer shader_data
{
    mat4 model_matrix[];
};

out vec4 position_ws;

void main()
{
    position_ws = model_matrix[gl_InstanceID] * position_ms;
}
