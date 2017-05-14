#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject 
{
    mat3 camera_matrix;
    vec3 camera_ws;
    vec3 light_ws;
    vec2 focal_scale;
    float time;
} ubo;

layout(location = 0) in vec2 position_in;
layout(location = 1) in vec3 color_in;
layout(location = 2) in vec2 uv_in;

layout(location = 0) out vec3 color;
layout(location = 1) out vec2 uv;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() 
{
    gl_Position = vec4(position_in, 0.0, 1.0);
    color = color_in;
    uv = uv_in;
}