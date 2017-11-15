#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D luma_tex;
layout (rgba32f) uniform image2D normal_image;

uniform float amplitude;
uniform int tex_level;

subroutine vec2 derivative_filter_func(sampler2D sampler, vec2 uv, vec2 texel_size, float lod);
subroutine uniform derivative_filter_func derivative_func;

//==============================================================================================================================================================
// Symmetric difference filter : 4 texture reads
//==============================================================================================================================================================
subroutine(derivative_filter_func) vec2 symm_diff_ls(sampler2D sampler, vec2 uv, vec2 texel_size, float lod)
{
    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

    vec4 bc = textureLod(sampler, vec2(uv0.x, uvm.y), lod);             // bottom center
    vec4 cl = textureLod(sampler, vec2(uvm.x, uv0.y), lod);             // center left
    vec4 cr = textureLod(sampler, vec2(uvp.x, uv0.y), lod);             // center right
    vec4 tc = textureLod(sampler, vec2(uv0.x, uvp.y), lod);             // top center
    
    vec4 drl = cr - cl;
    vec4 dtb = tc - bc;


    vec2 dL0 = vec2(drl.r, dtb.r);
    vec2 dS0 = vec2(drl.g, dtb.g);
    vec2 dL1 = vec2(drl.b, dtb.b);
    vec2 dS1 = vec2(drl.a, dtb.a);

    float att = 1.0 / (0.25 + dot(dS0, dS0));

    dL0 = att * dL0;

    return dL0;
}

//==============================================================================================================================================================
// Symmetric difference filter : 4 texture reads
//==============================================================================================================================================================
subroutine(derivative_filter_func) vec2 symm_diff(sampler2D sampler, vec2 uv, vec2 texel_size, float lod)
{
    const float SYMM_DIFF_NORMALIZATION_FACTOR = 1.0f / 2.0f;

    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

    float bc = textureLod(sampler, vec2(uv0.x, uvm.y), lod).r;             // bottom center
    float cl = textureLod(sampler, vec2(uvm.x, uv0.y), lod).r;             // center left
    float cr = textureLod(sampler, vec2(uvp.x, uv0.y), lod).r;             // center right
    float tc = textureLod(sampler, vec2(uv0.x, uvp.y), lod).r;             // top center

    vec2 dL = vec2(cr - cl, tc - bc);
    return SYMM_DIFF_NORMALIZATION_FACTOR * dL;
}

//==============================================================================================================================================================
// Sobel 3x3 filter : 8 texture reads
//==============================================================================================================================================================
subroutine(derivative_filter_func) vec2 sobel3x3(sampler2D sampler, vec2 uv, vec2 texel_size, float lod)
{
    const float SOBEL_3x3_NORMALIZATION_FACTOR = 1.0f / 8.0f;

    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

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
    return SOBEL_3x3_NORMALIZATION_FACTOR * dL;
}

//==============================================================================================================================================================
// Sobel 5x5 filter : 24 texture reads
//==============================================================================================================================================================
subroutine(derivative_filter_func) vec2 sobel5x5(sampler2D sampler, vec2 uv, vec2 texel_size, float lod)
{
    const float SOBEL_5x5_NORMALIZATION_FACTOR = 1.0f / 96.0f;

    vec2 uv00 = uv;
    vec2 uvp1 = uv + texel_size;
    vec2 uvm1 = uv - texel_size;
    vec2 uvp2 = uvp1 + texel_size;
    vec2 uvm2 = uvm1 - texel_size;

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

    return SOBEL_5x5_NORMALIZATION_FACTOR * dL;
}

//==============================================================================================================================================================
// Scharr 3x3 filter : 8 texture reads
//==============================================================================================================================================================
subroutine(derivative_filter_func) vec2 scharr3x3(sampler2D sampler, vec2 uv, vec2 texel_size, float lod)
{
    const float SCHARR_3x3_NORMALIZATION_FACTOR = 1.0f / 32.0f;

    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

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
    return SCHARR_3x3_NORMALIZATION_FACTOR * dL;
}

//==============================================================================================================================================================
// Scharr 5x5 filter : 24 texture reads
//==============================================================================================================================================================
subroutine(derivative_filter_func) vec2 scharr5x5(sampler2D sampler, vec2 uv, vec2 texel_size, float lod)
{
    const float SCHARR_5x5_NORMALIZATION_FACTOR = 1.0f / 42.0f;

    vec2 uv00 = uv;
    vec2 uvp1 = uv + texel_size;
    vec2 uvm1 = uv - texel_size;
    vec2 uvp2 = uvp1 + texel_size;
    vec2 uvm2 = uvm1 - texel_size;

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

    return SCHARR_5x5_NORMALIZATION_FACTOR * dL;
}

//==============================================================================================================================================================
// Prewitt 3x3 filter : 8 texture reads
//==============================================================================================================================================================
subroutine(derivative_filter_func) vec2 prewitt3x3(sampler2D sampler, vec2 uv, vec2 texel_size, float lod)
{
    const float PREWITT_3x3_NORMALIZATION_FACTOR = 1.0f / 6.0f;

    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

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
    return PREWITT_3x3_NORMALIZATION_FACTOR * dL;
}

//==============================================================================================================================================================
// Prewitt 5x5 filter : 24 texture reads
//==============================================================================================================================================================
subroutine(derivative_filter_func) vec2 prewitt5x5(sampler2D sampler, vec2 uv, vec2 texel_size, float lod)
{
    const float PREWITT_5x5_NORMALIZATION_FACTOR = 1.0f / 30.0f;

    vec2 uv00 = uv;
    vec2 uvp1 = uv + texel_size;
    vec2 uvm1 = uv - texel_size;
    vec2 uvp2 = uvp1 + texel_size;
    vec2 uvm2 = uvm1 - texel_size;

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

    return PREWITT_5x5_NORMALIZATION_FACTOR * dL;
}

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 Q = textureSize(luma_tex, tex_level);
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    if ((P.x >= Q.x) || (P.y >= Q.y)) return;

    vec2 texel_size = 1.0 / Q;
    vec2 uv = texel_size * (vec2(P) + 0.5);
    float lod = tex_level;

    vec2 dL = derivative_func(luma_tex, uv, texel_size, lod);
    vec3 n = normalize(vec3(amplitude * dL, 1.0));

    imageStore(normal_image, P, vec4(0.5 + 0.5 * n, 1.0));
}