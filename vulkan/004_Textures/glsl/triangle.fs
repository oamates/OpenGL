#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4(color, 1.0) * texture(texSampler, uv);
}