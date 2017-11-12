#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D normal_tex;
uniform sampler2D disp_tex;
layout (r32f) uniform image2D output_image;

uniform vec2 texel_size;
uniform vec2 delta;

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv0 = texel_size * (vec2(P) + 0.5);
    vec2 uvp = uv0 + delta;
    vec2 uvm = uv0 - delta;

    vec2 xp = vec2(uvp.x, uv0.y);
    float H_xp = texture(disp_tex, xp).r;
    float n_xp = texture(normal_tex, xp).r;

    vec2 xm = vec2(uvm.x, uv0.y);
    float H_xm = texture(disp_tex, xm).r;
    float n_xm = texture(normal_tex, xm).r;

    vec2 yp = vec2(uv0.x, uvp.y);
    float H_yp = texture(disp_tex, yp).r;
    float n_yp = texture(normal_tex, yp).g;

    vec2 ym = vec2(uv0.x, uvm.y);
    float H_ym = texture(disp_tex, ym).r;
    float n_ym = texture(normal_tex, ym).g;

    float h = 0.25f * (H_xp + H_xm + H_yp + H_ym) -
              0.25f * (n_xp - n_xm + n_yp - n_ym);

    imageStore(output_image, P, vec4(h, 0.0, 0.0, 0.0));
}