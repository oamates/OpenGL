#version 450
#extension GL_ARB_separate_shader_objects : enable

const vec2 uvs[4] = vec2[4] 
(
    vec2(-1.0f,  1.0f),
    vec2(-1.0f, -1.0f),
    vec2( 1.0f,  1.0f),
    vec2( 1.0f, -1.0f)
);

layout(binding = 0) uniform UniformBufferObject
{
    mat3 camera_matrix;
    vec3 camera_ws;
    vec3 light_ws;
    vec2 focal_scale;
    float time;
} ubo;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 view;

void main()
{
    uv = uvs[gl_VertexIndex];
    view = ubo.camera_matrix * vec3(ubo.focal_scale * uv, -1.0f);
    gl_Position = vec4(uv, 0.0f, 1.0f);
}