#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D diffuse_tex;
layout (r32f) uniform image2D luma_image;
uniform int tex_level;

const vec3 rgb_power_bt709 = vec3(0.2126f, 0.7152f, 0.0722f);

subroutine float luminosity_filter_func(vec3 rgb);
subroutine uniform luminosity_filter_func luminosity_func;


//==============================================================================================================================================================
// different diffuse --> luminosity converting functions
//==============================================================================================================================================================
subroutine(luminosity_filter_func) float luma_bt709(vec3 rgb)
    { return dot(rgb, rgb_power_bt709); }

subroutine(luminosity_filter_func) float luma_bt709_gc(vec3 rgb)
    { return dot(pow(rgb, vec3(2.2f)), rgb_power_bt709); }

subroutine(luminosity_filter_func) float luma_max(vec3 rgb)
    { return max(rgb.r, max(rgb.g, rgb.b)); }

subroutine(luminosity_filter_func) float luma_max_gc(vec3 rgb)
    { return pow(max(rgb.r, max(rgb.g, rgb.b)), 2.2); }

subroutine(luminosity_filter_func) float luma_product(vec3 rgb)
    { return 1.0 - (1.0 - rgb.r) * (1.0 - rgb.g) * (1.0 - rgb.b); }

subroutine(luminosity_filter_func) float luma_product_gc(vec3 rgb)
{
    vec3 rgb_gc = pow(rgb, vec3(2.2));
    return 1.0 - (1.0 - rgb_gc.r) * (1.0 - rgb_gc.g) * (1.0 - rgb_gc.b);
}

subroutine(luminosity_filter_func) float luma_laplace(vec3 rgb)
{
    vec3 rgb_gc = pow(rgb, vec3(2.2));
    return 1.0 - (1.0 - rgb_gc.r) * (1.0 - rgb_gc.g) * (1.0 - rgb_gc.b);
}

subroutine(luminosity_filter_func) float luma_laplace_gc(vec3 rgb)
{
    vec3 rgb_gc = pow(rgb, vec3(2.2));
    return 1.0 - (1.0 - rgb_gc.r) * (1.0 - rgb_gc.g) * (1.0 - rgb_gc.b);
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
    float l = luminosity_func(c);

    imageStore(luma_image, P, vec4(l, 0.0, 0.0, 0.0));
}