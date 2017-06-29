#version 430 core

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform uimageBuffer list_buffer;

out vec4 FragmentColor;

#define MAX_FRAGMENTS 24

uvec4 fragments[MAX_FRAGMENTS];
float depth[MAX_FRAGMENTS];

void main(void)
{
	ivec2 uv = ivec2(gl_FragCoord.xy);
    int fragment_count = 0;
    uint head = imageLoad(head_pointer_image, uv).x;

    while (head != 0 && (fragment_count < MAX_FRAGMENTS))
    {
        uvec4 fragment = imageLoad(list_buffer, int(head));
        float d = uintBitsToFloat(fragment.w);
        int k = fragment_count - 1;

        while (k >= 0 && (depth[k] < d))
        {
            depth[k + 1] = depth[k];
            fragments[k + 1] = fragments[k];
            k--;
        }

        fragments[k + 1] = fragment;
        depth[k + 1] = d;
        head = fragment.x;

        fragment_count++;
    }

    vec4 color = vec4(0.0);

    for (uint i = 0; i < fragment_count; i++)
    {
        vec4 modulator = unpackUnorm4x8(fragments[i].y);
        vec4 additive_component = unpackUnorm4x8(fragments[i].z);
        color = mix(color, modulator, modulator.a) + additive_component;
    }

    FragmentColor = color;
}