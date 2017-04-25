#version 330 core

in vec2 uv;

uniform sampler2D ssao_input;

out float OcclusionBlurred;

void main()
{
    vec2 texelSize = 1.0f / textureSize(ssao_input, 0).xy;
    float occlusion = 0.0;

    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            occlusion += texture(ssao_input, uv + offset).r;
        }
    }

    OcclusionBlurred = 0.04 * occlusion;
}  