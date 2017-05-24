#version 330 core

in vec2 uv;

uniform sampler2D ssao_input;
uniform vec2 texel_size;

out float OcclusionBlurred;

void main()
{
    float occlusion = 0.0;

    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texel_size;
            occlusion += texture(ssao_input, uv + offset).r;
        }
    }

    OcclusionBlurred = 0.04 * occlusion;
}  