#version 330 core

in vec3 view;

uniform samplerCube environment_map;

out vec4 FragmentColor;


const float half_pi = 1.57079632679f;
const float pi      = 3.14159265359f;
const float two_pi  = 6.28318530718f;

const int Q = 40;                               // must be even
const float delta = half_pi / Q;


void main()
{		
    vec3 axis_z = normalize(view);
    vec3 axis_x = normalize(cross(vec3(0.0f, 1.0f, 0.0f), axis_z));
    vec3 axis_y = cross(axis_z, axis_x);

    vec3 irradiance = vec3(0.0f);
    float weight = 0.0f;

    //==========================================================================================================================================================
    // Spherical coordinate system used : 
    //  x = r * cos(phi) * sin(theta)
    //  y = r * sin(phi) * sin(theta)
    //  z = r * cos(theta)
    // For fixed r = 1, sphere surface element is sin(theta) * d(theta) * d(phi)
    //==========================================================================================================================================================
    

    for (int p = 0; p < 4 * Q; ++p)
    {
        float phi = delta * p;

        float cs_phi = cos(phi);
        float sn_phi = sin(phi);

        vec3 axis0 =  cs_phi * axis_x + sn_phi * axis_y;
        vec3 axis1 = -sn_phi * axis_x + cs_phi * axis_y;
        vec3 axis2 = -cs_phi * axis_x - sn_phi * axis_y;
        vec3 axis3 =  sn_phi * axis_x - cs_phi * axis_y;

        for (int t = 0; t < Q; ++t)
        {
            float theta = delta * t;

            float cos_theta = cos(theta);
            float sin_theta = sin(theta);

            vec3 z_comp = cos_theta * axis_z;

            float w = sin_theta * cos_theta;        /* weight : area element (sin_theta) * lambert cosine factor */

            irradiance += w * (texture(environment_map, z_comp + sin_theta * axis0).rgb + 
                               texture(environment_map, z_comp + sin_theta * axis1).rgb + 
                               texture(environment_map, z_comp + sin_theta * axis2).rgb + 
                               texture(environment_map, z_comp + sin_theta * axis3).rgb);
            weight += w;
        }
    }

    float inv_area = 0.25f / weight;
    FragmentColor = vec4(inv_area * irradiance, 1.0);
    FragmentColor = texture(environment_map, view);
}
