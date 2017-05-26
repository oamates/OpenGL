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

uniform sampler2D data_ws;

uniform vec2 resolution;
uniform vec4 texel_size;            // = inverse resolution and 0 the last component
uniform vec2 focal_scale;
uniform vec2 inv_focal_scale;
uniform mat4 projection_view_matrix;
uniform mat3 camera_matrix;
uniform vec3 camera_ws;

const int kernel_size = 32;
uniform vec4 samples[kernel_size];

const float two_pi = 6.28318530718;
const float half_pi = 1.57079632679;

const mat3 hash_matrix = mat3
(
    vec3(11.0, 14.0, 71.0),
    vec3(78.0, 13.0, 57.0),
    vec3(22.0, 19.0, 17.0)
);

void main()
{
    vec2 uv = (vec2(gl_GlobalInvocationID.xy) + vec2(0.5)) / resolution;
    vec2 ndc = 2.0 * uv - 1.0;
    vec3 view = vec3(focal_scale * ndc, -1.0f);

    //==========================================================================================================================================================
    // Get camera-space normal and Z
    //==========================================================================================================================================================
    vec4 g = texture(data_ws, uv);
    vec3 N_ws = normalize(g.xyz);
    float R = g.w;                                        // distance from fragment to camera 
    vec3 position_cs = R * normalize(view);
    vec3 position_ws = camera_ws + camera_matrix * position_cs;

    //==========================================================================================================================================================
    // Cook some random 2x2 rotation matrix
    //==========================================================================================================================================================
    vec3 T = fract(41719.73157 * cos(hash_matrix * vec3(gl_GlobalInvocationID.xy, 1.0)));
    vec3 t = normalize(T - dot(T, N_ws) * N_ws);
    mat3 tbn = mat3(t, cross(N_ws, t), N_ws);

    //==========================================================================================================================================================
    // Iterate over the sample kernel and calculate occlusion factor
    //==========================================================================================================================================================
    float ao = 0.0;
    float W = 0.0f;

    for(int i = 0; i < kernel_size; ++i)
    {
        vec3 s = tbn * samples[i].xyz;
        float dp = dot(s, N_ws);

        if (dp > 0.0f)
        {
            //======================================================================================================================================================
            // get sample position in camera space and in ndc
            //======================================================================================================================================================
            float radius = samples[i].w;

            vec3 sample_ws = position_ws + 0.125 * radius * s;
            float sample_R = length(sample_ws - camera_ws);

            vec4 sample_ss = projection_view_matrix * vec4(sample_ws, 1.0f);
            vec2 ndc = sample_ss.xy / sample_ss.w;
            vec2 uv = 0.5f + 0.5f * ndc;

            float actual_R = texture(data_ws, uv).w;

            //======================================================================================================================================================
            // get sample distance, range check & accumulate
            //======================================================================================================================================================
            ao += dp * smoothstep(1.0 * sample_R, (1.0 + 0.06125 / actual_R) * sample_R, actual_R);
            W += dp;
        }
    }

    ao /= W;
    imageStore(ssao_image, ivec2(gl_GlobalInvocationID.xy), vec4(ao, 0.0, 0.0, 0.0));

}
