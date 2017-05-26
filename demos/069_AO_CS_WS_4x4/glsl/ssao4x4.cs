#version 430 core

layout (local_size_x = 4, local_size_y = 4, local_size_z = 1) in;

const int SQUARE_SIZE = int(gl_WorkGroupSize.x) * int(gl_WorkGroupSize.y);

//==============================================================================================================================================================
// This invocation will compute occlusion for 4x4 square
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

const int PIXEL_INPUT_SIZE = 4;
const int CLOUD_SIZE = SQUARE_SIZE * PIXEL_INPUT_SIZE;
uniform vec4 samples[CLOUD_SIZE];

const float two_pi = 6.28318530718;
const float half_pi = 1.57079632679;

const mat3 hash_matrix = mat3
(
    vec3(11.0, 14.0, 71.0),
    vec3(78.0, 13.0, 57.0),
    vec3(22.0, 19.0, 17.0)
);

shared vec4 cloud[CLOUD_SIZE];

float attenuation(float distance)
{
    return exp(-2.0 * distance);
}

const int neighbor_pixels[SQUARE_SIZE][8] = 
{
    { 1,  4,  5,  2,  8,  6,  9, 10},
    { 0,  2,  5,  4,  6,  3,  9,  8},
    { 1,  3,  6,  5,  7,  0, 10, 11},
    { 2,  7,  6,  1, 11,  5, 10,  9},

    { 0,  5,  8,  1,  9,  6, 12,  2},
    { 1,  4,  6,  9,  0,  2,  8, 10},
    { 2,  5,  7, 10,  1,  3,  9, 11},
    { 3,  6, 11,  2, 10,  5, 15,  1},

    { 4,  9, 12,  5, 13,  0, 10, 14},
    { 5,  8, 10, 13,  4,  6, 12, 14},
    { 6,  9, 11, 14,  5,  7, 13, 15},
    { 7, 10, 15,  6, 14,  3,  9, 13},
     
    { 8, 13,  9,  4, 14,  5, 10,  6},
    { 9, 12, 14,  8, 10,  5, 15,  4},
    {10, 13, 15,  9, 11,  6, 12,  7},
    {11, 14, 10,  7, 13,  6,  9,  5}
};

void main()
{
    int index = int(gl_LocalInvocationIndex);

    vec2 texel_center = vec2(gl_GlobalInvocationID.xy) + vec2(0.5);
    vec2 uv = texel_size.xy * texel_center;
    vec2 ndc = 2.0 * uv - 1.0;
    vec3 view = vec3(focal_scale * ndc, -1.0f);

    vec4 g = texture(data_ws, uv);

    vec3 n = normalize(g.xyz);

    vec3 position_cs = g.w * normalize(view);                               // g.w = distance from fragment to camera 
    vec3 p = camera_ws + camera_matrix * position_cs;

    vec3 rand_vec3 = fract(41719.73157 * cos(hash_matrix * vec3(texel_center, 1.0)));    
    vec3 t = normalize(rand_vec3 - dot(rand_vec3, n) * n);
    mat3 tbn = mat3(t, cross(n, t), n);

    float ao = 0.0f;
    float W = 0.0f;

    for (int k = 0; k < PIXEL_INPUT_SIZE; ++k)
    {
        vec3 s = tbn * samples[k].xyz;
        float radius = samples[k].w;

        vec3 sample_ws = p + radius * s;
        float sample_R = length(sample_ws - camera_ws);

        vec4 sample_ss = projection_view_matrix * vec4(sample_ws, 1.0f);
        vec2 ndc = sample_ss.xy / sample_ss.w;
        vec2 uv = 0.5f + 0.5f * ndc;

        float actual_R = texture(data_ws, uv).w;

        float ao_input = smoothstep(sample_R, (1.0 + 0.06125 / actual_R) * sample_R, actual_R);
        float w = samples[k].z * attenuation(radius);

        ao += w * ao_input;
        W += w;

        cloud[index * PIXEL_INPUT_SIZE + k] = vec4(sample_ws, ao_input);
    }

    barrier();

    //==========================================================================================================================================================
    // Now use the whole cloud to finish occlusion of all 4x4 pixels
    //==========================================================================================================================================================

    int gs = 0;
    int i = 0;

    while ((i < 8) && (gs < 24))
    {
        int base = neighbor_pixels[index][i] * PIXEL_INPUT_SIZE;

        //======================================================================================================================================================
        // Iterate over the sample kernel and calculate occlusion factor
        //======================================================================================================================================================

        for(int k = 0; k < PIXEL_INPUT_SIZE; ++k)
        {
            vec4 c = cloud[base + k];
            vec3 d = c.xyz - p;

            float l = length(d);
            vec3 q = d / l;
            float dp = dot(q, n);

            if ((l < 0.125) && (dp > 0))
            {
                float w = dp * attenuation(l);
                ao += w * c.w;
                W += w;
                ++gs;
            }
        }
        ++i;
    }
    ao /= W;
    imageStore(ssao_image, ivec2(gl_GlobalInvocationID.xy), vec4(ao, 0.0, 0.0, 0.0));
}
