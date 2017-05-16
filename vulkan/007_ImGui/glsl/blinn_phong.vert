#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 projection_matrix;
    mat4 view_matrix;
    mat4 model_matrix;
};

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec2 uv_in;

layout(location = 0) out vec3 normal_ws;
layout(location = 1) out vec2 uv;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position_in, 1.0);
    normal_ws = normal_in;
    uv = uv_in;
}