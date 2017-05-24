#version 400 core

in vec2 uv;

uniform sampler2D ssao_input;
uniform vec2 texel_size;

out float OcclusionBlurred;

void main()
{


    vec2 q = gl_FragCoord.xy;

    const float size = 2.0;

    vec4 c00_00 = textureGather(ssao_input, uv);

    vec4 cm1_m1 = textureGather(ssao_input, uv + texel_size * vec2(-size, -size));
    vec4 cm1_p1 = textureGather(ssao_input, uv + texel_size * vec2(-size,  size));
    vec4 cp1_m1 = textureGather(ssao_input, uv + texel_size * vec2( size, -size));
    vec4 cp1_p1 = textureGather(ssao_input, uv + texel_size * vec2( size,  size));
    vec4 c00_m1 = textureGather(ssao_input, uv + texel_size * vec2(  0.0, -size));
    vec4 c00_p1 = textureGather(ssao_input, uv + texel_size * vec2(  0.0,  size));
    vec4 cm1_00 = textureGather(ssao_input, uv + texel_size * vec2(-size,   0.0));
    vec4 cp1_00 = textureGather(ssao_input, uv + texel_size * vec2( size,   0.0));

    vec4 t = 12.0 * c00_00 + 4.0 * (c00_m1 + c00_p1 + cm1_00 + cp1_00) + 1.0 * (cm1_m1 + cm1_p1 + cp1_m1 + cp1_p1);

    float occlusion = dot(t, vec4(1.0)) / 128.0;

    OcclusionBlurred = occlusion;
    OcclusionBlurred = texture(ssao_input, uv).r;


/*

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
*/

}  