#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D normal_tex;
layout (r32f) uniform image2D laplace_image;

subroutine float laplacian_filter_func(sampler2D sampler, vec2 uv, vec2 texel_size);
subroutine uniform laplacian_filter_func laplacian_func;

//==============================================================================================================================================================
// Symmetric difference filter : 4 texture reads
//==============================================================================================================================================================
subroutine(laplacian_filter_func) float symm_diff(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float SYMM_DIFF_NORMALIZATION_FACTOR = 1.0f / 2.0f;

    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

    vec3 bc = texture(sampler, vec2(uv0.x, uvm.y)).rgb; bc.xy /= bc.z;
    vec3 cl = texture(sampler, vec2(uvm.x, uv0.y)).rgb; cl.xy /= cl.z;
    vec3 cr = texture(sampler, vec2(uvp.x, uv0.y)).rgb; cr.xy /= cr.z;
    vec3 tc = texture(sampler, vec2(uv0.x, uvp.y)).rgb; tc.xy /= tc.z;

    float nxx = cr.x - cl.x;
    float nyy = tc.y - bc.y;
    return SYMM_DIFF_NORMALIZATION_FACTOR * (nxx + nyy);
}

//==============================================================================================================================================================
// Sobel 3x3 filter : 8 texture reads
//==============================================================================================================================================================
subroutine(laplacian_filter_func) float sobel3x3(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float SOBEL_3x3_NORMALIZATION_FACTOR = 1.0f / 8.0f;

    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

    vec3 bl = texture(sampler, vec2(uvm.x, uvm.y)).rgb; bl.xy /= bl.z;
    vec3 bc = texture(sampler, vec2(uv0.x, uvm.y)).rgb; bc.xy /= bc.z;
    vec3 br = texture(sampler, vec2(uvp.x, uvm.y)).rgb; br.xy /= br.z;
    vec3 cl = texture(sampler, vec2(uvm.x, uv0.y)).rgb; cl.xy /= cl.z;
    vec3 cr = texture(sampler, vec2(uvp.x, uv0.y)).rgb; cr.xy /= cr.z;
    vec3 tl = texture(sampler, vec2(uvm.x, uvp.y)).rgb; tl.xy /= tl.z;
    vec3 tc = texture(sampler, vec2(uv0.x, uvp.y)).rgb; tc.xy /= tc.z;
    vec3 tr = texture(sampler, vec2(uvp.x, uvp.y)).rgb; tr.xy /= tr.z;

    float nxx = (br.x - bl.x) + 2.0 * (cr.x - cl.x) + (tr.x - tl.x);
    float nyy = (tl.y - bl.y) + 2.0 * (tc.y - bc.y) + (tr.y - br.y);
    return SOBEL_3x3_NORMALIZATION_FACTOR * (nxx + nyy);
}

