#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D diffuse_tex;
layout (r32f) uniform image2D luma_image;
uniform int tex_level;

const vec3 rgb_power_bt709 = vec3(0.2126f, 0.7152f, 0.0722f);

float luma_bt709(vec3 rgb)
    { return dot(rgb, rgb_power_bt709); }

float luma_bt709_gc(vec3 rgb)
    { return dot(pow(rgb, vec3(2.2f)), rgb_power_bt709); }

float luma(vec3 rgb)
{
    return luma_bt709_gc(rgb);
}

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 Q = textureSize(diffuse_tex, tex_level);
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    if ((P.x >= Q.x) || (P.y >= Q.y)) return;

    float lod = tex_level;
    vec2 uv = (vec2(P) + 0.5) / vec2(Q);
    vec3 c = textureLod(diffuse_tex, uv, lod).rgb;
    float l = luma(c);

    imageStore(luma_image, P, vec4(l, 0.0, 0.0, 0.0));
}