#version 420 core

in vec2 uv;
in float value;
in vec3 normal;

uniform sampler2D grass;
uniform sampler2D dirt;
uniform sampler2D snow;
uniform float time;
uniform float normalizer;

float hermite5(float x)
    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

out vec4 FragmentColor;

void main()
{
    vec2 tc = vec2(1.0) + normalizer * uv;

    vec3 grass_color = texture(grass, tc).rgb;
    vec3 dirt_color  = texture(dirt, tc).rgb;
    vec3 snow_color  = texture(snow, tc).rgb;

    vec3 l = 0.5 * vec3(cos(0.5 * time), sin(0.5 * time), 1.0);

    float q = 0.5 + 0.5 * dot(l, normal);

    float b0 = (1.0 - value) * (1.0 - value), 
          b1 = 2.0 * value * (1.0 - value), 
          b2 = value * value;

    vec3 diffuse = b0 * grass_color + b1 * dirt_color + b2 * snow_color;

    
    
    FragmentColor = vec4(q * diffuse, 1.0);
}
