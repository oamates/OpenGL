#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D laplace_tex;
uniform sampler2D input_tex;
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

    float H_xp = texture(input_tex, vec2(uvp.x, uv0.y)).r;
    float H_xm = texture(input_tex, vec2(uvm.x, uv0.y)).r;
    float H_yp = texture(input_tex, vec2(uv0.x, uvp.y)).r;
    float H_ym = texture(input_tex, vec2(uv0.x, uvm.y)).r;

    float nabla = texture(laplace_tex, xp).r;

    float h = 0.25f * (H_xp + H_xm + H_yp + H_ym) - 0.25f * nabla;

    imageStore(output_image, P, vec4(h, 0.0, 0.0, 0.0));
}