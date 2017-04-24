#version 330 core

uniform float time;
uniform vec3 mouse;
uniform vec2 resolution;

uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;

out vec4 FragmentColor;

float sphere(vec3 p, vec3 rd, float r)
{
    float b = dot(-p, rd);
    float inner = b * b - dot(p,p) + r * r;
    return (inner < 0.0f) ? -1.0f : b - sqrt(inner);
}

vec3 kset(vec3 p) 
{
    float m = 1000.0f;
    float mi = 0.0f;
    for (float i = 0.0f; i < 15.0f; i++)
    {
        p = abs(p) / dot(p,p) - 1.0f;
        float l = length(p);
        if (l < m)
        {
            m = l;
            mi = i;
        }
    }
    float factor = pow(max(0.0f, 1.0f - m), 2.5f + 0.3f * sin(25.0f * m + 1.5f * time));
    return factor * normalize(3.0 + texture(iChannel0, vec2(mi * 0.218954)).xyz);
}

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    vec2 mo = mouse.z < 0.0f ? vec2(0.1f, 0.2f) : ((mouse.xy / resolution.xy) - vec2(0.5f));
    uv = (uv - vec2(0.5f)) * resolution.x / resolution.y;
    
    float t = time * 0.03;    
    vec3 ro = -vec3(mo, 2.5f - 0.7f * sin(t * 3.7535f));
    vec3 rd = normalize(vec3(uv, 3.0f));
    vec3 v = vec3(0.0f);
    float c = cos(t);
    float s = sin(t);
    mat2 rot = mat2(c, -s, s, c);
    
    for (float i = 20.0f; i < 40.0f; i++)
    {
        float tt = sphere(ro, rd, i * 0.03);
        vec3 p = ro + rd * tt;
        p.xy *= rot;
        p.xz *= rot;
        v = 0.9f * v + 0.5f * kset(p + p) * step(0.0f, tt);
    }

    FragmentColor = vec4(v * v * v * v * v * v * vec3(1.2f, 1.05f, 0.9f), 1.0f);
}