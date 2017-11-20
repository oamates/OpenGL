#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D heightmap_tex;
layout (rgba32f) uniform image2D normal_disp_image;

uniform float amplitude;
const float lod = 0.0f;

subroutine vec4 normal_disp_filter_func(sampler2D sampler, vec2 uv, vec2 texel_size);
subroutine uniform normal_disp_filter_func normal_disp_func;

//==============================================================================================================================================================
// Symmetric difference filter : 4 texture reads
//==============================================================================================================================================================
subroutine(normal_disp_filter_func) vec4 symm_diff(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float SYMM_DIFF_NORMALIZATION_FACTOR = 1.0f / 2.0f;

    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

    float cc = textureLod(sampler, uv0, lod).r;
    float bc = textureLod(sampler, vec2(uv0.x, uvm.y), lod).r;             // bottom center
    float cl = textureLod(sampler, vec2(uvm.x, uv0.y), lod).r;             // center left
    float cr = textureLod(sampler, vec2(uvp.x, uv0.y), lod).r;             // center right
    float tc = textureLod(sampler, vec2(uv0.x, uvp.y), lod).r;             // top center

    vec2 dL = vec2(cr - cl, tc - bc);
    dL *= (amplitude * SYMM_DIFF_NORMALIZATION_FACTOR);
    vec3 n = vec3(dL, 1.0);
    n = 0.5 + 0.5 * normalize(n);
    return vec4(n, cc);
}

//==============================================================================================================================================================
// Sobel 3x3 filter : 8 texture reads
//==============================================================================================================================================================
subroutine(normal_disp_filter_func) vec4 sobel3x3(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float SOBEL_3x3_NORMALIZATION_FACTOR = 1.0f / 8.0f;

    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

    float cc = textureLod(sampler, uv0, lod).r;
    float bl = textureLod(sampler, vec2(uvm.x, uvm.y), lod).r;             // bottom left
    float bc = textureLod(sampler, vec2(uv0.x, uvm.y), lod).r;             // bottom center
    float br = textureLod(sampler, vec2(uvp.x, uvm.y), lod).r;             // bottom right
    float cl = textureLod(sampler, vec2(uvm.x, uv0.y), lod).r;             // center left
    float cr = textureLod(sampler, vec2(uvp.x, uv0.y), lod).r;             // center right
    float tl = textureLod(sampler, vec2(uvm.x, uvp.y), lod).r;             // top left
    float tc = textureLod(sampler, vec2(uv0.x, uvp.y), lod).r;             // top center
    float tr = textureLod(sampler, vec2(uvp.x, uvp.y), lod).r;             // top right

    vec2 dL;
    dL.x = (br - bl) + 2.0 * (cr - cl) + (tr - tl);
    dL.y = (tl - bl) + 2.0 * (tc - bc) + (tr - br);
    dL *= (amplitude * SOBEL_3x3_NORMALIZATION_FACTOR);
    vec3 n = vec3(dL, 1.0);
    n = 0.5 + 0.5 * normalize(n);
    return vec4(n, cc);
}

