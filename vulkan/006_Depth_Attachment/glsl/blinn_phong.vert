#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 projection_view_matrix;
    vec4 camera_ws4;
    vec4 light_ws4;
};

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec2 uv_in;

layout(location = 0) out vec3 position_ws;
layout(location = 1) out vec3 normal_ws;
layout(location = 2) out vec2 uv;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    position_ws = position_in;
    normal_ws = normal_in;
    uv = uv_in;

    gl_Position = projection_view_matrix * vec4(position_in, 1.0);
}
