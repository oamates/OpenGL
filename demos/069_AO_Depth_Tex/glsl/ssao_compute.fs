#version 330 core

in vec2 uv;
in vec3 view;

const int kernel_size = 64;

uniform sampler2D noise_tex;
uniform sampler2D normal_cs_tex;

uniform vec2 inv_focal_scale;
uniform vec4 samples[kernel_size];
uniform float radius;
uniform float bias;

out float FragmentOcclusion;

void main()
{
    //==========================================================================================================================================================
    // Get camera-space normal and Z
    //==========================================================================================================================================================
    vec4 data_in = texture(normal_cs_tex, uv);
    vec3 N_cs = normalize(data_in.xyz);
    float Z_cs = data_in.w; 
    vec3 position_cs = -Z_cs * view;
    
    //==========================================================================================================================================================
    // Iterate over the sample kernel and calculate occlusion factor
    //==========================================================================================================================================================
    float occlusion = 0.0;
    float total = 0.0f;

    for(int i = 0; i < kernel_size; ++i)
    {
        vec3 sample = samples[i].xyz;
        float dp = dot(sample, N_cs);

        if (dp > 0.0f)
        {
            float radius = samples[i].w;
            //======================================================================================================================================================
            // get sample position in camera space and in ndc
            //======================================================================================================================================================
            vec3 sample_cs = position_cs + radius * sample;
            float sample_Z_cs = sample_cs.z;

            vec2 ndc = inv_focal_scale * sample_cs.xy / (-sample_Z_cs);
            vec2 uv = 0.5f + 0.5f * ndc;

            float actual_Z_cs = texture(normal_cs_tex, uv).w;

            //======================================================================================================================================================
            // get sample z-value, range check & accumulate
            //======================================================================================================================================================
            occlusion += (sample_Z_cs > actual_Z_cs + 0.05) ? 1.0 : 0.0;
            total += 1.0;
        }
    }
    
    FragmentOcclusion = occlusion / total;
}                                            