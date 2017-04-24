#version 330 core

uniform float time;
uniform vec3 mouse;
uniform vec2 resolution;

uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;

out vec4 FragmentColor;

float rnd, sizer;
void randomize(in vec2 p)
{
    rnd = fract(sin(dot(p, vec2(13.3145, 17.7391))) * 317.7654321);
}
vec3 fsign(vec3 p)
{
    return vec3(p.x < 0.0 ? -1.0 : 1.0, p.y < 0.0 ? -1.0 : 1.0, p.z < 0.0 ? -1.0 : 1.0);
}
vec3 paxis(vec3 p)
{
    vec3 a = abs(p);
    return fsign(p) * max(fsign(a - max(a.yzx, a.zxy)), 0.0f);
}

vec3 paxis2(vec3 p)
{
    vec3 a = abs(p);
    return fsign(p) * max(fsign(a - min(a.yzx, a.zxy)), 0.0f);
}

float DE(in vec3 p)
{
    float b = 1.0f;
    for(int i = 0; i < 3; i++)
    {
        p -= paxis(p) * b * sizer;
        b *= 0.5f;
    }
    float d = length(p) - 0.28f;
    for(int i = 0; i < 4; i++)
    {
        p -= paxis2(p) * b;
        b *= 0.5;
    }
    p = abs(p);
    float d2 = max(p.x, max(p.y, p.z)) - b;
    return max(d2, -d);
}

vec4 mcol;
float CE(in vec3 p)
{
    float d = DE(p);
    if(p.y > -1.45)
    {
        vec2 c = floor(p.xz) + p.xy * 0.5f;
        mcol += vec4(vec3(cos(c.y) * sin(c.x), sin(c.y), cos(c.y) * cos(c.x)) * 0.33f + 0.66f, 0.2f);
    }
    else
        mcol += vec4(0.3f, 0.4f, 1.0f, 0.3f);
    return d;
}

float ShadAO(in vec3 ro, in vec3 rd)
{
    float t = 0.0f, s = 1.0f, d, mn = 0.01f + 0.04f * rnd;
    for(int i = 0; i < 8; i++)
    {
        d = max(DE(ro + rd * t) * 1.5f, mn);
        s = min(s, d / t + t * 0.5f);
        t += d;
    }
    return s;
}
vec3 LightDir(in vec3 ro)
{
    float tim = time * 0.1f;
    vec3 LD = vec3(cos(tim), 1.0f, sin(tim)) * 0.707f;
    return LD;
}
vec3 Backdrop(in vec3 rd)
{
    vec3 L = LightDir(rd);
    vec3 col=vec3(0.3f, 0.4f, 0.5f) + rd * 0.1f + vec3(1.0f, 0.8f, 0.6f) * (max(0.0f, dot(rd, L)) * 0.2f + pow(max(0.0f, dot(rd, L)), 40.0f));
    col *= sqrt(0.5f * (rd.y + 1.0f));
    return col;
}

vec3 scene(in vec3 ro, in vec3 rd)
{
    float d = DE(ro) * rnd * 0.15f, t = d, od=1.0, pxl = 1.6 / resolution.y;
    vec4 dm = vec4(1000.0), tm = vec4(-1.0);
    for(int i = 0; i < 64; i++)
    {
        d = DE(ro + rd * t);
        if(d < pxl * t && d < od && tm.w < 0.0)
        {
            dm = vec4(d, dm.xyz);
            tm = vec4(t, tm.xyz);
        }
        t += min(d, 0.1f + t * t * 0.04f);
        od = d;
        if(t > 20.0 || d < 0.00001)
            break;
    }
    if(d < pxl * t && d < dm.x)
    {
        dm.x = d;
        tm.x = t;
    }
    vec3 col = Backdrop(rd);
    vec3 fcol = col;
    for(int i = 0; i < 4; i++)
    {
        if(tm.x < 0.0) break;
        float px = pxl * tm.x;
        vec3 so = ro + rd * tm.x;
        mcol = vec4(0.0);
        vec3 ve = vec3(px,0.0,0.0);
        float d1 = CE(so);
        vec3 dn = vec3(CE(so - ve.xyy), CE(so - ve.yxy), CE(so - ve.yyx));
        vec3 dp = vec3(CE(so + ve.xyy), CE(so + ve.yxy), CE(so + ve.yyx));
        vec3 N = (dp - dn) / (length(dp - vec3(d1)) + length(vec3(d1) - dn));
        vec3 L = LightDir(so);
        vec3 scol = mcol.rgb * 0.14f;
        vec3 R = reflect(rd, N);
        float v = dot(-rd, N), l = dot(N, L);
        float shad=ShadAO(so + N * 0.001f, L);
        vec3 cc = vec3(0.6f, 0.8f, 1.0f),
             lc = vec3(1.0f, 0.8f, 0.6f);
        float cd = exp(-distance(ro, so));
        float spcl = pow(clamp(dot(R, L), 0.0f, 1.0f), 10.0f),
              spcc = pow(max(0.0f, dot(R, -rd)), 1.0f + cd) * 0.25f;
        scol = scol * (cd * v * cc + shad * l * lc) + (cd * spcc * cc + shad * spcl * lc) * mcol.a;
        scol = clamp(scol, 0.0f, 1.0f);
        float fog = min(pow(tm.x, 0.4f) * 0.3f, 1.0f);
        scol = mix(scol, fcol, fog);
        col = mix(scol, col, clamp(dm.x / px, 0.0f, 1.0f));
        dm = dm.yzwx;
        tm = tm.yzwx;
    }
    if(col != col) col = vec3(1.0f, 0.0f, 0.0f);
    return clamp(col * 2.0f, 0.0f, 1.0f);
}

mat3 lookat(vec3 fw)
{
    fw = normalize(fw);
    vec3 rt = normalize(cross(fw, vec3(0.0f, 1.0f, 0.0f)));
    return mat3(rt, cross(rt, fw), fw);
}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    randomize(fragCoord);
    vec3 rd = normalize(vec3((2.0f * fragCoord.xy - resolution.xy) / resolution.y, 1.0f));
    float tim = time;
    sizer = -0.05f + abs(sin(tim * 0.1f));
    vec3 ro = vec3(cos(tim * 0.6) * 3.0f, 1.1f + cos(tim * 0.2f), sin(tim * 0.9f));
    rd = lookat(-ro) * rd;
    FragmentColor = vec4(scene(ro, rd), 1.0f);
}