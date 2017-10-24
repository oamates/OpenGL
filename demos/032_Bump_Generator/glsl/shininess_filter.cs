#version 430 core

//==============================================================================================================================================================
// Every workgroup will work on 8 x 8 pixel area
//==============================================================================================================================================================
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image2D diffuse_image;
layout (r32f, binding = 5) uniform image2D shininess_image;

vec3 diffuse_color(ivec2 P)
{
    return imageLoad(diffuse_image, P).rgb;
}

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main(void)
{
    ivec2 B = ivec2(gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    ivec2 Pmin = max(P - 1, -P);
    ivec2 Pmax = min(P + 1, 2 * B - P - 2);

    imageStore(shininess_image, P, vec4(1.0, 0.0, 0.0, 0.0));
}
