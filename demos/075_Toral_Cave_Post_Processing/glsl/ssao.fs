#version 330 core

in vec2 uv;
in vec2 view_ray;

uniform sampler2D depth_map;

const float sample_radius = 0.2f;
uniform mat4 projection_matrix;

const int MAX_CLOUD_SIZE = 256;
uniform int cloud_size;
uniform vec3 spherical_rand[MAX_CLOUD_SIZE];

out vec4 FragmentOcclusion;

void main()
{

    float depth = texture(depth_map, uv).x;
    float linear_depth = 2.0f / (depth - 1.0f);
    vec3 position = linear_depth * vec3(view_ray, 1.0f);

    float AO = 0.0f;

    for (int i = 0; i < cloud_size; i++) 
    {
        vec3 sample_position = position + 1.0f * spherical_rand[i];
        vec4 offset = projection_matrix * vec4(sample_position, 1.0f);
		vec2 tc = (offset.xy / offset.w) * 0.5f + vec2(0.5f);
            
        float sample_depth = texture(depth_map, tc).x;
        float sample_linear_depth = 2.0f / (sample_depth - 1.0f);

        if (abs(linear_depth - sample_linear_depth) < sample_radius) AO += step(sample_linear_depth, sample_position.z);
    }

    AO /= 128.0f; 
    FragmentOcclusion = vec4(AO);
}