//==============================================================================================================================================================
// Sobel 5x5 filter : 24 texture reads
//==============================================================================================================================================================
subroutine(normal_disp_filter_func) vec4 sobel5x5(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float SOBEL_5x5_NORMALIZATION_FACTOR = 1.0f / 96.0f;

    vec2 uv00 = uv;
    vec2 uvp1 = uv + texel_size;
    vec2 uvm1 = uv - texel_size;
    vec2 uvp2 = uvp1 + texel_size;
    vec2 uvm2 = uvm1 - texel_size;

    float oo_oo = textureLod(sampler, uv00, lod).r;
    float m2_p2 = textureLod(sampler, vec2(uvm2.x, uvp2.y), lod).r;
    float m2_p1 = textureLod(sampler, vec2(uvm2.x, uvp1.y), lod).r;
    float m2_oo = textureLod(sampler, vec2(uvm2.x, uv00.y), lod).r;
    float m2_m1 = textureLod(sampler, vec2(uvm2.x, uvm1.y), lod).r;
    float m2_m2 = textureLod(sampler, vec2(uvm2.x, uvm2.y), lod).r;
    float m1_p2 = textureLod(sampler, vec2(uvm1.x, uvp2.y), lod).r;
    float m1_p1 = textureLod(sampler, vec2(uvm1.x, uvp1.y), lod).r;
    float m1_oo = textureLod(sampler, vec2(uvm1.x, uv00.y), lod).r;
    float m1_m1 = textureLod(sampler, vec2(uvm1.x, uvm1.y), lod).r;
    float m1_m2 = textureLod(sampler, vec2(uvm1.x, uvm2.y), lod).r;
    float oo_p2 = textureLod(sampler, vec2(uv00.x, uvp2.y), lod).r;
    float oo_p1 = textureLod(sampler, vec2(uv00.x, uvp1.y), lod).r;
    float oo_m1 = textureLod(sampler, vec2(uv00.x, uvm1.y), lod).r;
    float oo_m2 = textureLod(sampler, vec2(uv00.x, uvm2.y), lod).r;
    float p1_p2 = textureLod(sampler, vec2(uvp1.x, uvp2.y), lod).r;
    float p1_p1 = textureLod(sampler, vec2(uvp1.x, uvp1.y), lod).r;
    float p1_oo = textureLod(sampler, vec2(uvp1.x, uv00.y), lod).r;
    float p1_m1 = textureLod(sampler, vec2(uvp1.x, uvm1.y), lod).r;
    float p1_m2 = textureLod(sampler, vec2(uvp1.x, uvm2.y), lod).r;
    float p2_p2 = textureLod(sampler, vec2(uvp2.x, uvp2.y), lod).r;
    float p2_p1 = textureLod(sampler, vec2(uvp2.x, uvp1.y), lod).r;
    float p2_oo = textureLod(sampler, vec2(uvp2.x, uv00.y), lod).r;
    float p2_m1 = textureLod(sampler, vec2(uvp2.x, uvm1.y), lod).r;
    float p2_m2 = textureLod(sampler, vec2(uvp2.x, uvm2.y), lod).r;

    vec2 dL;
    dL.x =       (p2_m2 - m2_m2 + p2_p2 - m2_p2) + 
           4.0 * (p2_m1 - m2_m1 + p2_p1 - m2_p1) + 
           6.0 * (p2_oo - m2_oo) + 
           2.0 * (p1_m2 - m1_m2 + p1_p2 - m1_p2) + 
           8.0 * (p1_m1 - m1_m1 + p1_p1 - m1_p1) + 
          12.0 * (p1_oo - m1_oo);

    dL.y =       (m2_p2 - m2_m2 + p2_p2 - p2_m2) + 
           4.0 * (m1_p2 - m1_m2 + p1_p2 - p1_m2) + 
           6.0 * (oo_p2 - oo_m2) + 
           2.0 * (m2_p1 - m2_m1 + p2_p1 - p2_m1) + 
           8.0 * (m1_p1 - m1_m1 + p1_p1 - p1_m1) + 
          12.0 * (oo_p1 - oo_m1);

    dL *= (amplitude * SOBEL_5x5_NORMALIZATION_FACTOR);
    vec3 n = vec3(dL, 1.0);
    n = 0.5 + 0.5 * normalize(n);
    return vec4(n, oo_oo);
}

