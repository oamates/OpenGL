#version 330 core
out vec4 FragmentColor;

in vec3 position_ws;
in vec3 normal_ws;
in vec2 uv;

uniform vec3 light_ws[4];
uniform vec3 light_color[4];

uniform sampler2D diffuse_texture;
uniform vec3 camera_ws;

void main()
{           
    vec3 color = texture(diffuse_texture, uv).rgb;
    vec3 normal = normalize(normal_ws);
    // Ambient
    vec3 ambient = 0.0 * color;
    // Lighting
    vec3 lighting = vec3(0.0f);
    for(int i = 0; i < 4; i++)
    {
        // Diffuse
        vec3 l = normalize(light_ws[i] - position_ws);
        float diff = max(dot(l, normal), 0.0);
        vec3 diffuse = light_color[i] * diff * color;      
        vec3 result = diffuse;        
        // Attenuation (use quadratic as we have gamma correction)
        float distance = length(position_ws - light_ws[i]);
        result *= 1.0 / (distance * distance);
        lighting += result;
    }
    FragmentColor = vec4(ambient + lighting, 1.0f);
}