#version 330 core

in vec2 uv;

uniform sampler2D raymarch_tex;

out vec4 FragmentColor;

uniform vec2 focal_scale;
uniform vec2 pixel_size;
uniform float z_near;  

const float DIAGONAL_WEIGHT_FACTOR = 0.5;

float weight_func(float z, float w)
{
    float d = z - w;
    float d2 = d * d;
    return 0.25 - 0.25 * exp(-d2);
}

void main()
{
    vec2 uv0 = uv;
    vec2 uvp = uv + pixel_size;
    vec2 uvm = uv - pixel_size;

    vec4 c00 = texture(raymarch_tex, uv0);
    vec3 acc_rgb = c00.rgb;
    float z = c00.w;
    float total_weight = 1.0f;

    //==========================================================================================================================================================
    // Top, bottom, right and left neighbors
    //==========================================================================================================================================================
    /* BOTTOM */
    vec4 c = texture(raymarch_tex, vec2(uvm.x, uv0.y));
    float w = weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    /* TOP */
    c = texture(raymarch_tex, vec2(uvp.x, uv0.y));
    w = weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    /* LEFT */
    c = texture(raymarch_tex, vec2(uv0.x, uvm.y));
    w = weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    /* RIGHT */
    c = texture(raymarch_tex, vec2(uv0.x, uvp.y));
    w = weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    //==========================================================================================================================================================
    // Diagonal pixels input
    //==========================================================================================================================================================
    /* BOTTOM LEFT */
    c = texture(raymarch_tex, uvm);
    w = DIAGONAL_WEIGHT_FACTOR * weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    /* TOP LEFT */
    c = texture(raymarch_tex, vec2(uvm.x, uvp.y));
    w = DIAGONAL_WEIGHT_FACTOR * weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    /* BOTTOM RIGHT */
    c = texture(raymarch_tex, vec2(uvp.x, uvm.y));
    w = DIAGONAL_WEIGHT_FACTOR * weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    /* TOP RIGHT */
    c = texture(raymarch_tex, uvp);
    w = DIAGONAL_WEIGHT_FACTOR * weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    //==========================================================================================================================================================
    // output
    //==========================================================================================================================================================
    vec2 ndc = 2.0f * uv - 1.0f;
    vec3 view_cs = vec3(focal_scale * ndc, -1.0f);
    view_cs = normalize(view_cs);

    float inv_total_weight = 1.0f / total_weight;

    vec3 position_cs = z * normalize(view_cs);
    float z_aff = position_cs.z;
    gl_FragDepth = 1.0 + z_near / z_aff;   

    FragmentColor = vec4(inv_total_weight * acc_rgb, 1.0f);
}  