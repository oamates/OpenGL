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

layout (r32f, binding = 0) uniform image2D ssao_image;

uniform sampler2D depth_tex;

uniform vec2 resolution;
uniform vec3 texel_size;            // = inverse resolution and 0 the last component

uniform vec2 focal_scale;
uniform vec2 inv_focal_scale;

const int kernel_size = 64;
uniform vec4 samples[kernel_size];


float csZ(float depth)
{                                
    //==========================================================================================================================================================
    // d = (-z - 2 * znear) / (-z) -- the value after projective division
    // the value read form depth texture is 0.5 + 0.5 * d = depth
    // 2 * depth - 1 = 1 + 2 * znear / z
    // depth - 1 = znear / z
    // z = znear / (depth - 1) -- negative value
    //==========================================================================================================================================================
    const float znear = 0.5;
    return znear / (depth - 1.0);
}

vec3 csPosition(vec2 uv)
{
    vec2 ndc = 2.0 * uv - 1.0;
    vec3 view = vec3(focal_scale * ndc, -1.0f);
    float depth = texture(depth_tex, uv).r;
    float Z = csZ(depth);
    vec3 position_cs = -Z * view;
    return position_cs;
}

void main()
{
    float FragmentOcclusion;

    vec2 uv = vec2(gl_GlobalInvocationID.xy) / resolution;

    vec3 P   = GetViewPos(uv);    
    vec3 Pr  = GetViewPos(uv + texel_size.xz);
    vec3 Pl  = GetViewPos(uv - texel_size.xz);
    vec3 Pt  = GetViewPos(uv + texel_size.zy);
    vec3 Pb  = GetViewPos(uv - texel_size.zy);



    vec2 ndc = 2.0 * uv - 1.0;
    vec3 view = vec3(focal_scale * ndc, -1.0f);

    float depth = texture(depth_tex, uv).r;
    float Z = csZ(depth);

    vec3 position_cs = -Z * view;

    
    vec3 dPdu = MinDiff(P, Pr, Pl);                                                     // Calculate tangent basis vectors using the minimum difference
    vec3 dPdv = MinDiff(P, Pt, Pb) * (AORes.y * InvAORes.x);

    
    vec3 random = texture(texture1, TexCoord.xy * NoiseScale).rgb;                      // Get the random samples from the noise texture

    // Calculate the projected size of the hemisphere
    vec2 rayRadiusUV = 0.5 * R * FocalLen / -P.z;
    float rayRadiusPix = rayRadiusUV.x * AORes.x;

    float ao = 1.0;

    // Make sure the radius of the evaluated hemisphere is more than a pixel
    if (rayRadiusPix > 1.0)
    {
        ao = 0.0;
        float numSteps;
        vec2 stepSizeUV;

        // Compute the number of steps
        ComputeSteps(stepSizeUV, numSteps, rayRadiusPix, random.z);

        float alpha = 2.0 * PI / numDirections;

        // Calculate the horizon occlusion of each direction
        for(float d = 0; d < numDirections; ++d)
        {
            float theta = alpha * d;

            // Apply noise to the direction
            vec2 dir = RotateDirections(vec2(cos(theta), sin(theta)), random.xy);
            vec2 deltaUV = dir * stepSizeUV;

            // Sample the pixels along the direction
            ao += HorizonOcclusion(deltaUV, P, dPdu, dPdv, random.z, numSteps);
        }

        // Average the results and produce the final AO
        ao = 1.0 - ao / numDirections * AOStrength;
    }



    //==========================================================================================================================================================
    // Iterate over the sample kernel and calculate occlusion factor
    //==========================================================================================================================================================
    float occlusion = 0.0;

    for(int i = 0; i < kernel_size; ++i)
    {
        float radius = samples[i].w;

        //======================================================================================================================================================
        // get sample position in camera space and in ndc
        //======================================================================================================================================================
        vec3 sample_cs = position_cs + radius * samples[i].xyz;
        float sample_Z = sample_cs.z;

        vec2 ndc = inv_focal_scale * sample_cs.xy / (-sample_Z);
        vec2 uv = 0.5f + 0.5f * ndc;

        float actual_depth = texture(depth_tex, uv).r;
        float actual_Z = csZ(actual_depth);

        //======================================================================================================================================================
        // get sample z-value, range check & accumulate
        //======================================================================================================================================================
        occlusion += (sample_Z > actual_Z + 0.01) ? 1.0 : 0.0;
    }
    
    FragmentOcclusion = occlusion / kernel_size;

    imageStore(ssao_image, ivec2(gl_GlobalInvocationID.xy), vec4(FragmentOcclusion, 0.0, 0.0, 0.0));
}