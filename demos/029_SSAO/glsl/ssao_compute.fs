#version 330 core

in vec2 uv;

uniform sampler2D position_tex;
uniform sampler2D normal_tex;
uniform sampler2D noise_tex;

const int kernelSize = 64;
uniform vec3 samples[kernelSize];

float radius = 0.5;
float bias = 0.025;

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
        // get sample position in world space
        //======================================================================================================================================================
        vec3 sample_ws = position_ws + radius * (TBN * samples[i]);
        
        //======================================================================================================================================================
        // project sample position onto screen
        //======================================================================================================================================================
        vec4 sample_scr = projection_view_matrix * vec4(sample_ws, 1.0f);
        vec2 sample_ndc = 0.5f + 0.5f * (sample_scr.xy / sample_scr.w);

        //======================================================================================================================================================
        // get sample z-value, range check & accumulate
        //======================================================================================================================================================
        float sample_z = texture(position_tex, sample_ndc).z;

        float range_factor = smoothstep(0.0, 1.0, radius / abs(position_ws.z - sample_z));
        occlusion += (sample_z >= sample_z + bias ? 1.0 : 0.0) * range_factor;
    }
    
    FragmentOcclusion = 1.0f - (occlusion / kernelSize);
}                                            