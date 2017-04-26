#version 330 core

in vec2 uv;

uniform sampler2D position_tex;
uniform sampler2D normal_tex;
uniform sampler2D noise_tex;

const int kernelSize = 64;
uniform vec3 samples[kernelSize];
uniform vec3 camera_ws;

float radius = 1.5;
float bias = 0.25;

const vec2 noiseScale = vec2(1920.0 / 4.0, 1080.0 / 4.0); 

uniform mat4 projection_view_matrix;

out float FragmentOcclusion;

void main()
{
    //==========================================================================================================================================================
    // Get input for SSAO algorithm
    //==========================================================================================================================================================
    vec3 position_ws = texture(position_tex, uv).xyz;
    vec3 normal_ws = texture(normal_tex, uv).xyz;
    vec3 randomVec = normalize(texture(noise_tex, uv * noiseScale).xyz);

    //==========================================================================================================================================================
    // Create TBN change-of-basis matrix: from tangent-space to view-space
    //==========================================================================================================================================================
    vec3 tangent_ws = normalize(randomVec - normal_ws * dot(randomVec, normal_ws));
    vec3 bitangent_ws = cross(normal_ws, tangent_ws);
    mat3 TBN = mat3(tangent_ws, bitangent_ws, normal_ws);

    //==========================================================================================================================================================
    // Iterate over the sample kernel and calculate occlusion factor
    //==========================================================================================================================================================
    float occlusion = 0.0;

    for(int i = 0; i < kernelSize; ++i)
    {
        //======================================================================================================================================================
        // get sample position in world space and in ndc
        //======================================================================================================================================================
        vec3 sample_ws = position_ws + radius * (TBN * samples[i]);
        vec4 sample_scr = projection_view_matrix * vec4(sample_ws, 1.0f);
        vec2 sample_ndc = 0.5f + 0.5f * (sample_scr.xy / sample_scr.w);
        vec3 actual_ws = texture(position_tex, sample_ndc).xyz;
        float sample_dist = length(sample_ws - camera_ws);
        float actual_dist = length(actual_ws - camera_ws);

        //======================================================================================================================================================
        // get sample z-value, range check & accumulate
        //======================================================================================================================================================
        occlusion += (actual_dist >= sample_dist + bias ? 1.0 : 0.0);
    }
    
    FragmentOcclusion = occlusion / kernelSize;
}                                            