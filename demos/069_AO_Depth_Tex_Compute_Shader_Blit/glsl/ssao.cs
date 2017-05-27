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

layout (r32f, binding = 0) coherent uniform image2D ssao_image;

uniform sampler2D depth_tex;

uniform vec2 resolution;
uniform vec4 texel_size;            // = inverse resolution and 0 the last component
uniform vec2 focal_scale;
uniform vec2 inv_focal_scale;

const int kernel_size = 32;
uniform vec4 samples[kernel_size];

const float two_pi = 6.28318530718;
const float half_pi = 1.57079632679;
const float znear = 0.5;

float csZ(float depth)
{                                
    return znear / (depth - 1.0);
}

vec3 position_cs(vec2 uv)
{
    vec2 ndc = 2.0 * uv - 1.0;
    vec3 view = vec3(focal_scale * ndc, -1.0f);
    float depth = texture(depth_tex, uv).r;
    float Z = csZ(depth);
    vec3 p = -Z * view;
    return p;
}

vec3 position_cs2(vec2 uv, float depth)
{
    vec2 ndc = 2.0 * uv - 1.0;
    vec3 view = vec3(focal_scale * ndc, -1.0f);
    float Z = csZ(depth);
    vec3 p = -Z * view;
    return p;
}

const mat2 hash_matrix = mat2
(
    vec2( 11.173, 41.273),
    vec2(-15.991, 71.457)
);

void main()
{
    vec2 uv = texel_size.xy * (vec2(gl_GlobalInvocationID.xy) + vec2(0.5));

    vec3 P = position_cs(uv);

    vec4 g = textureGather(depth_tex, uv);

    vec3 Pr  = position_cs2(uv + texel_size.xy, g.y);
    vec3 Pl  = position_cs2(uv + texel_size.zw, g.w);
    vec3 Pt  = position_cs2(uv + texel_size.zy, g.x);
    vec3 Pb  = position_cs2(uv + texel_size.xw, g.z);
    
    vec3 dPu = Pr - Pl;
    vec3 dPv = Pt - Pb;

    vec3 n = normalize(cross(dPu, dPv));

    vec2 rand_vec2 = normalize(fract(12657.5719 * cos(hash_matrix * vec2(gl_GlobalInvocationID.xy))));
    vec3 t = normalize(dPu * rand_vec2.x + dPv * rand_vec2.y);

    mat3 tbn = mat3(t, cross(n, t), n);
    
    float ao = 0.0;
    float W = 0.0f;
    
    for (int i = 0; i < kernel_size; i++)
    {
        float radius = samples[i].w;
        vec3 sample_cs = P + radius * (tbn * samples[i].xyz);

        float w = samples[i].z;
        float sample_Z = sample_cs.z;

        vec2 sample_ndc = inv_focal_scale * sample_cs.xy / (-sample_Z);
        vec2 sample_uv = 0.5f + 0.5f * sample_ndc;

        float actual_depth = texture(depth_tex, sample_uv).r;
        float actual_Z = csZ(actual_depth);

        ao += w * smoothstep(actual_Z, actual_Z - 0.025 / sample_Z, sample_Z);
        W += w;
    }

    ao /= W;

    imageStore(ssao_image, ivec2(gl_GlobalInvocationID.xy), vec4(ao, 0.0, 0.0, 0.0));
}