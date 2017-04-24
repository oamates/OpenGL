#version 330 core

uniform float time;
uniform vec3 mouse;
uniform vec2 resolution;

uniform sampler2D iChannel0;

out vec4 FragmentColor;

float field(in vec3 p, float s)
{
    float strength = 7.0f + 0.03f * log(1.0e-6 + fract(sin(time) * 4373.11f));
    float accum = 0.25f * s;
    float prev = 0.0f;
    float tw = 0.0f;
    for (int i = 0; i < 26; ++i)
    {
        float mag = dot(p, p);
        p = abs(p) / mag + vec3(-0.5f, -0.4f, -1.5f);
        float w = exp(-float(i) / 7.0f);
        accum += w * exp(-strength * pow(abs(mag - prev), 2.2f));
        tw += w;
        prev = mag;
    }
    return max(0.0f, 5.0f * accum / tw - 0.7f);
}

float field2(in vec3 p, float s)
{
    float strength = 7.0f + 0.03f * log(1.0e-6 + fract(sin(time) * 4373.11f));
    float accum = 0.25f * s;
    float prev = 0.0f;
    float tw = 0.0f;
    for (int i = 0; i < 18; ++i)
    {
        float mag = dot(p, p);
        p = abs(p) / mag + vec3(-0.5f, -0.4f, -1.5f);
        float w = exp(-float(i) / 7.0f);
        accum += w * exp(-strength * pow(abs(mag - prev), 2.2f));
        tw += w;
        prev = mag;
    }
    return max(0.0f, 5.0f * accum / tw - 0.7f);
}

vec3 nrand3(vec2 co)
{
    vec3 a = fract(cos(co.x * 8.3e-3 + co.y) * vec3(1.3e5, 4.7e5, 2.9e5));
    vec3 b = fract(sin(co.x * 0.3e-3 + co.y) * vec3(8.1e5, 1.0e5, 0.1e5));
    vec3 c = mix(a, b, 0.5f);
    return c;
}


void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 uv = 2.0f * fragCoord.xy / resolution.xy - 1.0f;
    vec2 uvs = uv * resolution.xy / max(resolution.x, resolution.y);
    vec3 p = vec3(uvs / 4.0f, 0.0f) + vec3(1.0f, -1.3f, 0.0f);
    p += 0.2f * vec3(sin(time / 16.0f), sin(time / 12.0f),  sin(time / 128.0f));
    
    float freqs[4];
    freqs[0] = texture(iChannel0, vec2(0.01f, 0.25f)).x;
    freqs[1] = texture(iChannel0, vec2(0.07f, 0.25f)).x;
    freqs[2] = texture(iChannel0, vec2(0.15f, 0.25f)).x;
    freqs[3] = texture(iChannel0, vec2(0.30f, 0.25f)).x;

    float t = field(p, freqs[2]);
    float v = (1.0f - exp((abs(uv.x) - 1.0f) * 6.0f)) * (1.0f - exp((abs(uv.y) - 1.0f) * 6.0f));
    
    vec3 p2 = vec3(uvs / (4.0f + sin(time * 0.11f) * 0.2f + 0.2f + sin(time * 0.15f) * 0.3f + 0.4f), 1.5f) + vec3(2.0f, -1.3f, -1.0f);
    p2 += 0.25 * vec3(sin(time / 16.0f), sin(time / 12.0f),  sin(time / 128.0f));
    float t2 = field2(p2, freqs[3]);
    vec4 c2 = mix(0.4f, 1.0f, v) * vec4(1.3f * t2 * t2 * t2, 1.8f * t2 * t2 , t2 * freqs[0], t2);
    
    vec2 seed = p.xy * 2.0f; 
    seed = floor(seed * resolution.x);
    vec3 rnd = nrand3(seed);
    vec4 starcolor = vec4(pow(rnd.y, 40.0f));
    
    //Second Layer
    vec2 seed2 = p2.xy * 2.0f;
    seed2 = floor(seed2 * resolution.x);
    vec3 rnd2 = nrand3(seed2);
    starcolor += vec4(pow(rnd2.y, 40.0f));
    
    FragmentColor = mix(freqs[3] - 0.3f, 1.0f, v) * vec4(1.5f * freqs[2] * t * t * t, 1.2f * freqs[1] * t * t, freqs[3] * t, 1.0f) + c2 + starcolor;
}