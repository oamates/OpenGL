#version 330 core

uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex; 

in vec3 view;
in vec3 light;
in vec3 normal;
in vec3 tangent_x;
in vec3 tangent_y;
in vec2 uv;

out vec4 FragmentColor;

const float Ns = 20.0f;

void main()
{    
    vec3 l = normalize(light);
    vec3 n = texture(normal_tex, uv).xyz - vec3(0.5f, 0.5f, 0.5f);
    vec3 b = normalize(n.x * tangent_x + n.y * tangent_y + n.z * normal);
    vec3 v = normalize(view);
    
    vec3 diffuse_color = texture(diffuse_tex, uv).rgb;
    vec3 ambient_color = 0.125f * diffuse_color;
    vec3 specular_color = vec3(1.0f);

    float cos_theta = dot(b, l);

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
        float cos_alpha = max(dot(h, b), 0.0f);

        color += pow(cos_alpha, Ns) * specular_color;
    }

    FragmentColor = vec4(color, 1.0f);                 
}





