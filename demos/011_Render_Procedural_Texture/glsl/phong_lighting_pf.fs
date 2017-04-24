#version 330 core

in vec3 position_ws;
in vec3 normal_ws;
in vec3 tangent_x_ws;
in vec3 tangent_y_ws;
in vec2 uv;

uniform vec3 camera_ws;
uniform vec3 light_ws;

uniform sampler2D diffuse_texture;
uniform sampler2D normal_texture;

out vec4 FragmentColor;

void main()
{
    vec3 v = normalize(camera_ws - position_ws);
    vec3 l = normalize(light_ws - position_ws);

    vec3 diffuse_color = texture(diffuse_texture, uv).rgb;
    vec3 normal_direction = 2.0f * texture(normal_texture, uv).xyz - 1.0f;

    vec3 n = normal_direction.x * tangent_x_ws + normal_direction.y * tangent_y_ws + normal_direction.z * normal_ws;

    float cos_theta = 0.5 + 0.5 * dot(n, l);

    vec3 color = vec3(0.0f);
    vec3 specular_color = mix(diffuse_color, vec3(0.707f), 0.5);
    const float Ns = 80.0f;

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

        color += 0.88 * pow(cos_alpha, Ns) * specular_color;
    }

    FragmentColor = vec4(color, 1.0f);                 
}