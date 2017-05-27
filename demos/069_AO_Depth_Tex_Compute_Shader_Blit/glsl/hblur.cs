#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

//==============================================================================================================================================================
// Predefined compute shader inputs :: 
//  const uvec3 gl_WorkGroupSize      = uvec3(local_size_x, local_size_y, local_size_z)
//  in uvec3 gl_NumWorkGroups         ----- the number of work groups passed to the dispatch function
//  in uvec3 gl_WorkGroupID           ----- the current work group for this shader invocation
//  in uvec3 gl_LocalInvocationID     ----- the current invocation of the shader within the work group
//  in uvec3 gl_GlobalInvocationID    ----- unique identifier of this invocation of the compute shader among all invocations of this compute dispatch call
//==============================================================================================================================================================

layout (r32f, binding = 1) uniform image2D hblurred_image;

uniform sampler2D data_in;
uniform vec3 texel_size;            // = inverse resolution and 0 the last component

//==============================================================================================================================================================
// Kernel width 7 x 7                                                                                                                                           
//==============================================================================================================================================================
// const int steps = 2;                                                                                                                                          
// const float weight[steps] = float[] (0.44908, 0.05092);
// const float offset[steps] = float[] (0.53805, 2.06278);

//==============================================================================================================================================================
// Kernel width 11 x 11
//==============================================================================================================================================================
const int steps = 3;
const float weight[steps] = float[] (0.25134, 0.17234, 0.07632);
const float offset[steps] = float[] (0.57107, 2.10447, 4.13467);

//==============================================================================================================================================================
// Kernel width 15 x 15
//==============================================================================================================================================================
// const int steps = 4;
// const float weight[steps] = float[] (0.20754, 0.17234, 0.07632, 0.04380);
// const float offset[steps] = float[] (0.57107, 2.17441, 4.25481, 6.19675);

//==============================================================================================================================================================
// Kernel width 35 x 35                                                                                                                                         
//==============================================================================================================================================================
// const int steps = 9;
// const float weight[steps] = float[] (0.10855, 0.13135, 0.10406, 0.07216, 0.04380,  0.02328,  0.01083,  0.00441,  0.00157);
// const float offset[steps] = float[] (0.66293, 2.47904, 4.46232, 6.44568, 8.42917, 10.41281, 12.39664, 14.38070, 16.36501);

void main()
{
    vec2 texel_center = vec2(gl_GlobalInvocationID.xy) + vec2(0.5);
    vec2 uv_center = texel_size.xy * texel_center;
    vec2 uv_offset;
    float q = 0.0f;

    for(int s = 0; s < steps; ++s)                                                                                                                             
    {                                                                                                                                                                
        uv_offset = offset[s] * texel_size.xz;                                                                                                           
        q += weight[s] * (texture(data_in, uv_center + uv_offset).r + texture(data_in, uv_center - uv_offset).r);
    }                                                                                                                                                                

    imageStore(hblurred_image, ivec2(gl_GlobalInvocationID.xy), vec4(q, 0.0, 0.0, 0.0));
}
