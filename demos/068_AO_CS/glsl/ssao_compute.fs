#version 330 core

in vec2 uv;
in vec3 view;

const int kernel_size = 32;

uniform sampler2D normal_cs_tex;

uniform vec2 inv_focal_scale;
uniform vec3 samples[kernel_size];
uniform float radius;
uniform float bias;

out float FragmentOcclusion;

const mat3 hash_matrix = mat3
(
    vec3(11.0, 14.0, 71.0),
    vec3(78.0, 13.0, 57.0),
    vec3(22.0, 19.0, 17.0)
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
    vec3 rand_vec3 = fract(41719.73157 * sin(hash_matrix * vec3(gl_FragCoord.xy, 1.0)));

    vec3 T_cs = normalize(rand_vec3 - N_cs * dot(rand_vec3, N_cs));
    vec3 B_cs = cross(N_cs, T_cs);
    mat3 tangent_frame = mat3(T_cs, B_cs, N_cs);

    //==========================================================================================================================================================
    // Iterate over the sample kernel and calculate occlusion factor
    //==========================================================================================================================================================
    float occlusion = 0.0;

    for(int i = 0; i < kernel_size; ++i)
    {
        //======================================================================================================================================================
        // get sample position in camera space and in ndc
        //======================================================================================================================================================
        vec3 sample_cs = position_cs + radius * (tangent_frame * samples[i]);
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