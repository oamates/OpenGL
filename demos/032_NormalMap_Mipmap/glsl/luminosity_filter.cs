#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D diffuse_tex;
layout (rgba32f) uniform image2D luma_image;
uniform int tex_level;
uniform float gamma;
uniform float brightness;

uniform vec3 light;

const vec3 rgb_power_bt709 = vec3(0.2126f, 0.7152f, 0.0722f);
const vec3 unit_rgb_power_bt709 = normalize(rgb_power_bt709);

subroutine vec4 luminosity_filter_func(sampler2D sampler, vec2 uv, float lod);
subroutine uniform luminosity_filter_func luminosity_func;

//==============================================================================================================================================================
// different diffuse --> luminosity converting functions
//==============================================================================================================================================================
subroutine(luminosity_filter_func) vec4 luma_bt709_4c(sampler2D sampler, vec2 uv, float lod)
{
    vec3 c0 = pow(textureLod(sampler, uv, lod).rgb, vec3(gamma));
    vec3 c1 = pow(textureLod(sampler, uv, lod + 1.0).rgb, vec3(gamma));
 
    vec4 L;
    L.r = dot(c0, unit_rgb_power_bt709);
    vec3 s = c0 - L.r * unit_rgb_power_bt709;
    L.g = length(s);

    L.b = dot(c1, unit_rgb_power_bt709);
    s = c1 - L.b * unit_rgb_power_bt709;
    L.a = length(s);
    return L; 
}

subroutine(luminosity_filter_func) vec4 luma_bt709(sampler2D sampler, vec2 uv, float lod)
{
    vec3 c = textureLod(sampler, uv, lod).rgb;
    float L = dot(pow(c, vec3(gamma)), rgb_power_bt709);
    return vec4(L, 0.0, 0.0, 0.0); 
}

subroutine(luminosity_filter_func) vec4 luma_max(sampler2D sampler, vec2 uv, float lod)
{
    vec3 c = textureLod(sampler, uv, lod).rgb;
    float L = pow(max(c.r, max(c.g, c.b)), gamma);
    return vec4(L, 0.0, 0.0, 0.0); 
}

subroutine(luminosity_filter_func) vec4 luma_product(sampler2D sampler, vec2 uv, float lod)
{
    vec3 c = textureLod(sampler, uv, lod).rgb;
    vec3 gcc = pow(c, vec3(gamma));
    float L = 1.0 - (1.0 - gcc.r) * (1.0 - gcc.g) * (1.0 - gcc.b);
    return vec4(L, 0.0, 0.0, 0.0); 
}

vec2 texel_size;
subroutine(luminosity_filter_func) vec4 luma_laplace(sampler2D sampler, vec2 uv, float lod)
{
    const float LAPLACE_3x3_NORMALIZATION_FACTOR = 1.0f / 16.0f;

    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

    float bl = dot(pow(textureLod(sampler, vec2(uvm.x, uvm.y), lod).rgb, vec3(gamma)), rgb_power_bt709);             // bottom left
    float bc = dot(pow(textureLod(sampler, vec2(uv0.x, uvm.y), lod).rgb, vec3(gamma)), rgb_power_bt709);             // bottom center
    float br = dot(pow(textureLod(sampler, vec2(uvp.x, uvm.y), lod).rgb, vec3(gamma)), rgb_power_bt709);             // bottom right
    float cl = dot(pow(textureLod(sampler, vec2(uvm.x, uv0.y), lod).rgb, vec3(gamma)), rgb_power_bt709);             // center left
    float cc = dot(pow(textureLod(sampler, vec2(uvm.x, uv0.y), lod).rgb, vec3(gamma)), rgb_power_bt709);             // center center
    float cr = dot(pow(textureLod(sampler, vec2(uvp.x, uv0.y), lod).rgb, vec3(gamma)), rgb_power_bt709);             // center right
    float tl = dot(pow(textureLod(sampler, vec2(uvm.x, uvp.y), lod).rgb, vec3(gamma)), rgb_power_bt709);             // top left
    float tc = dot(pow(textureLod(sampler, vec2(uv0.x, uvp.y), lod).rgb, vec3(gamma)), rgb_power_bt709);             // top center
    float tr = dot(pow(textureLod(sampler, vec2(uvp.x, uvp.y), lod).rgb, vec3(gamma)), rgb_power_bt709);             // top right

    float L = 8.0 * cc - (bl + bc + br + cl + cr + tl + tc + tr);
    return vec4(LAPLACE_3x3_NORMALIZATION_FACTOR * L, 0.0, 0.0, 0.0);
}

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 Q = textureSize(diffuse_tex, tex_level);
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    if ((P.x >= Q.x) || (P.y >= Q.y)) return;

    texel_size = 1.0 / vec2(Q);
    float lod = tex_level;
    vec2 uv = (vec2(P) + 0.5) / vec2(Q);
    vec4 L = brightness * luminosity_func(diffuse_tex, uv, lod);

    imageStore(luma_image, P, L);
}