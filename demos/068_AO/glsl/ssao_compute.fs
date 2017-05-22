#version 330 core

in vec2 uv;
in vec3 view;

const int kernelSize = 32;

uniform sampler2D noise_tex;
uniform sampler2D normal_cs_tex;

uniform mat4 projection_matrix;
uniform vec3 samples[kernelSize];
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
    vec3 position_cs = Z_cs * view;
    
    //==========================================================================================================================================================
    // Get random vector tangent vector and compute local tangent and bitangent
    //==========================================================================================================================================================
    vec3 rand_vec3 = texture(noise_tex, uv).xyz;
    vec3 T_cs = normalize(rand_vec3 - N_cs * dot(rand_vec3, N_cs));
    vec3 B_cs = cross(N_cs, T_cs);
    mat3 tangent_frame = mat3(T_cs, B_cs, N_cs);

    //==========================================================================================================================================================
    // Iterate over the sample kernel and calculate occlusion factor
    //==========================================================================================================================================================
    float occlusion = 0.0;

    for(int i = 0; i < kernelSize; ++i)
    {
        //======================================================================================================================================================
        // get sample position in camera space and in ndc
        //======================================================================================================================================================
        vec3 sample_cs = position_cs + radius * (tangent_frame * samples[i]);

        vec4 sample_scr = projection_matrix * vec4(sample_cs, 1.0f);
        vec2 sample_ndc = 0.5f + 0.5f * (sample_scr.xy / sample_scr.w);

        float actual_Z_cs = texture(normal_cs_tex, sample_ndc).w;

        //======================================================================================================================================================
        // get sample z-value, range check & accumulate
        //======================================================================================================================================================
        occlusion += (actual_Z_cs >= sample_cs.z + bias ? 1.0 : 0.0);
    }
    
    FragmentOcclusion = occlusion / kernelSize;
}                                            