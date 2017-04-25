#version 330 core

in vec2 uv;

uniform sampler2D position_tex;
uniform sampler2D normal_tex;
uniform sampler2D ssao_blurred_tex;

uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform int draw_mode;

out vec4 FragmentColor;


const vec3 light_color = vec3(0.81, 0.87, 0.21);

void main()
{             
    vec3 position_ws = texture(position_tex, uv).rgb;
    vec3 normal_ws = texture(normal_tex, uv).rgb;
    float ao = texture(ssao_blurred_tex, uv).r;

    vec3 diffuse_color = vec3(0.65, 0.53, 0.15);
    vec3 ambient_color = 0.25f * ao * diffuse_color;
    vec3 color = ambient_color;

    vec3 view = camera_ws - position_ws;
    vec3 light = light_ws - position_ws;

    float distance = length(light);
    vec3 v = normalize(view);
    vec3 l = light / distance;

    vec3 diffuse = max(dot(normal_ws, l), 0.0) * diffuse_color * light_color;
    vec3 h = normalize(l + v);  
    float specular_factor = pow(max(dot(normal_ws, h), 0.0), 18.0);

    vec3 specular = light_color * specular_factor;

    float attenuation = 1.0 / (1.0f + 0.09f * distance + 0.032f * distance * distance);
    color = color + attenuation * (diffuse + specular);

    if(draw_mode == 0)
        FragmentColor = vec4(color, 1.0);
    else if(draw_mode == 1)
        FragmentColor = vec4(0.15 * abs(position_ws), 1.0);
     else if(draw_mode == 2)
        FragmentColor = vec4(abs(normal_ws), 1.0);
    else if(draw_mode == 3)
        FragmentColor = vec4(vec3(ao), 1.0);
}
