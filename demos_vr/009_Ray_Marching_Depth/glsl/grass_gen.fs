#version 330 core

uniform sampler2D blade_tex;

uniform vec3 camera_ws;
uniform vec3 light_ws;

in vec3 position;
in vec3 normal;

out vec4 FragmentColor;

vec3 grass_tex(vec3 p, vec3 n)
{
    p += 10.0 * n;
    vec3 txz = texture(blade_tex, p.zx).rgb;
    return txz;
}

void main()
{
    vec3 l = normalize(light_ws - position);
    float q = 0.5f + 0.5f * abs(dot(normal, l));
    vec3 color = grass_tex(position, normal);
    FragmentColor = vec4(q * color, 1.0f);
}