//==============================================================================================================================================================
// Sobel 5x5 filter : 24 texture reads
//==============================================================================================================================================================
subroutine(laplacian_filter_func) float sobel5x5(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float SOBEL_5x5_NORMALIZATION_FACTOR = 1.0f / 96.0f;

    vec2 uv00 = uv;
    vec2 uvp1 = uv + texel_size;
    vec2 uvm1 = uv - texel_size;
    vec2 uvp2 = uvp1 + texel_size;
    vec2 uvm2 = uvm1 - texel_size;

    vec3 m2_p2 = texture(sampler, vec2(uvm2.x, uvp2.y)).rgb; m2_p2.xy /= m2_p2.z;
    vec3 m2_p1 = texture(sampler, vec2(uvm2.x, uvp1.y)).rgb; m2_p1.xy /= m2_p1.z;
    vec3 m2_oo = texture(sampler, vec2(uvm2.x, uv00.y)).rgb; m2_oo.xy /= m2_oo.z;
    vec3 m2_m1 = texture(sampler, vec2(uvm2.x, uvm1.y)).rgb; m2_m1.xy /= m2_m1.z;
    vec3 m2_m2 = texture(sampler, vec2(uvm2.x, uvm2.y)).rgb; m2_m2.xy /= m2_m2.z;
    vec3 m1_p2 = texture(sampler, vec2(uvm1.x, uvp2.y)).rgb; m1_p2.xy /= m1_p2.z;
    vec3 m1_p1 = texture(sampler, vec2(uvm1.x, uvp1.y)).rgb; m1_p1.xy /= m1_p1.z;
    vec3 m1_oo = texture(sampler, vec2(uvm1.x, uv00.y)).rgb; m1_oo.xy /= m1_oo.z;
    vec3 m1_m1 = texture(sampler, vec2(uvm1.x, uvm1.y)).rgb; m1_m1.xy /= m1_m1.z;
    vec3 m1_m2 = texture(sampler, vec2(uvm1.x, uvm2.y)).rgb; m1_m2.xy /= m1_m2.z;
    vec3 oo_p2 = texture(sampler, vec2(uv00.x, uvp2.y)).rgb; oo_p2.xy /= oo_p2.z;
    vec3 oo_p1 = texture(sampler, vec2(uv00.x, uvp1.y)).rgb; oo_p1.xy /= oo_p1.z;
    vec3 oo_m1 = texture(sampler, vec2(uv00.x, uvm1.y)).rgb; oo_m1.xy /= oo_m1.z;
    vec3 oo_m2 = texture(sampler, vec2(uv00.x, uvm2.y)).rgb; oo_m2.xy /= oo_m2.z;
    vec3 p1_p2 = texture(sampler, vec2(uvp1.x, uvp2.y)).rgb; p1_p2.xy /= p1_p2.z;
    vec3 p1_p1 = texture(sampler, vec2(uvp1.x, uvp1.y)).rgb; p1_p1.xy /= p1_p1.z;
    vec3 p1_oo = texture(sampler, vec2(uvp1.x, uv00.y)).rgb; p1_oo.xy /= p1_oo.z;
    vec3 p1_m1 = texture(sampler, vec2(uvp1.x, uvm1.y)).rgb; p1_m1.xy /= p1_m1.z;
    vec3 p1_m2 = texture(sampler, vec2(uvp1.x, uvm2.y)).rgb; p1_m2.xy /= p1_m2.z;
    vec3 p2_p2 = texture(sampler, vec2(uvp2.x, uvp2.y)).rgb; p2_p2.xy /= p2_p2.z;
    vec3 p2_p1 = texture(sampler, vec2(uvp2.x, uvp1.y)).rgb; p2_p1.xy /= p2_p1.z;
    vec3 p2_oo = texture(sampler, vec2(uvp2.x, uv00.y)).rgb; p2_oo.xy /= p2_oo.z;
    vec3 p2_m1 = texture(sampler, vec2(uvp2.x, uvm1.y)).rgb; p2_m1.xy /= p2_m1.z;
    vec3 p2_m2 = texture(sampler, vec2(uvp2.x, uvm2.y)).rgb; p2_m2.xy /= p2_m2.z;

    float nxx =  (p2_m2.x - m2_m2.x + p2_p2.x - m2_p2.x) + 
           4.0 * (p2_m1.x - m2_m1.x + p2_p1.x - m2_p1.x) + 
           6.0 * (p2_oo.x - m2_oo.x) + 
           2.0 * (p1_m2.x - m1_m2.x + p1_p2.x - m1_p2.x) + 
           8.0 * (p1_m1.x - m1_m1.x + p1_p1.x - m1_p1.x) + 
          12.0 * (p1_oo.x - m1_oo.x);

    float nyy =  (m2_p2.y - m2_m2.y + p2_p2.y - p2_m2.y) + 
           4.0 * (m1_p2.y - m1_m2.y + p1_p2.y - p1_m2.y) + 
           6.0 * (oo_p2.y - oo_m2.y) +
           2.0 * (m2_p1.y - m2_m1.y + p2_p1.y - p2_m1.y) + 
           8.0 * (m1_p1.y - m1_m1.y + p1_p1.y - p1_m1.y) + 
          12.0 * (oo_p1.y - oo_m1.y);

    return SOBEL_5x5_NORMALIZATION_FACTOR * (nxx + nyy);
}

//==============================================================================================================================================================
// Scharr 3x3 filter : 8 texture reads
//==============================================================================================================================================================
subroutine(laplacian_filter_func) float scharr3x3(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float SCHARR_3x3_NORMALIZATION_FACTOR = 1.0f / 32.0f;

    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

    vec3 bl = texture(sampler, vec2(uvm.x, uvm.y)).rgb; bl.xy /= bl.z;
    vec3 bc = texture(sampler, vec2(uv0.x, uvm.y)).rgb; bc.xy /= bc.z;
    vec3 br = texture(sampler, vec2(uvp.x, uvm.y)).rgb; br.xy /= br.z;
    vec3 cl = texture(sampler, vec2(uvm.x, uv0.y)).rgb; cl.xy /= cl.z;
    vec3 cr = texture(sampler, vec2(uvp.x, uv0.y)).rgb; cr.xy /= cr.z;
    vec3 tl = texture(sampler, vec2(uvm.x, uvp.y)).rgb; tl.xy /= tl.z;
    vec3 tc = texture(sampler, vec2(uv0.x, uvp.y)).rgb; tc.xy /= tc.z;
    vec3 tr = texture(sampler, vec2(uvp.x, uvp.y)).rgb; tr.xy /= tr.z;

    float nxx = 3.0 * ((br.x - bl.x) + (tr.x - tl.x)) + 10.0 * (cr.x - cl.x);
    float nyy = 3.0 * ((tl.y - bl.y) + (tr.y - br.y)) + 10.0 * (tc.y - bc.y);
    return SCHARR_3x3_NORMALIZATION_FACTOR * (nxx + nyy);
}

