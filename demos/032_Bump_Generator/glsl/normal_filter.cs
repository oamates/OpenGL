#version 430 core

//==============================================================================================================================================================
// Every workgroup will work on 8 x 8 pixel area
//==============================================================================================================================================================
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D luma_tex;
layout (rgba32f, binding = 4) uniform image2D normal_image;

uniform float amplitude;
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

    float bl = texture(luma_tex, vec2(uvm.x, uvm.y));             // bottom left
    float bc = texture(luma_tex, vec2(uv0.x, uvm.y));             // bottom center
    float br = texture(luma_tex, vec2(uvp.x, uvm.y));             // bottom right
    float cl = texture(luma_tex, vec2(uvm.x, uv0.y));             // center left
    float cr = texture(luma_tex, vec2(uvp.x, uv0.y));             // center right
    float tl = texture(luma_tex, vec2(uvm.x, uvp.y));             // top left
    float tc = texture(luma_tex, vec2(uv0.x, uvp.y));             // top center
    float tr = texture(luma_tex, vec2(uvp.x, uvp.y));             // top right

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

    vec3 n = normalize(vec3(amplitude * vec2(dL_dx, dL_dy), 1.0));
    imageStore(normal_image, P, vec4(0.5 + 0.5 * n, 1.0));
}