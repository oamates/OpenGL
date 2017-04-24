#version 330 core

uniform float time;
uniform vec3 mouse;
uniform vec2 resolution;

out vec4 FragmentColor;

float noise(vec3 p)
{
    vec3 i = floor(p);
    vec4 a = dot(i, vec3(1.0f, 57.0f, 21.0f)) + vec4(0.0f, 57.0f, 21.0f, 78.0f);
    vec3 f = 0.5f - 0.5f * cos((p - i) * acos(-1.0f));
    a = mix(sin(cos(a) * a), sin(cos(1.0f + a) * (1.0f + a)), f.x);
    a.xy = mix(a.xz, a.yw, f.y);
    return mix(a.x, a.y, f.z);
}

float sphere(vec3 p, vec4 spr)
{
    return length(spr.xyz - p) - spr.w;
}

float flame(vec3 p)
{
    float d = sphere(p * vec3(1.0f, 0.5f, 1.0f), vec4(0.0f, -1.0f, 0.0f, 1.0f));
    return d + (noise(p + vec3(0.0f, 2.0f * time, 0.0f)) + noise(3.0f * p) * 0.5f) * 0.25f * p.y;
}

float scene(vec3 p)
{
    return min(100.0f - length(p), abs(flame(p)));
}

vec4 raymarch(vec3 org, vec3 dir)
{
    float d = 0.0f, glow = 0.0f, eps = 0.02f;
    vec3  p = org;
    bool glowed = false;
    
    for(int i = 0; i < 64; i++)
    {
        d = scene(p) + eps;
        p += d * dir;
        if(d > eps)
        {
            if(flame(p) < 0.0f)
                glowed = true;
            if(glowed)
                glow = float(i) / 64.0f;
        }
    }
    return vec4(p, glow);
}

void main()
{
    vec2 v = -1.0f + 2.0f * gl_FragCoord.xy / resolution;
    v.x *= resolution.x / resolution.y;
    
    vec3 org = vec3(0.0f, -2.0f, 4.0f);
    vec3 dir = normalize(vec3(v.x * 1.6f, -v.y, -1.5f));
    
    vec4 p = raymarch(org, dir);
    float glow = p.w;
    
    vec4 col = mix(vec4(1.0f, 0.5f, 0.1f, 1.0f), vec4(0.1f, 0.5f, 1.0f, 1.0f), p.y * 0.02f + 0.4f);
    
    FragmentColor = mix(vec4(0.0f), col, pow(glow * 2.0f, 4.0f));
}

