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

uniform sampler2D data_cs;

uniform vec2 resolution;
uniform vec4 texel_size;            // = inverse resolution and 0 the last component
uniform vec2 focal_scale;
uniform vec2 inv_focal_scale;

const int kernel_size = 32;
uniform vec4 samples[kernel_size];

const float two_pi = 6.28318530718;
const float half_pi = 1.57079632679;
const float znear = 0.5;

const mat3 hash_matrix = mat3
(
    vec3(11.0, 14.0, 71.0),
    vec3(78.0, 13.0, 57.0),
    vec3(22.0, 19.0, 17.0)
);

void main()
{
    vec2 uv = (vec2(gl_GlobalInvocationID.xy) + vec2(0.5)) / resolution;

    vec4 g = texture(data_cs, uv);

    float Z = g.w;
    vec2 ndc = 2.0 * uv - 1.0;
    vec3 view = vec3(focal_scale * ndc, -1.0f);
    vec3 P = -Z * view;

    vec3 n = normalize(g.xyz);

    vec3 T = fract(41719.73157 * cos(hash_matrix * vec3(gl_GlobalInvocationID.xy, 1.0)));
    vec3 t = normalize(T - dot(T, n) * n);
    mat3 tbn = mat3(t, cross(n, t), n);
    
    float ao = 0.0;
    float W = 0.0f;
    
    for (int i = 0; i < kernel_size; i++)
    {
        float radius = samples[i].w;
        vec3 sample_cs = P + 0.125 * radius * (tbn * samples[i].xyz);

        float w = samples[i].z;
        float sample_Z = sample_cs.z;

        vec2 sample_ndc = inv_focal_scale * sample_cs.xy / (-sample_Z);
        vec2 sample_uv = 0.5f + 0.5f * sample_ndc;

        float actual_Z = texture(data_cs, sample_uv).w;

        ao += w * smoothstep(actual_Z, actual_Z + 0.00125 * abs(sample_Z), sample_Z);
        W += w;
    }

    ao /= W;

    imageStore(ssao_image, ivec2(gl_GlobalInvocationID.xy), vec4(ao, 0.0, 0.0, 0.0));

/*
    barrier();

    float ao_values[7];

    //====================================================================================================================================================================================================================
    // Horizontal blur
    //====================================================================================================================================================================================================================
    for (int i = -3; i <= 3; ++i)
        ao_values[i + 3] = imageLoad(ssao_image, ivec2(gl_GlobalInvocationID.xy) + ivec2(i, 0)).r;        

    barrier();

    ao = 0;
    for (int i = -3; i <= 3; ++i)
        ao += ao_values[i + 3];
    ao /= 7.0;

    imageStore(ssao_image, ivec2(gl_GlobalInvocationID.xy), vec4(ao, 0.0, 0.0, 0.0));

    //====================================================================================================================================================================================================================
    // Vertical blur
    //====================================================================================================================================================================================================================

    for (int i = -3; i <= 3; ++i)
        ao_values[i + 3] = imageLoad(ssao_image, ivec2(gl_GlobalInvocationID.xy) + ivec2(0, i)).r;        

    barrier();

    ao = 0;
    for (int i = -3; i <= 3; ++i)
        ao += ao_values[i + 3];
    ao /= 7.0;

    imageStore(ssao_image, ivec2(gl_GlobalInvocationID.xy), vec4(ao, 0.0, 0.0, 0.0));

*/
}