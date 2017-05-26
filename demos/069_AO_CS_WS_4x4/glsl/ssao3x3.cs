#version 430 core

layout (local_size_x = 2, local_size_y = 2, local_size_z = 1) in;

//==============================================================================================================================================================
// This invocation will compute occlusion for 3x3 square
//==============================================================================================================================================================

layout (r32f, binding = 0) uniform image2D ssao_image;

uniform sampler2D data_ws;

uniform vec2 resolution;
uniform vec4 texel_size;                // = inverse resolution and 0 the last component
uniform vec2 focal_scale;
uniform vec2 inv_focal_scale;
uniform mat4 projection_view_matrix;
uniform mat3 camera_matrix;
uniform vec3 camera_ws;

const int PIXEL_INPUT_SIZE = 6;
const int CLOUD_SIZE = 9 * PIXEL_INPUT_SIZE;
uniform vec4 samples[CLOUD_SIZE];

const float two_pi = 6.28318530718;
const float half_pi = 1.57079632679;

const mat3 hash_matrix = mat3
(
    vec3(11.0, 14.0, 71.0),
    vec3(78.0, 13.0, 57.0),
    vec3(22.0, 19.0, 17.0)
);

vec2 base_shift[9] = vec2[9]
(
    vec2(-1.0f, -1.0f),
    vec2( 0.0f, -1.0f),
    vec2( 1.0f, -1.0f),
    vec2(-1.0f,  0.0f),
    vec2( 0.0f,  0.0f),
    vec2( 1.0f,  0.0f),
    vec2(-1.0f,  1.0f),
    vec2( 0.0f,  1.0f),
    vec2( 1.0f,  1.0f)
);

ivec2 ibase_shift[9] = ivec2[9]
(
    ivec2(-1, -1),
    ivec2( 0, -1),
    ivec2( 1, -1),
    ivec2(-1,  0),
    ivec2( 0,  0),
    ivec2( 1,  0),
    ivec2(-1,  1),
    ivec2( 0,  1),
    ivec2( 1,  1)
);


void main()
{
    vec2 base = 3.0 * vec2(gl_GlobalInvocationID.xy) + vec2(1.5);

    vec3 position_ws[9];
    vec3 normal_ws[9];

    //==========================================================================================================================================================
    // Generate common point cloud taking few representatives from each pixel
    //==========================================================================================================================================================

    int l = 0;                          // current index into samples (and cloud) array
    vec4 cloud[CLOUD_SIZE];

    for (int i = 0; i < 9; ++i)
    {
        //==================================================================================================================================================
        // Get world-space normal and Z
        //==================================================================================================================================================
        vec2 texel_center = base + base_shift[i];
        vec2 uv = texel_size.xy * texel_center;
        vec2 ndc = 2.0 * uv - 1.0;
        vec3 view = vec3(focal_scale * ndc, -1.0f);

        vec4 g = texture(data_ws, uv);
        vec3 n = normalize(g.xyz);
        normal_ws[i] = n;

        float R = g.w;                                        // distance from fragment to camera 
        vec3 position_cs = R * normalize(view);

        position_ws[i] = camera_ws + camera_matrix * position_cs;

        //==================================================================================================================================================
        // Cook some random tangent-bitangent-normal frame
        //==================================================================================================================================================
        vec3 rand_vec3 = fract(41719.73157 * cos(hash_matrix * vec3(texel_center, 1.0)));    
        vec3 t = normalize(rand_vec3 - dot(rand_vec3, n) * n);
        mat3 tbn = mat3(t, cross(n, t), n);

        for (int k = 0; k < PIXEL_INPUT_SIZE; ++k)
        {
            vec3 s = tbn * samples[l].xyz;
            float radius = samples[l].w;

            vec3 sample_ws = position_ws[i] + 0.0125 * radius * s;
            float sample_R = length(sample_ws - camera_ws);

            vec4 sample_ss = projection_view_matrix * vec4(sample_ws, 1.0f);
            vec2 ndc = sample_ss.xy / sample_ss.w;
            vec2 uv = 0.5f + 0.5f * ndc;

            float actual_R = texture(data_ws, uv).w;

            float ao_input = smoothstep(1.0 * sample_R, (1.0 + 0.06125 / actual_R) * sample_R, actual_R);

            //==============================================================================================================================================
            // ! the point is to read the distance from texture just once and use it multiple times
            //==============================================================================================================================================
            cloud[l++] = vec4(sample_ws, ao_input);
        }
    }

    //==========================================================================================================================================================
    // Now use the whole cloud to compute occlusion of all 4x4 pixels
    //==========================================================================================================================================================
    for (int i = 0; i < 9; ++i)
    {
        vec3 p = position_ws[i];
        vec3 n = normal_ws[i];

        //==================================================================================================================================================
        // Iterate over the sample kernel and calculate occlusion factor
        //==================================================================================================================================================
        float ao = 0.0;
        float W = 0.0f;

        for(int k = 0; k < CLOUD_SIZE; ++k)
        {
            vec4 c = cloud[k];
            vec3 sample_ws = c.xyz;

            vec3 d = sample_ws - p;
            float l = length(d);
            vec3 q = d / l;

            float dp = dot(q, n);
            float w = max(dp, 0.0f) * exp(-2.0 * l);

            ao += w * c.w;
            W += w;
        }

        ao /= W;
        imageStore(ssao_image, 3 * ivec2(gl_GlobalInvocationID.xy) + ibase_shift[i], vec4(ao, 0.0, 0.0, 0.0));
    }
}
