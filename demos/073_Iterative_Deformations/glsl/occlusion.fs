#version 330 core

in vec3 position_ws;  
in vec3 normal_ws;  

const vec3 light_ws = vec3(2.0f, 3.0f, 4.0f);
uniform vec3 camera_ws;

out vec4 FragmentColor;


void main()
{
    vec3 v = normalize(camera_ws - position_ws);
    vec3 light = light_ws - position_ws;
    float dist = length(light);
    vec3 l = light / dist;
    vec3 n = normalize(normal_ws);

    vec3 diffuse_color = vec3(1.0, 1.0, 0.0);


    float cos_theta = 0.5 + 0.5 * dot(n, l);

    vec3 color = diffuse_color * 0.25;                                      // ambient component
    vec3 specular_color = vec3(0.707f, 0.707f, 0.707f);
    const float Ns = 64.0f;

    if (cos_theta > 0.0f) 
    {
        float factor = 1.0f / (1.0 + 0.21 * dist);
        color += cos_theta * diffuse_color * factor;

        // Phong lighting
//        vec3 r = reflect(-l, n);
//        float cos_alpha = max(dot(v, r), 0.0f);
//        float exponent = 0.25f * specular_exponent();
        
        // Blinn - Phong lighting
        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, n), 0.0f);

        color += pow(cos_alpha, Ns) * specular_color * factor;
    }
    FragmentColor = vec4(color, 1.0f);
}

 