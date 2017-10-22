#version 400 core

in vec3 position;
in vec3 normal;
in float height;

uniform vec3 camera_ws;
uniform vec3 light_ws;

out vec4 FragmentColor;

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec3 light = light_ws - position;
    vec3 view = camera_ws - position;

    vec3 l = normalize(light);
    vec3 v = normalize(view);
    vec3 n = normalize(normal);
    vec3 r = reflect(-l, n);

    vec3 diffuse_color = hsv2rgb(vec3(0.175 + 0.125 * height, 0.75, 0.55 + height));
    vec3 ambient_color = 0.297f * diffuse_color;
    vec3 specular_color = ambient_color + vec3(0.125f);

    float cos_theta = dot(n, l);
    vec3 color = ambient_color + max(cos_theta, 0.0f) * diffuse_color;

    FragmentColor = vec4(color, 1.0f);                 
}