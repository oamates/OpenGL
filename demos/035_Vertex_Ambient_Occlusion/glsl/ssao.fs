#version 330 core

out float FragmentOcclusion;
in vec2 uv;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform mat4 projection_matrix;
uniform vec3 samples[64];

const int KERNEL_SIZE = 64;
const float radius = 1.25f;
const float bias = 0.025;
const vec2 noiseScale = vec2(1920.0 / 16.0, 1080.0 / 16.0); 

void main()
{
    vec3 position_cs = texture(gPosition, uv).xyz;
    vec3 normal_cs   = normalize(texture(gNormal, uv).rgb);
    vec3 random_vector  = normalize(texture(texNoise, uv * noiseScale).xyz);

    vec3 tangent_cs = normalize(random_vector - normal_cs * dot(random_vector, normal_cs));
    vec3 bitangent_cs = cross(normal_cs, tangent_cs);
    mat3 TBN = mat3(tangent_cs, bitangent_cs, normal_cs);

    float occlusion = 0.0;

    for(int i = 0; i < KERNEL_SIZE; ++i)
    {
        vec3 sample = TBN * samples[i];
        sample = position_cs + sample * radius; 
        
        vec4 offset = vec4(sample, 1.0);
        offset = projection_matrix * offset;
        offset.xyz /= offset.w;             
        offset.xyz = offset.xyz * 0.5 + 0.5;
        
        float sampleDepth = texture(gPosition, offset.xy).z;
        
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position_cs.z - sampleDepth));
        occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }

    occlusion = 1.0f - (occlusion / KERNEL_SIZE);
    
    FragmentOcclusion = occlusion;
}                                            