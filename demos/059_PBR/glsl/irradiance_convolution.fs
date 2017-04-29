#version 330 core

in vec3 position_ws;

uniform samplerCube environmentMap;

out vec4 FragmentColor;


const float half_pi = 3.14159265359f;
const float pi = 3.14159265359f;
const float two_pi = 3.14159265359f;

const int Q = 40;                               // must be even
const float delta = pi / Q;


void main()
{		
    vec3 N = normalize(position_ws);
    
    vec3 right = cross(vec3(0.0f, 1.0f, 0.0f), N);
    vec3 up = cross(N, right);
       
    vec3 irradiance = vec3(0.0f);

    for(float phi = 0.0; phi < two_pi; phi += delta)
    {
        vec3 aux_axis = cos(phi) * right + sin(phi) * up;
        for(float theta = 0.0; theta < half_pi; theta += delta)
        {
            float cos_theta = cos(theta);
            float sin_theta = sin(theta);
            vec3 sample_ws = sin_theta * aux_axis + cos_theta * N;
            irradiance += texture(environmentMap, sample_ws).rgb * cos_theta * sin_theta;
        }
    }

    irradiance = (pi / Q * Q) * irradiance;
    
    FragmentColor = vec4(irradiance, 1.0);
}
