#version 330 core

in vec3 view_direction;
in vec3 light_direction;
in vec3 position;
in vec3 normal;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;

out vec4 FragmentColor;

void main()
{
    vec3 n = normalize(normal);
    vec3 v = normalize(view_direction);                                                         

    float light_distance = length(light_direction);
    vec3 l = light_direction / light_distance;                                      

    float diffuse_factor = 0.0f;
    float specular_factor = 0.0f;

    float cos_theta = dot(n, l);

    if (cos_theta > 0.0f) 
    {
        diffuse_factor = cos_theta;

        // Phong lighting
        vec3 r = reflect(-l, n);
        float cos_alpha = max(dot(v, r), 0.0f);
        float exponent = 0.25f * Ns;
        
        // Blinn - Phong lighting
        // vec3 h = normalize(l + v);
        // float cos_alpha = max(dot(h, n), 0.0f);
        // float exponent = Ns;

        specular_factor = pow(cos_alpha, exponent);
    }
    vec3 color = Ka + diffuse_factor * Kd + specular_factor * Ks;

    FragmentColor = vec4(color, 1.0f);
}

