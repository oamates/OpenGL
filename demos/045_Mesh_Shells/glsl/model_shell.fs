#version 330 core

in vec3 position_ws;
in vec3 normal_ws;

uniform vec3 color;
uniform vec3 camera_ws;
uniform vec3 light_ws;

out vec4 FragmentColor;

void main()
{
    vec3 n = normalize(normal_ws);

    vec3 view = camera_ws - position_ws;
    vec3 v = normalize(view);

    vec3 light = light_ws - position_ws;
    float light_distance = length(light);
    vec3 l = light / light_distance;                                      

    float diffuse_factor = 0.0f;
    float specular_factor = 0.0f;
    float cos_theta = 0.5 + 0.5 * dot(n, l);

    vec3 ambient = 0.175 * color;
    vec3 diffuse = color;


    if (cos_theta > 0.0f) 
    {
        // Phong lighting
        //vec3 r = reflect(-l, n);
        //float cos_alpha = max(dot(v, r), 0.0f);
        //float exponent = 0.25f * Ns;
        diffuse_factor = cos_theta;
        
        // Blinn - Phong lighting
        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, n), 0.0f);

        specular_factor = 0.125 * pow(cos_alpha, 40.0);
    }
    vec3 color = ambient + diffuse_factor * diffuse + vec3(specular_factor);

    FragmentColor = vec4(color, 1.0f);
}