//==============================================================================================================================================================
// Scharr 5x5 filter : 24 texture reads
//==============================================================================================================================================================
subroutine(laplacian_filter_func) float scharr5x5(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float SCHARR_5x5_NORMALIZATION_FACTOR = 1.0f / 42.0f;

    vec2 uv00 = uv;
    vec2 uvp1 = uv + texel_size;
    vec2 uvm1 = uv - texel_size;
    vec2 uvp2 = uvp1 + texel_size;
    vec2 uvm2 = uvm1 - texel_size;

    vec3 m2_p2 = texture(sampler, vec2(uvm2.x, uvp2.y)).rgb; m2_p2.xy /= m2_p2.z;
    vec3 m2_p1 = texture(sampler, vec2(uvm2.x, uvp1.y)).rgb; m2_p1.xy /= m2_p1.z;
    vec3 m2_oo = texture(sampler, vec2(uvm2.x, uv00.y)).rgb; m2_oo.xy /= m2_oo.z;
    vec3 m2_m1 = texture(sampler, vec2(uvm2.x, uvm1.y)).rgb; m2_m1.xy /= m2_m1.z;
    vec3 m2_m2 = texture(sampler, vec2(uvm2.x, uvm2.y)).rgb; m2_m2.xy /= m2_m2.z;
    vec3 m1_p2 = texture(sampler, vec2(uvm1.x, uvp2.y)).rgb; m1_p2.xy /= m1_p2.z;
    vec3 m1_p1 = texture(sampler, vec2(uvm1.x, uvp1.y)).rgb; m1_p1.xy /= m1_p1.z;
    vec3 m1_oo = texture(sampler, vec2(uvm1.x, uv00.y)).rgb; m1_oo.xy /= m1_oo.z;
    vec3 m1_m1 = texture(sampler, vec2(uvm1.x, uvm1.y)).rgb; m1_m1.xy /= m1_m1.z;
    vec3 m1_m2 = texture(sampler, vec2(uvm1.x, uvm2.y)).rgb; m1_m2.xy /= m1_m2.z;
    vec3 oo_p2 = texture(sampler, vec2(uv00.x, uvp2.y)).rgb; oo_p2.xy /= oo_p2.z;
    vec3 oo_p1 = texture(sampler, vec2(uv00.x, uvp1.y)).rgb; oo_p1.xy /= oo_p1.z;
    vec3 oo_m1 = texture(sampler, vec2(uv00.x, uvm1.y)).rgb; oo_m1.xy /= oo_m1.z;
    vec3 oo_m2 = texture(sampler, vec2(uv00.x, uvm2.y)).rgb; oo_m2.xy /= oo_m2.z;
    vec3 p1_p2 = texture(sampler, vec2(uvp1.x, uvp2.y)).rgb; p1_p2.xy /= p1_p2.z;
    vec3 p1_p1 = texture(sampler, vec2(uvp1.x, uvp1.y)).rgb; p1_p1.xy /= p1_p1.z;
    vec3 p1_oo = texture(sampler, vec2(uvp1.x, uv00.y)).rgb; p1_oo.xy /= p1_oo.z;
    vec3 p1_m1 = texture(sampler, vec2(uvp1.x, uvm1.y)).rgb; p1_m1.xy /= p1_m1.z;
    vec3 p1_m2 = texture(sampler, vec2(uvp1.x, uvm2.y)).rgb; p1_m2.xy /= p1_m2.z;
    vec3 p2_p2 = texture(sampler, vec2(uvp2.x, uvp2.y)).rgb; p2_p2.xy /= p2_p2.z;
    vec3 p2_p1 = texture(sampler, vec2(uvp2.x, uvp1.y)).rgb; p2_p1.xy /= p2_p1.z;
    vec3 p2_oo = texture(sampler, vec2(uvp2.x, uv00.y)).rgb; p2_oo.xy /= p2_oo.z;
    vec3 p2_m1 = texture(sampler, vec2(uvp2.x, uvm1.y)).rgb; p2_m1.xy /= p2_m1.z;
    vec3 p2_m2 = texture(sampler, vec2(uvp2.x, uvm2.y)).rgb; p2_m2.xy /= p2_m2.z;

    float nxx =  (p2_m2.x - m2_m2.x + p2_p2.x - m2_p2.x) + 
           2.0 * (p2_m1.x - m2_m1.x + p2_p1.x - m2_p1.x) + 
           3.0 * (p2_oo.x - m2_oo.x) +
                 (p1_m2.x - m1_m2.x + p1_p2.x - m1_p2.x) + 
           2.0 * (p1_m1.x - m1_m1.x + p1_p1.x - m1_p1.x) + 
           6.0 * (p1_oo.x - m1_oo.x);

    float nyy =  (m2_p2.y - m2_m2.y + p2_p2.y - p2_m2.y) + 
           2.0 * (m1_p2.y - m1_m2.y + p1_p2.y - p1_m2.y) + 
           3.0 * (oo_p2.y - oo_m2.y) +
                 (m2_p1.y - m2_m1.y + p2_p1.y - p2_m1.y) + 
           2.0 * (m1_p1.y - m1_m1.y + p1_p1.y - p1_m1.y) + 
           6.0 * (oo_p1.y - oo_m1.y);

    return SCHARR_5x5_NORMALIZATION_FACTOR * (nxx + nyy);
}

