#version 430 core

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform uimageBuffer list_buffer;

out vec4 FragmentColor;

#define MAX_FRAGMENTS 40

uvec4 fragment_list[MAX_FRAGMENTS];

void main(void)
{
    uint fragment_count = 0;
    uint current_index = imageLoad(head_pointer_image, ivec2(gl_FragCoord).xy).x;

    while (current_index != 0 && (fragment_count < MAX_FRAGMENTS))
    {
        uvec4 fragment = imageLoad(list_buffer, int(current_index));
        fragment_list[fragment_count] = fragment;
        current_index = fragment.x;
        fragment_count++;
    };

    if (fragment_count > 1)
    {
        for (uint i = 0; i < fragment_count - 1; i++)
        {
            for (uint j = i + 1; j < fragment_count; j++)
            {
                uvec4 fragment1 = fragment_list[i];
                uvec4 fragment2 = fragment_list[j];

                float depth1 = uintBitsToFloat(fragment1.z);
                float depth2 = uintBitsToFloat(fragment2.z);

                if (depth1 < depth2)
                {
                    fragment_list[i] = fragment2;
                    fragment_list[j] = fragment1;
                };
            };
        };
    };

    vec4 color = vec4(0.0);

    for (uint i = 0; i < fragment_count; i++)
    {
        vec4 modulator = unpackUnorm4x8(fragment_list[i].y);
        vec4 additive_component = unpackUnorm4x8(fragment_list[i].w);

        color = mix(color, modulator, modulator.a) + additive_component;
    };

    FragmentColor = color;
}