//==============================================================================================================================================================
// Scharr 3x3 filter : 8 texture reads
//==============================================================================================================================================================
subroutine(normal_disp_filter_func) vec4 scharr3x3(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float SCHARR_3x3_NORMALIZATION_FACTOR = 1.0f / 32.0f;

    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

    float cc = textureLod(sampler, uv0, lod).r;
    float bl = textureLod(sampler, vec2(uvm.x, uvm.y), lod).r;             // bottom left
    float bc = textureLod(sampler, vec2(uv0.x, uvm.y), lod).r;             // bottom center
    float br = textureLod(sampler, vec2(uvp.x, uvm.y), lod).r;             // bottom right
    float cl = textureLod(sampler, vec2(uvm.x, uv0.y), lod).r;             // center left
    float cr = textureLod(sampler, vec2(uvp.x, uv0.y), lod).r;             // center right
    float tl = textureLod(sampler, vec2(uvm.x, uvp.y), lod).r;             // top left
    float tc = textureLod(sampler, vec2(uv0.x, uvp.y), lod).r;             // top center
    float tr = textureLod(sampler, vec2(uvp.x, uvp.y), lod).r;             // top right

    vec2 dL;
    dL.x = 3.0 * (br - bl) + 10.0 * (cr - cl) + 3.0 * (tr - tl);
    dL.y = 3.0 * (tl - bl) + 10.0 * (tc - bc) + 3.0 * (tr - br);
    dL *= (amplitude * SCHARR_3x3_NORMALIZATION_FACTOR);
    vec3 n = vec3(dL, 1.0);
    n = 0.5 + 0.5 * normalize(n);
    return vec4(n, cc);
}

//==============================================================================================================================================================
// Scharr 5x5 filter : 24 texture reads
//==============================================================================================================================================================
subroutine(normal_disp_filter_func) vec4 scharr5x5(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float SCHARR_5x5_NORMALIZATION_FACTOR = 1.0f / 42.0f;

    vec2 uv00 = uv;
    vec2 uvp1 = uv + texel_size;
    vec2 uvm1 = uv - texel_size;
    vec2 uvp2 = uvp1 + texel_size;
    vec2 uvm2 = uvm1 - texel_size;

    float oo_oo = textureLod(sampler, uv00, lod).r;
    float m2_p2 = textureLod(sampler, vec2(uvm2.x, uvp2.y), lod).r;
    float m2_p1 = textureLod(sampler, vec2(uvm2.x, uvp1.y), lod).r;
    float m2_oo = textureLod(sampler, vec2(uvm2.x, uv00.y), lod).r;
    float m2_m1 = textureLod(sampler, vec2(uvm2.x, uvm1.y), lod).r;
    float m2_m2 = textureLod(sampler, vec2(uvm2.x, uvm2.y), lod).r;
    float m1_p2 = textureLod(sampler, vec2(uvm1.x, uvp2.y), lod).r;
    float m1_p1 = textureLod(sampler, vec2(uvm1.x, uvp1.y), lod).r;
    float m1_oo = textureLod(sampler, vec2(uvm1.x, uv00.y), lod).r;
    float m1_m1 = textureLod(sampler, vec2(uvm1.x, uvm1.y), lod).r;
    float m1_m2 = textureLod(sampler, vec2(uvm1.x, uvm2.y), lod).r;
    float oo_p2 = textureLod(sampler, vec2(uv00.x, uvp2.y), lod).r;
    float oo_p1 = textureLod(sampler, vec2(uv00.x, uvp1.y), lod).r;
    float oo_m1 = textureLod(sampler, vec2(uv00.x, uvm1.y), lod).r;
    float oo_m2 = textureLod(sampler, vec2(uv00.x, uvm2.y), lod).r;
    float p1_p2 = textureLod(sampler, vec2(uvp1.x, uvp2.y), lod).r;
    float p1_p1 = textureLod(sampler, vec2(uvp1.x, uvp1.y), lod).r;
    float p1_oo = textureLod(sampler, vec2(uvp1.x, uv00.y), lod).r;
    float p1_m1 = textureLod(sampler, vec2(uvp1.x, uvm1.y), lod).r;
    float p1_m2 = textureLod(sampler, vec2(uvp1.x, uvm2.y), lod).r;
    float p2_p2 = textureLod(sampler, vec2(uvp2.x, uvp2.y), lod).r;
    float p2_p1 = textureLod(sampler, vec2(uvp2.x, uvp1.y), lod).r;
    float p2_oo = textureLod(sampler, vec2(uvp2.x, uv00.y), lod).r;
    float p2_m1 = textureLod(sampler, vec2(uvp2.x, uvm1.y), lod).r;
    float p2_m2 = textureLod(sampler, vec2(uvp2.x, uvm2.y), lod).r;

    vec2 dL;
    dL.x =       (p2_m2 - m2_m2 + p2_p2 - m2_p2) + 
           2.0 * (p2_m1 - m2_m1 + p2_p1 - m2_p1) + 
           3.0 * (p2_oo - m2_oo) + 
                 (p1_m2 - m1_m2 + p1_p2 - m1_p2) + 
           2.0 * (p1_m1 - m1_m1 + p1_p1 - m1_p1) + 
           6.0 * (p1_oo - m1_oo);

    dL.y =       (m2_p2 - m2_m2 + p2_p2 - p2_m2) + 
           2.0 * (m1_p2 - m1_m2 + p1_p2 - p1_m2) + 
           3.0 * (oo_p2 - oo_m2) + 
                 (m2_p1 - m2_m1 + p2_p1 - p2_m1) + 
           2.0 * (m1_p1 - m1_m1 + p1_p1 - p1_m1) + 
           6.0 * (oo_p1 - oo_m1);

    dL *= (amplitude * SCHARR_5x5_NORMALIZATION_FACTOR);
    vec3 n = vec3(dL, 1.0);
    n = 0.5 + 0.5 * normalize(n);
    return vec4(n, oo_oo);
}

