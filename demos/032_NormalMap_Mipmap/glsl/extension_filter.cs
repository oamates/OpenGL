#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D normal_tex;
layout (r32f) uniform image2D normal_ext_image;
uniform int tex_level;
uniform int radius;
uniform float sharpness;

const float sqrt_half = 0.70710678118f;
const float cos_pi8   = 0.92387953251f;
const float sin_pi8   = 0.38268343236f;

const vec2 right_16gon[16] = vec2[16]
(
    vec2(      1.0f,       0.0f),
    vec2(   cos_pi8,    sin_pi8),
    vec2( sqrt_half,  sqrt_half),
    vec2(   sin_pi8,    cos_pi8),
    vec2(      0.0f,       1.0f),
    vec2(  -sin_pi8,    cos_pi8),
    vec2(-sqrt_half,  sqrt_half),
    vec2(  -cos_pi8,    sin_pi8),
    vec2(     -1.0f,       0.0f),
    vec2(  -cos_pi8,   -sin_pi8),
    vec2(-sqrt_half, -sqrt_half),
    vec2(  -sin_pi8,   -cos_pi8),
    vec2(      0.0f,      -1.0f),
    vec2(   sin_pi8,   -cos_pi8),
    vec2( sqrt_half, -sqrt_half),
    vec2(   cos_pi8,   -sin_pi8)
);

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 Q = textureSize(normal_tex, tex_level);
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    if ((P.x >= Q.x) || (P.y >= Q.y)) return;

    vec2 texel_size = 1.0 / Q;
    vec2 uv0 = texel_size * (vec2(P) + 0.5);

    float lod = tex_level;
    vec3 n = vec3(0.0);

    for(int i = -radius; i <= radius; ++i)
    {
        for(int j = -radius; j <= radius; ++j)
        {
            vec2 d = texel_size * vec2(i, j);
            vec2 uv = uv0 + d;
            vec3 n0 = 2.0 * textureLod(normal_tex, uv, lod).rgb - 1.0;
            float l = pow(length(n0.xy), sharpness);
            n += l * n0;
        }
    }

    n = normalize(n);
    imageStore(normal_ext_image, P, vec4(0.5 + 0.5 * n, 1.0));
}