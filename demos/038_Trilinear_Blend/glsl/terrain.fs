#version 330 core

in float value;
in vec2 uv;
in vec3 position_ws;
in vec3 normal_ws;

uniform sampler2D grass;
uniform sampler2D dirt;

uniform vec3 camera_ws;
uniform vec3 light_ws;

float hermite5(float x)
    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

float smootherstep(float edge0, float edge1, float x)
{
    x = clamp((x - edge0)/(edge1 - edge0), 0.0, 1.0);
    return hermite5(x);
}

out vec4 FragmentColor;

void main()
{
    vec3 l = normalize(light_ws - position_ws);

    vec2 tc = 0.5 + 0.5 * uv;

    vec3 grass_color = texture(grass, tc).rgb;
    vec3 dirt_color  = texture(dirt, tc).rgb;

    float q = 0.5 + 0.5 * max(dot(l, normal_ws), 0.0f);

    float w = smootherstep(0.4, 0.6, value);

    vec3 diffuse = mix(dirt_color, grass_color, hermite5(value));
    
    FragmentColor = vec4(q * diffuse, 1.0);
}
