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

    float cos_theta = abs(dot(n, l));

    FragmentColor = 0.25 * (0.5f + 0.5f * cos_theta) * vec4(diffuse_color, 1.0f);
}