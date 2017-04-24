#version 430 core

layout (binding = 0, r32ui) uniform uimage2D output_image;

layout (location = 0) out vec4 FragmentColor;

void main(void)
{
    FragmentColor = vec4(imageLoad(output_image, ivec2(gl_FragCoord.xy)).xxxx) / 64.0f;
}