//==============================================================================================================================================================
// Prewitt 3x3 filter : 8 texture reads
//==============================================================================================================================================================
subroutine(laplacian_filter_func) float prewitt3x3(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float PREWITT_3x3_NORMALIZATION_FACTOR = 1.0f / 6.0f;

    vec2 uv0 = uv;
    vec2 uvp = uv + texel_size;
    vec2 uvm = uv - texel_size;

    vec3 bl = texture(sampler, vec2(uvm.x, uvm.y)).rgb; bl.xy /= bl.z;
    vec3 bc = texture(sampler, vec2(uv0.x, uvm.y)).rgb; bc.xy /= bc.z;
    vec3 br = texture(sampler, vec2(uvp.x, uvm.y)).rgb; br.xy /= br.z;
    vec3 cl = texture(sampler, vec2(uvm.x, uv0.y)).rgb; cl.xy /= cl.z;
    vec3 cr = texture(sampler, vec2(uvp.x, uv0.y)).rgb; cr.xy /= cr.z;
    vec3 tl = texture(sampler, vec2(uvm.x, uvp.y)).rgb; tl.xy /= tl.z;
    vec3 tc = texture(sampler, vec2(uv0.x, uvp.y)).rgb; tc.xy /= tc.z;
    vec3 tr = texture(sampler, vec2(uvp.x, uvp.y)).rgb; tr.xy /= tr.z;

    float nxx = (br.x - bl.x) + (cr.x - cl.x) + (tr.x - tl.x);
    float nyy = (tl.y - bl.y) + (tc.y - bc.y) + (tr.y - br.y);

    return PREWITT_3x3_NORMALIZATION_FACTOR * (nxx + nyy);
}