//==============================================================================================================================================================
// Prewitt 3x3 filter : 8 texture reads
//==============================================================================================================================================================
subroutine(normal_disp_filter_func) vec4 prewitt3x3(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float PREWITT_3x3_NORMALIZATION_FACTOR = 1.0f / 6.0f;

    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

    float cc = textureLod(sampler, uv0, lod).r;
    float bl = textureLod(sampler, vec2(uvm.x, uvm.y), lod).r;             // bottom left
    float bc = textureLod(sampler, vec2(uv0.x, uvm.y), lod).r;             // bottom center
    float br = textureLod(sampler, vec2(uvp.x, uvm.y), lod).r;             // bottom right
    float cl = textureLod(sampler, vec2(uvm.x, uv0.y), lod).r;             // center left
    float cr = textureLod(sampler, vec2(uvp.x, uv0.y), lod).r;             // center right
    float tl = textureLod(sampler, vec2(uvm.x, uvp.y), lod).r;             // top left
    float tc = textureLod(sampler, vec2(uv0.x, uvp.y), lod).r;             // top center
    float tr = textureLod(sampler, vec2(uvp.x, uvp.y), lod).r;             // top right

    vec2 dL;
    dL.x = (br - bl) + (cr - cl) + (tr - tl);
    dL.y = (tl - bl) + (tc - bc) + (tr - br);
    dL *= (amplitude * PREWITT_3x3_NORMALIZATION_FACTOR);
    vec3 n = vec3(dL, 1.0);
    n = 0.5 + 0.5 * normalize(n);
    return vec4(n, cc);
}

