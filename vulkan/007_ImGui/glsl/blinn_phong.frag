#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D model_tex;

layout(location = 0) in vec3 normal_ws;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 FragmentColor;

const vec3 l = vec3(0.707, 0.707, 0.0);

void main()
{
    vec3 n = normalize(normal_ws);

    float q = 0.5 + 0.5 * dot(l, n);

    FragmentColor = texture(model_tex, uv) * vec4(q, q, q, 1.0f);
}