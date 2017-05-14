#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 camera_matrix;
    vec4 light_ws4;
    vec2 focal_scale;
};

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 view;

out gl_PerVertex
{
    vec4 gl_Position;
};

const vec2 ndc[4] = vec2[4]
(
    vec2(-1.0f, -1.0f),
    vec2(-1.0f,  1.0f),
    vec2( 1.0f, -1.0f),
    vec2( 1.0f,  1.0f)
);

void main() 
{
    uv = ndc[gl_VertexIndex];
    view = vec3(camera_matrix * vec4(focal_scale * uv, -1.0f, 0.0f));
    gl_Position = vec4(uv, 0.0, 1.0);
}