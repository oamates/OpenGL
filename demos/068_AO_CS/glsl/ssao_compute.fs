#version 330 core

in vec2 uv;
in vec3 view;

const int kernel_size = 32;

uniform sampler2D normal_cs_tex;

uniform vec2 inv_focal_scale;
uniform vec4 samples[kernel_size];
uniform float radius;
uniform float bias;

out float FragmentOcclusion;

const mat3 hash_matrix = mat3
(
    vec3(11.0f, 14.0f, 71.0f),
    vec3(78.0f, 13.0f, 57.0f),
    vec3(22.0f, 19.0f, 17.0f)
);

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
    // Get random vector tangent vector and compute local tangent and bitangent
    //==========================================================================================================================================================
    vec3 rand_vec3 = fract(41719.73157 * cos(hash_matrix * vec3(gl_FragCoord.xy, 1.0)));
    vec3 T_cs = normalize(rand_vec3 - N_cs * dot(rand_vec3, N_cs));
    mat3 tbn = mat3(T_cs, cross(N_cs, T_cs), N_cs);

    //==========================================================================================================================================================
    // Iterate over the sample kernel and calculate occlusion factor
    //==========================================================================================================================================================
    float occlusion = 0.0;

    for(int i = 0; i < 32; ++i)
    {
        //======================================================================================================================================================
        // get sample position in camera space and in ndc
        //======================================================================================================================================================
        float radius = samples[i].w;
        vec3 sample_cs = position_cs + 0.125 * radius * (tbn * samples[i].xyz);        
        float sample_Z_cs = sample_cs.z;

        vec2 ndc = inv_focal_scale * sample_cs.xy / (-sample_Z_cs);
        vec2 uv = 0.5f + 0.5f * ndc;

        float actual_Z_cs = texture(normal_cs_tex, uv).w;

        //======================================================================================================================================================
        // get sample z-value, range check & accumulate
        //======================================================================================================================================================
        occlusion += (sample_Z_cs > actual_Z_cs + 0.05) ? 1.0 : 0.0;
    }
    
    FragmentOcclusion = occlusion / kernel_size;
}                                            