#version 330 core

in vec2 uv;

out vec4 ssao_blurred;

uniform sampler2D ssao_image;

const float offset[] = const float[] (-1.5f, -0.5f, 0.5f, 1.5f);

void main()
{
    vec3 average_color = vec3(0.0f);
    vec2 texture_size = textureSize(ssao_image, 0);

    for (int i = 0 ; i < 4 ; i++)
        for (int j = 0 ; j < 4 ; j++)
            average_color += texture(ssao_image, uv + vec2(offset[i], offset[j]) / texture_size).xyz;

    ssao_blurred = vec4(average_color / 16.0f, 1.0f);
}