#version 430 core

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

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
layout (r32ui, binding = 0) uniform uimage3D sdf_tex;
layout (rgba32f, binding = 1) uniform imageBuffer cloud_buffer;

uniform int cloud_size;


void main()
{
    //==========================================================================================================================================================
    // index in the input buffer this invocation will work with
    //==========================================================================================================================================================
    int id = int(gl_GlobalInvocationID.x);
    if (id < cloud_size) return;

    vec3 point = imageLoad(cloud_buffer, id).xyz;

    //==========================================================================================================================================================
    // determine nearest lattice point index in the input buffer this invocation will work with
    //==========================================================================================================================================================

    ivec3 lattice_point = ivec3(round(127.5 * (1.0 + clamp(point, -1.0, 1.0))));

    for(int i = -2; i <= 2; ++i)
    for(int j = -2; j <= 2; ++j)
    for(int k = -2; k <= 2; ++k)
    {
        uint q = i + j + k; 
        imageAtomicMin(sdf_tex, lattice_point + ivec3(i, j, k), q);
    }
}