//==============================================================================================================================================================
// Prewitt 5x5 filter : 24 texture reads
//==============================================================================================================================================================
subroutine(normal_disp_filter_func) vec4 prewitt5x5(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float PREWITT_5x5_NORMALIZATION_FACTOR = 1.0f / 30.0f;

    vec2 uv00 = uv;
    vec2 uvp1 = uv + texel_size;
    vec2 uvm1 = uv - texel_size;
    vec2 uvp2 = uvp1 + texel_size;
    vec2 uvm2 = uvm1 - texel_size;

    float oo_oo = textureLod(sampler, uv00, lod).r;
    float m2_p2 = textureLod(sampler, vec2(uvm2.x, uvp2.y), lod).r;
    float m2_p1 = textureLod(sampler, vec2(uvm2.x, uvp1.y), lod).r;
    float m2_oo = textureLod(sampler, vec2(uvm2.x, uv00.y), lod).r;
    float m2_m1 = textureLod(sampler, vec2(uvm2.x, uvm1.y), lod).r;
    float m2_m2 = textureLod(sampler, vec2(uvm2.x, uvm2.y), lod).r;
    float m1_p2 = textureLod(sampler, vec2(uvm1.x, uvp2.y), lod).r;
    float m1_p1 = textureLod(sampler, vec2(uvm1.x, uvp1.y), lod).r;
    float m1_oo = textureLod(sampler, vec2(uvm1.x, uv00.y), lod).r;
    float m1_m1 = textureLod(sampler, vec2(uvm1.x, uvm1.y), lod).r;
    float m1_m2 = textureLod(sampler, vec2(uvm1.x, uvm2.y), lod).r;
    float oo_p2 = textureLod(sampler, vec2(uv00.x, uvp2.y), lod).r;
    float oo_p1 = textureLod(sampler, vec2(uv00.x, uvp1.y), lod).r;
    float oo_m1 = textureLod(sampler, vec2(uv00.x, uvm1.y), lod).r;
    float oo_m2 = textureLod(sampler, vec2(uv00.x, uvm2.y), lod).r;
    float p1_p2 = textureLod(sampler, vec2(uvp1.x, uvp2.y), lod).r;
    float p1_p1 = textureLod(sampler, vec2(uvp1.x, uvp1.y), lod).r;
    float p1_oo = textureLod(sampler, vec2(uvp1.x, uv00.y), lod).r;
    float p1_m1 = textureLod(sampler, vec2(uvp1.x, uvm1.y), lod).r;
    float p1_m2 = textureLod(sampler, vec2(uvp1.x, uvm2.y), lod).r;
    float p2_p2 = textureLod(sampler, vec2(uvp2.x, uvp2.y), lod).r;
    float p2_p1 = textureLod(sampler, vec2(uvp2.x, uvp1.y), lod).r;
    float p2_oo = textureLod(sampler, vec2(uvp2.x, uv00.y), lod).r;
    float p2_m1 = textureLod(sampler, vec2(uvp2.x, uvm1.y), lod).r;
    float p2_m2 = textureLod(sampler, vec2(uvp2.x, uvm2.y), lod).r;

    vec2 dL;
    dL.x = 2.0 * ((p2_p2 - m2_p2) + (p2_p1 - m2_p1) + (p2_oo - m2_oo) + (p2_m1 - m2_m1) + (p2_m2 - m2_m2)) + 
                 ((p1_p2 - m1_p2) + (p1_p1 - m1_p1) + (p1_oo - m1_oo) + (p1_m1 - m1_m1) + (p1_m2 - m1_m2));
    dL.y = 2.0 * ((p2_p2 - p2_m2) + (p1_p2 - p1_m2) + (oo_p2 - oo_m2) + (m1_p2 - m1_m2) + (m2_p2 - m2_m2)) + 
                 ((p2_p1 - p2_m1) + (p1_p1 - p1_m1) + (oo_p1 - oo_m1) + (m1_p1 - m1_m1) + (m2_p1 - m2_m1));

    dL *= (amplitude * PREWITT_5x5_NORMALIZATION_FACTOR);
    vec3 n = vec3(dL, 1.0);
    n = 0.5 + 0.5 * normalize(n);
    return vec4(n, oo_oo);
}

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 Q = textureSize(heightmap_tex, 0);
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    if ((P.x >= Q.x) || (P.y >= Q.y)) return;

    vec2 texel_size = 1.0 / Q;
    vec2 uv = texel_size * (vec2(P) + 0.5);
    vec4 normal_disp = normal_disp_func(heightmap_tex, uv, texel_size);

    imageStore(normal_disp_image, P, normal_disp);
}