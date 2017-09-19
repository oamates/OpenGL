#version 330 core

uniform sampler2DArray blade_tex;

uniform vec3 camera_ws;
uniform vec3 light_ws;

in vec3 position;
in vec3 normal;
in vec3 uv;

out vec4 FragmentColor;


vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
 

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

const float hue = 1.0 / 6.0;

void main()
{
    vec3 l = normalize(light_ws - position);
    float q = (0.5f + 0.5f * uv.y) * (0.5f + 0.5f * abs(dot(normal, l)));
    vec3 rgb = texture(blade_tex, uv).rgb;
    vec3 hsv = rgb2hsv(rgb);
    hsv.x = mix(hsv.x, hue, 0.75);
//    hsv.y *= 0.75;
    hsv.z *= q;

    FragmentColor = vec4(hsv2rgb(hsv), 1.0f);
}


