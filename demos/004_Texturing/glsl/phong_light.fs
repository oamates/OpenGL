#version 330 core

uniform sampler2D diffuse_texture;
uniform sampler2D normal_texture; 

in vec3 view_direction;
in vec3 light_direction;
in vec3 normal_direction;
in vec3 tangent_x_direction;
in vec3 tangent_y_direction;
in vec2 texture_coord;

out vec4 FragmentColor;

const float Ns = 20.0f;

void main()
{    
    vec3 l = normalize(light_direction);
    vec3 components = texture(normal_texture, texture_coord).xyz - vec3(0.5f, 0.5f, 0.5f);
    vec3 n = normalize(components.x * tangent_x_direction + components.y * tangent_y_direction + components.z * normal_direction);
    vec3 v = normalize(view_direction);                                                         
    vec3 r = reflect(l,n);
    
    vec3 diffuse_color = texture(diffuse_texture, texture_coord).rgb;
    vec3 ambient_color = 0.125f * diffuse_color;
    vec3 specular_color = ambient_color + vec3(0.125f);

    float cos_theta = dot(n, l);

    vec3 color = ambient_color;

    if (cos_theta > 0.0f) 
    {
        color += cos_theta * diffuse_color;

        // Phong lighting
//        vec3 r = reflect(-l, n);
//        float cos_alpha = max(dot(v, r), 0.0f);
//        float exponent = 0.25f * specular_exponent();
        
        // Blinn - Phong lighting
        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, n), 0.0f);

        color += pow(cos_alpha, Ns) * specular_color;
    }

    FragmentColor = vec4(color, 1.0f);                 
}





