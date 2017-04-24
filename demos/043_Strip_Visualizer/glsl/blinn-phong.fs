#version 430 core

in vec3 view_direction;
in vec3 light_direction;
in vec3 position;
in vec3 normal;

uniform vec3 Ks;
uniform float Ns;

out vec4 FragmentColor;

layout (std430, binding = 0) buffer shader_data
{
    int strip_id[];
};


void main()
{

    int id = strip_id[gl_PrimitiveID];

    vec3 diffuse_color = vec3(0.5f + 0.5f * sin(31.987161f * id - 425.51337f), 
                              0.5f + 0.5f * sin(12.083593f * id + 124.79121f), 
                              0.5f + 0.5f * sin(47.092147f * id + 161.99211f));
    
    vec3 ambient_color = 0.25 * diffuse_color; 

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
    vec3 color = ambient_color + diffuse_factor * diffuse_color + specular_factor * Ks;

    FragmentColor = vec4(color, 1.0f);
}

