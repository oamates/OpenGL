#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D luma_tex;
layout (rgba32f) uniform image2D normal_image;

uniform float inv_amplitude;
uniform int tex_level;

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 Q = textureSize(luma_tex, tex_level);
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    if ((P.x >= Q.x) || (P.y >= Q.y)) return;

    vec2 texel_size = 1.0 / Q;
    vec2 uv0 = texel_size * (vec2(P) + 0.5);
    vec2 uvp = uv0 + texel_size;
    vec2 uvm = uv0 - texel_size;

    float lod = tex_level;
    float bl = textureLod(luma_tex, vec2(uvm.x, uvm.y), lod).r;             // bottom left
    float bc = textureLod(luma_tex, vec2(uv0.x, uvm.y), lod).r;             // bottom center
    float br = textureLod(luma_tex, vec2(uvp.x, uvm.y), lod).r;             // bottom right
    float cl = textureLod(luma_tex, vec2(uvm.x, uv0.y), lod).r;             // center left
    float cr = textureLod(luma_tex, vec2(uvp.x, uv0.y), lod).r;             // center right
    float tl = textureLod(luma_tex, vec2(uvm.x, uvp.y), lod).r;             // top left
    float tc = textureLod(luma_tex, vec2(uv0.x, uvp.y), lod).r;             // top center
    float tr = textureLod(luma_tex, vec2(uvp.x, uvp.y), lod).r;             // top right

    /* Sobel filter */
    /*
    float dL_dx = (br + 2.0 * cr + tr) - (bl + 2.0 * cl + tl);
    float dL_dy = (tl + 2.0 * tc + tr) - (bl + 2.0 * bc + br);
    dL_dx *= 0.25;
    dL_dy *= 0.25;
    */

    /* Scharr filter */
    float dL_dx = (3.0 * br + 10.0 * cr + 3.0 * tr) - (3.0 * bl + 10.0 * cl + 3.0 * tl);
    float dL_dy = (3.0 * tl + 10.0 * tc + 3.0 * tr) - (3.0 * bl + 10.0 * bc + 3.0 * br);
    dL_dx *= 0.0625;
    dL_dy *= 0.0625;

    vec3 n = vec3(-dL_dx, -dL_dy, inv_amplitude);
    n = normalize(n);

    imageStore(normal_image, P, vec4(0.5 + 0.5 * n, 1.0));
}