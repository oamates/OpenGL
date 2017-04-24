#version 330 core   

uniform vec3 camera_ws;
uniform vec3 light_ws;                                                        
                                                                                    
uniform sampler2D diffuse_texture;
uniform sampler2D bump_texture; 

in vec3 position_ws;
in vec3 normal_ws;
in vec3 tangent_x_ws;
in vec3 tangent_y_ws;
in vec2 uv;

out vec4 FragmentColor;
                                                                                    
const float Ns = 60.0f;

void main()
{
    vec3 view_direction = camera_ws - position_ws;
    vec3 light_direction = light_ws - position_ws;

    vec3 l = normalize(light_direction);
    vec3 components = texture(bump_texture, uv).xyz - vec3(0.5f, 0.5f, 0.0f);
    vec3 n = normalize(components.x * tangent_x_ws + components.y * tangent_y_ws + components.z * normal_ws);
    vec3 v = normalize(view_direction);                                                         
    vec3 r = reflect(l, n);
    
    vec3 diffuse_color = texture(diffuse_texture, uv).rgb;
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





