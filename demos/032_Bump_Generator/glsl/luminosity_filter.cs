#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

//==============================================================================================================================================================
// Every workgroup will work on 16 x 16 pixel area
// Filters that will be applied to the image will work with 5x5 surrounding window
// hence we need to load image data of size 20 x 20
//
// Predefined compute shader inputs :: 
//  const uvec3 gl_WorkGroupSize      = uvec3(local_size_x, local_size_y, local_size_z)
//  in uvec3 gl_NumWorkGroups         ----- the number of work groups passed to the dispatch function
//  in uvec3 gl_WorkGroupID           ----- the current work group for this shader invocation
//  in uvec3 gl_LocalInvocationID     ----- the current invocation of the shader within the work group
//  in uvec3 gl_GlobalInvocationID    ----- unique identifier of this invocation of the compute shader among all invocations of this compute dispatch call
//==============================================================================================================================================================

layout (binding = 0) uniform image2D diffuse_image;
layout (r32f, binding = 1) uniform image2D luminosity_image;


//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main(void)
{
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    vec3 rgb = imageLoad(diffuse_image, P).rgb;
    float luminosity = dot(rgb, vec3(1.0f));
    imageStore(luminosity_image, P, vec4(luminosity, 0.0, 0.0, 1.0));
}
