#version 430 core

//==============================================================================================================================================================
// Every workgroup will work on 8 x 8 pixel area
//==============================================================================================================================================================
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image2D diffuse_image;
layout (r32f, binding = 1) uniform image2D luminosity_image;

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
    vec3 rgb = imageLoad(diffuse_image, P).rgb;
//    float l = luma_bt709(rgb);
    float l = luma_bt709_gc(rgb);
    imageStore(luminosity_image, P, vec4(l, 0.0, 0.0, 0.0));
}
