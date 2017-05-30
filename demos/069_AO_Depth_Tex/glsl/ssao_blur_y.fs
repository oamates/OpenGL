#version 330 core

in vec2 uv;

uniform sampler2D ssao_input;
uniform vec2 texel_size;

layout (location = 0) out vec2 OcclusionBlurredR;

const int KERNEL_RADIUS = 8;
const float BLUR_DECAY = 0.025f;
const float SHARPNESS = 0.01125f;

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
    vec2 ao_r = texture(ssao_input, uv).rg;
    float center_R = ao_r.y;

    float w = 1.0;
    float total_ao = ao_r.x * w;
    float total_weight = w;

    int i = 1;

    while(i <= KERNEL_RADIUS / 2)
    {
        ao_r = sample_AO_R(vec2(0.0f,  i));
        w = weight(i, ao_r.y, center_R);
        total_ao += ao_r.x * w;
        total_weight += w;

        ao_r = sample_AO_R(vec2(0.0f, -i));
        w = weight(i, ao_r.y, center_R);
        total_ao += ao_r.x * w;
        total_weight += w;

        ++i;
    }

    while(i <= KERNEL_RADIUS)
    {
        ao_r = sample_AO_R( vec2(0.0f,  0.5f + i));
        w = weight(i, ao_r.y, center_R);
        total_ao += ao_r.x * w;
        total_weight += w;

        ao_r = sample_AO_R( vec2(0.0f, -0.5f - i));
        w = weight(i, ao_r.y, center_R);
        total_ao += ao_r.x * w;
        total_weight += w;
        i += 2;
    }    

    float ao = total_ao / total_weight;
    OcclusionBlurredR = vec2(ao, center_R);
}