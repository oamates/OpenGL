#version 430 core

//==============================================================================================================================================================
// Every workgroup will work on 8 x 8 pixel area
//==============================================================================================================================================================
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D diffuse_tex;
layout (r32f) uniform image2D luma_image;
uniform vec2 texel_size;

const vec3 rgb_power_bt709 = vec3(0.2126f, 0.7152f, 0.0722f);

float luma_bt709(vec3 rgb)
    { return dot(rgb, rgb_power_bt709); }

float luma_bt709_gc(vec3 rgb)
    { return dot(pow(rgb, vec3(2.2f)), rgb_power_bt709); }

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = texel_size * (vec2(P) + 0.5);

    vec3 c = texture(diffuse_tex, uv).rgb;
//    float l = luma_bt709(c);
    float l = luma_bt709_gc(c);
    imageStore(luma_image, P, vec4(l, 0.0, 0.0, 0.0));
}