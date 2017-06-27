#version 430 core

layout (binding = 0, r32ui) uniform uimage2D counter_image;

out vec4 FragmentColor;

void main(void)
{
    FragmentColor = vec4(imageLoad(counter_image, ivec2(gl_FragCoord.xy)).xxyx) / 64.0f;
}
