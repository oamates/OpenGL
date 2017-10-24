#version 430 core

//==============================================================================================================================================================
// Every workgroup will work on 8 x 8 pixel area
//==============================================================================================================================================================
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D input_image;
layout (rgba32f, binding = 5) uniform image2D blurred_image;

vec3 diffuse_color(vec2 uv)
{
    return texture(diffuse_image, P).rgb;
}

in vec2 uv;

uniform sampler2D input_tex;
uniform vec2 texel_size;

layout (location = 0) out vec2 OcclusionBlurredR;

const int KERNEL_RADIUS = 8;
const float BLUR_DECAY = 0.025f;
const float SHARPNESS = 0.03125f;

vec2 sample_AO_R(vec2 shift)
{
    return texture(ssao_input, uv + texel_size * shift).rg;
}

float weight(float r, float z, float z0)
{
    float dz = z0 - z;
    return exp2(-BLUR_DECAY * r * r - SHARPNESS * dz * dz);
}

void main(void)
{
    vec2 B = vec2(gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    vec2 texel_size = 1.0 / B;
    vec2 uv = 0.5f + texel_size * vec2(gl_GlobalInvocationID.xy);

    vec3 rgb = texture(input_tex, uv);

    float w = 1.0;
    float total_ao = ao_r.x * w;
    float total_weight = w;





    ivec2 Pmin = max(P - 1, -P);
    ivec2 Pmax = min(P + 1, 2 * B - P - 2);

    imageStore(shininess_image, P, vec4(1.0, 0.0, 0.0, 0.0));
}


void main(void)
{
    vec2 ao_r = texture(ssao_input, uv).rg;
    float center_R = ao_r.y;


    int i = 1;

    while(i <= KERNEL_RADIUS / 2)
    {
        ao_r = sample_AO_R(vec2( i, 0.0f));
        w = weight(i, ao_r.y, center_R);
        total_ao += ao_r.x * w;
        total_weight += w;

        ao_r = sample_AO_R(vec2(-i, 0.0f));
        w = weight(i, ao_r.y, center_R);
        total_ao += ao_r.x * w;
        total_weight += w;
        ++i;
    }

    while(i <= KERNEL_RADIUS)
    {
        ao_r = sample_AO_R( vec2( 0.5f + i, 0.0f));
        w = weight(i, ao_r.y, center_R);
        total_ao += ao_r.x * w;
        total_weight += w;

        ao_r = sample_AO_R( vec2(-0.5f - i, 0.0f));
        w = weight(i, ao_r.y, center_R);
        total_ao += ao_r.x * w;
        total_weight += w;
        i += 2;
    }

    float ao = total_ao / total_weight;
    OcclusionBlurredR = vec2(ao, center_R);
}