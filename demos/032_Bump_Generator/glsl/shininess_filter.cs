#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

//==============================================================================================================================================================
// Every workgroup will work on 8 x 8 pixel area
//==============================================================================================================================================================

uniform sampler2D diffuse_tex;
layout (r32f) uniform image2D shininess_image;

uniform vec2 texel_size;

const float w0 = 0.25f;
const float w1 = 0.125f;
const float w2 = 0.0625f;

const float inv_root2 = 0.70710678118;
//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main(void)
{
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv0 = texel_size * (vec2(P) + 0.5);
    vec2 uvp = uv0 + texel_size;
    vec2 uvm = uv0 - texel_size;    

    vec3 bl = texture(diffuse_tex, vec2(uvm.x, uvm.y)).rgb;             // bottom left
    vec3 bc = texture(diffuse_tex, vec2(uv0.x, uvm.y)).rgb;             // bottom center
    vec3 br = texture(diffuse_tex, vec2(uvp.x, uvm.y)).rgb;             // bottom right
    vec3 cl = texture(diffuse_tex, vec2(uvm.x, uv0.y)).rgb;             // center left
    vec3 cc = texture(diffuse_tex, vec2(uv0.x, uv0.y)).rgb;             // center center
    vec3 cr = texture(diffuse_tex, vec2(uvp.x, uv0.y)).rgb;             // center right
    vec3 tl = texture(diffuse_tex, vec2(uvm.x, uvp.y)).rgb;             // top left
    vec3 tc = texture(diffuse_tex, vec2(uv0.x, uvp.y)).rgb;             // top center
    vec3 tr = texture(diffuse_tex, vec2(uvp.x, uvp.y)).rgb;             // top right

    vec3 s1 = abs(cc - bc) + abs(cc - cl) + abs(cc - cr) + abs(cc - tc);
    vec3 s2 = abs(cc - bl) + abs(cc - br) + abs(cc - tl) + abs(cc - tr);

    float w = dot(s1 + inv_root2 * s2, vec3(1.0f));
    float q = clamp(w, 0.0, 1.0);

    imageStore(shininess_image, P, vec4(q, 0.0, 0.0, 0.0));
}