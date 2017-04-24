#version 330 core
layout (location = 0) out vec4 FragmentColor;
layout (location = 1) out vec4 BrightColor;

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
    vec3 ambient = 0.11 * color;
    // Lighting
    vec3 lighting = vec3(0.0f);
    vec3 v = normalize(camera_ws - position_ws);
    for(int i = 0; i < 4; i++)
    {
        // Diffuse
        vec3 l = normalize(light_ws[i] - position_ws);
        float diff = max(dot(l, normal), 0.0);
        vec3 result = light_color[i] * diff * color;      
        float distance = length(position_ws - light_ws[i]);         // Attenuation (use quadratic as we have gamma correction)
        result *= 1.0 / (distance * distance);
        lighting += result;
                
    }
    vec3 result = ambient + lighting;
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    FragmentColor = vec4(result, 1.0f);

    if(brightness > 1.0)
        BrightColor = FragmentColor;
}