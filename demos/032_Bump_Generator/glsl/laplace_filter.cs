#version 430 core

//==============================================================================================================================================================
// Every workgroup will work on 8 x 8 pixel area
//==============================================================================================================================================================
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D input_tex;
layout (r32f) uniform image2D output_image;

uniform vec2 texel_size;

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv0 = texel_size * (vec2(P) + 0.5);
    vec2 uvp = uv0 + texel_size;
    vec2 uvm = uv0 - texel_size;

    /* Laplace filter, simple variation -- basic + 4 sample points */

    float f = texture(input_tex, uv0).r;
    float fp0 = texture(input_tex, vec2(uvp.x, uv0.y)).r;
    float fm0 = texture(input_tex, vec2(uvm.x, uv0.y)).r;
    float f0p = texture(input_tex, vec2(uv0.x, uvp.y)).r;
    float f0m = texture(input_tex, vec2(uv0.x, uvm.y)).r;

    float q = 0.25 * (fp0 + fm0 + f0p + f0m) - f;
    q = 0.5 + 0.5 * q;

    imageStore(output_image, P, vec4(q, 0.0, 0.0, 0.0));

    /* Laplace filter, advanced variation -- basic + 8 sample points */

    /*
    float f = texture(input_tex, uv).r;
    float fp0 = texture(input_tex, vec2(uvp.x, uv0.y)).r;
    float fm0 = texture(input_tex, vec2(uvm.x, uv0.y)).r;
    float f0p = texture(input_tex, vec2(uv0.x, uvp.y)).r;
    float f0m = texture(input_tex, vec2(uv0.x, uvm.y)).r;

    float fpp = texture(input_tex, uvp).r;
    float fmm = texture(input_tex, uvm).r;
    float fpm = texture(input_tex, vec2(uvp.x, uvm.y)).r;
    float fmp = texture(input_tex, vec2(uvm.x, uvp.y)).r;

    float q = 0.15625 * (fp0 + fm0 + f0p + f0m) + 
              0.09375 * (fpp + fmm + fpm + fmp) - f;

    q = 0.5 + 0.5 * q;

    imageStore(output_image, P, vec4(q, 0.0, 0.0, 0.0));
    */
}