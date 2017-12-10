#version 330 core

in vec3 view;

uniform samplerCube environment_map;

out vec4 FragmentColor;


const float half_pi = 1.57079632679f;
const float pi      = 3.14159265359f;
const float two_pi  = 6.28318530718f;

const int Q = 16;                               // must be even
const float delta = pi / Q;


void main()
{		
    vec3 axis_z = normalize(view);
    vec3 axis_x = normalize(cross(vec3(0.0f, 1.0f, 0.0f), axis_z));
    vec3 axis_y = cross(axis_z, axis_x);

    vec3 irradiance = vec3(0.0f);
    float weight = 0.0f;

    for (float phi = 0.0; phi < two_pi; phi += delta)
    for (int p = 0; p < 2 * Q; ++p)
    {
        float phi = delta * p; 
        vec3 aux_axis = cos(phi) * axis_x + sin(phi) * axis_y;
        for (int t = 0; t < 2 /*Q / 2*/; ++t)
        {
            float theta = delta * t;
            float cos_theta = cos(theta);
            float sin_theta = sin(theta);
            vec3 sample_ws = sin_theta * aux_axis + cos_theta * axis_z;
            float w = cos_theta;
            irradiance += w * texture(environment_map, sample_ws).rgb;
            weight += w;
        }
    }

    irradiance /= weight;
    FragmentColor = vec4(irradiance, 1.0);
}
