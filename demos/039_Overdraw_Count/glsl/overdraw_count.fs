#version 430 core

layout (binding = 0, r32ui) uniform uimage2D output_buffer;

void main(void)
{
    imageAtomicAdd(output_buffer, ivec2(gl_FragCoord.xy), 1);
}