//==============================================================================================================================================================
// Prewitt 5x5 filter : 24 texture reads
//==============================================================================================================================================================
subroutine(laplacian_filter_func) float prewitt5x5(sampler2D sampler, vec2 uv, vec2 texel_size)
{
    const float PREWITT_5x5_NORMALIZATION_FACTOR = 1.0f / 30.0f;

    vec2 uv00 = uv;
    vec2 uvp1 = uv + texel_size;
    vec2 uvm1 = uv - texel_size;
    vec2 uvp2 = uvp1 + texel_size;
    vec2 uvm2 = uvm1 - texel_size;

    vec3 m2_p2 = texture(sampler, vec2(uvm2.x, uvp2.y)).rgb; m2_p2.xy /= m2_p2.z;
    vec3 m2_p1 = texture(sampler, vec2(uvm2.x, uvp1.y)).rgb; m2_p1.xy /= m2_p1.z;
    vec3 m2_oo = texture(sampler, vec2(uvm2.x, uv00.y)).rgb; m2_oo.xy /= m2_oo.z;
    vec3 m2_m1 = texture(sampler, vec2(uvm2.x, uvm1.y)).rgb; m2_m1.xy /= m2_m1.z;
    vec3 m2_m2 = texture(sampler, vec2(uvm2.x, uvm2.y)).rgb; m2_m2.xy /= m2_m2.z;
    vec3 m1_p2 = texture(sampler, vec2(uvm1.x, uvp2.y)).rgb; m1_p2.xy /= m1_p2.z;
    vec3 m1_p1 = texture(sampler, vec2(uvm1.x, uvp1.y)).rgb; m1_p1.xy /= m1_p1.z;
    vec3 m1_oo = texture(sampler, vec2(uvm1.x, uv00.y)).rgb; m1_oo.xy /= m1_oo.z;
    vec3 m1_m1 = texture(sampler, vec2(uvm1.x, uvm1.y)).rgb; m1_m1.xy /= m1_m1.z;
    vec3 m1_m2 = texture(sampler, vec2(uvm1.x, uvm2.y)).rgb; m1_m2.xy /= m1_m2.z;
    vec3 oo_p2 = texture(sampler, vec2(uv00.x, uvp2.y)).rgb; oo_p2.xy /= oo_p2.z;
    vec3 oo_p1 = texture(sampler, vec2(uv00.x, uvp1.y)).rgb; oo_p1.xy /= oo_p1.z;
    vec3 oo_m1 = texture(sampler, vec2(uv00.x, uvm1.y)).rgb; oo_m1.xy /= oo_m1.z;
    vec3 oo_m2 = texture(sampler, vec2(uv00.x, uvm2.y)).rgb; oo_m2.xy /= oo_m2.z;
    vec3 p1_p2 = texture(sampler, vec2(uvp1.x, uvp2.y)).rgb; p1_p2.xy /= p1_p2.z;
    vec3 p1_p1 = texture(sampler, vec2(uvp1.x, uvp1.y)).rgb; p1_p1.xy /= p1_p1.z;
    vec3 p1_oo = texture(sampler, vec2(uvp1.x, uv00.y)).rgb; p1_oo.xy /= p1_oo.z;
    vec3 p1_m1 = texture(sampler, vec2(uvp1.x, uvm1.y)).rgb; p1_m1.xy /= p1_m1.z;
    vec3 p1_m2 = texture(sampler, vec2(uvp1.x, uvm2.y)).rgb; p1_m2.xy /= p1_m2.z;
    vec3 p2_p2 = texture(sampler, vec2(uvp2.x, uvp2.y)).rgb; p2_p2.xy /= p2_p2.z;
    vec3 p2_p1 = texture(sampler, vec2(uvp2.x, uvp1.y)).rgb; p2_p1.xy /= p2_p1.z;
    vec3 p2_oo = texture(sampler, vec2(uvp2.x, uv00.y)).rgb; p2_oo.xy /= p2_oo.z;
    vec3 p2_m1 = texture(sampler, vec2(uvp2.x, uvm1.y)).rgb; p2_m1.xy /= p2_m1.z;
    vec3 p2_m2 = texture(sampler, vec2(uvp2.x, uvm2.y)).rgb; p2_m2.xy /= p2_m2.z;

    float nxx = 2.0 * ((p2_p2.x - m2_p2.x) + (p2_p1.x - m2_p1.x) + (p2_oo.x - m2_oo.x) + (p2_m1.x - m2_m1.x) + (p2_m2.x - m2_m2.x)) + 
                      ((p1_p2.x - m1_p2.x) + (p1_p1.x - m1_p1.x) + (p1_oo.x - m1_oo.x) + (p1_m1.x - m1_m1.x) + (p1_m2.x - m1_m2.x));
    float nyy = 2.0 * ((p2_p2.y - p2_m2.y) + (p1_p2.y - p1_m2.y) + (oo_p2.y - oo_m2.y) + (m1_p2.y - m1_m2.y) + (m2_p2.y - m2_m2.y)) + 
                      ((p2_p1.y - p2_m1.y) + (p1_p1.y - p1_m1.y) + (oo_p1.y - oo_m1.y) + (m1_p1.y - m1_m1.y) + (m2_p1.y - m2_m1.y));

    return PREWITT_5x5_NORMALIZATION_FACTOR * (nxx + nyy);
}

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 Q = textureSize(normal_tex, 0);
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    if ((P.x >= Q.x) || (P.y >= Q.y)) return;

    vec2 texel_size = 1.0 / Q;
    vec2 uv = texel_size * (vec2(P) + 0.5);

    float nabla = laplacian_func(normal_tex, uv, texel_size);
    imageStore(laplace_image, P, vec4(nabla, 0.0f, 0.0f, 0.0f));
}