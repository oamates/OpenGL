#version 330 core

in vec2 uv;

uniform sampler2D raymarch_tex;

out vec4 FragmentColor;

uniform vec2 focal_scale;
uniform vec2 pixel_size;
uniform float z_near;  
uniform int aa_mode;

const float DIAGONAL_WEIGHT_FACTOR = 0.707107;

float weight_func(float z, float w)
{
    float d = z - w;
    float d2 = d * d;

    return 0.317 * d2 / (d2 + 0.000625);
}

void main()
{
    ivec2 Q0 = ivec2(gl_FragCoord.xy);
    ivec2 Qp = min(Q0 + ivec2(1), textureSize(raymarch_tex, 0));
    ivec2 Qm = max(Q0 - ivec2(1), ivec2(0));

    vec4 c = texelFetch(raymarch_tex, Q0, 0);
    vec3 acc_rgb = c.rgb;
    float z = c.w;
    float total_weight = 1.0f;
    float w;

    //==========================================================================================================================================================
    // Top, bottom, right and left neighbors
    //==========================================================================================================================================================
    c = texelFetch(raymarch_tex, ivec2(Qm.x, Q0.y), 0);
    w = weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    c = texelFetch(raymarch_tex, ivec2(Qp.x, Q0.y), 0);
    w = weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    c = texelFetch(raymarch_tex, ivec2(Q0.x, Qm.y), 0);
    w = weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    c = texelFetch(raymarch_tex, ivec2(Q0.x, Qp.y), 0);
    w = weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    //==========================================================================================================================================================
    // Diagonal pixels input
    //==========================================================================================================================================================
    c = texelFetch(raymarch_tex, Qm, 0);
    w = DIAGONAL_WEIGHT_FACTOR * weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    c = texelFetch(raymarch_tex, ivec2(Qm.x, Qp.y), 0);
    w = DIAGONAL_WEIGHT_FACTOR * weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    c = texelFetch(raymarch_tex, ivec2(Qp.x, Qm.y), 0);
    w = DIAGONAL_WEIGHT_FACTOR * weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    c = texelFetch(raymarch_tex, Qp, 0);
    w = DIAGONAL_WEIGHT_FACTOR * weight_func(z, c.w);
    total_weight += w;
    acc_rgb += w * c.rgb;

    //==========================================================================================================================================================
    // output
    //==========================================================================================================================================================
    vec2 ndc = 2.0f * uv - 1.0f;
    vec3 view_cs = vec3(focal_scale * ndc, -1.0f);
    view_cs = normalize(view_cs);

    if (aa_mode == 0)
    {
        c = texelFetch(raymarch_tex, Q0, 0);
        acc_rgb = c.rgb;
    }
    else if (aa_mode == 1)
    {
        c = texelFetch(raymarch_tex, Q0, 0);
        acc_rgb = (2.0 - total_weight) * c.rgb + (total_weight - 1.0) * vec3(1.0, 0.0, 0.0);
    }
    else
    {
        float inv_total_weight = 1.0f / total_weight;
        acc_rgb *= inv_total_weight;
    }

    vec3 position_cs = z * normalize(view_cs);
    float z_aff = position_cs.z;
    gl_FragDepth = 1.0 + z_near / z_aff;   

    FragmentColor = vec4(acc_rgb, 1.0f);
//    FragmentColor = vec4(c00.rgb, 1.0f);
}  