#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

//==============================================================================================================================================================
// Predefined compute shader inputs :: 
//  const uvec3 gl_WorkGroupSize      = uvec3(local_size_x, local_size_y, local_size_z)
//  in uvec3 gl_NumWorkGroups         ----- the number of work groups passed to the dispatch function
//  in uvec3 gl_WorkGroupID           ----- the current work group for this shader invocation
//  in uvec3 gl_LocalInvocationID     ----- the current invocation of the shader within the work group
//  in uvec3 gl_GlobalInvocationID    ----- unique identifier of this invocation of the compute shader among all invocations of this compute dispatch call
//==============================================================================================================================================================

//==============================================================================================================================================================
// input  :: point cloud stored in GL_TEXTURE_BUFFER bound to image unit 0
// output :: unsigned integral SDF texture bound to image unit 1
//==============================================================================================================================================================
layout (r32ui, binding = 0) uniform uimage3D sdf_internal;
layout (r32ui, binding = 1) uniform uimage3D sdf_external;

layout (r32f, binding = 2) uniform image3D sdf;



void main()
{
    //==========================================================================================================================================================
    // index in the input buffer this invocation will work with
    //==========================================================================================================================================================
    ivec3 id = ivec3(gl_GlobalInvocationID.xyz);

    uint external_distance = imageLoad(sdf_external, id).x;
    uint internal_distance = imageLoad(sdf_internal, id).x;

    float sd;
    if (external_distance < internal_distance)
    {
        sd = -1.0;
    }
    else
    {
        sd = 1.0;
    }

    imageStore(sdf, id, vec4(sd, 0.0, 0.0, 0.0));
}
