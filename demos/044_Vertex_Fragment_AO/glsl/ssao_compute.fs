#version 330 core

in vec2 uv;

uniform sampler2D depth_tex;
uniform mat4 camera_matrix;
uniform mat4 projection_view_matrix;

const int KERNEL_SIZE = 64;
uniform vec3 ssao_kernel[KERNEL_SIZE];

out float FragmentOcclusion;

const float radius = 0.75f;

void main()
{  
    float depth = texture(depth_tex, uv).x;
    float z = 2.0f / (depth - 1.0f);
    vec4 position_cs4 = vec4(z * vec3(2.0f * uv - 1.0f, 1.0f), 1.0f);
    vec4 position_ws4 = camera_matrix * position_cs4;

    vec3 position_ws = position_ws4.xyz;

    float AO = 0.0f;
    for (int i = 0; i < KERNEL_SIZE; ++i) 
    {
        vec3 sample_ws = position_ws + radius * ssao_kernel[i];

        vec4 offset = projection_view_matrix * vec4(sample_ws, 1.0f);
        vec2 sample_ndc = 0.5f + 0.5f * (offset.xy / offset.w);
            
        float sample_depth = texture(depth_tex, sample_ndc).x;
        float sample_z = 2.0f / (sample_depth - 1.0f);

        if (abs(z - sample_z) < radius) AO += step(sample_z, position_ws.z);
    }

    FragmentOcclusion = 0.5f; //AO / 64.0f;
}