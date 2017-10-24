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

layout (rgba32f, binding = 0) uniform image2D diffuse_image;
layout (r32f, binding = 4) uniform image2D roughness_image;

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

    vec3 bl = diffuse_color(ivec2(Pmin.x, Pmin.y));             // bottom left
    vec3 bc = diffuse_color(ivec2(P.x,    Pmin.y));             // bottom center
    vec3 br = diffuse_color(ivec2(Pmax.x, Pmin.y));             // bottom right
    vec3 cl = diffuse_color(ivec2(Pmin.x, P.y   ));             // center left
    vec3 cc = diffuse_color(ivec2(P.x,    P.y   ));             // center center
    vec3 cr = diffuse_color(ivec2(Pmax.x, P.y   ));             // center right
    vec3 tl = diffuse_color(ivec2(Pmin.x, Pmax.y));             // top left
    vec3 tc = diffuse_color(ivec2(P.x,    Pmax.y));             // top center
    vec3 tr = diffuse_color(ivec2(Pmax.x, Pmax.y));             // top right


    vec3 r = 0.125f * (bl + bc + br + cl + cr + tl + tc + tr) - cc;

    float q = 17.71 * dot(r, r);
    q = clamp(q, 0.0f, 1.0f);

    imageStore(roughness_image, P, vec4(q, 0.0, 0.0, 0.0));
}