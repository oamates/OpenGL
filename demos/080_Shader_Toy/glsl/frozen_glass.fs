#version 330 core

uniform float time;
uniform vec3 mouse;
uniform vec2 resolution;

uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;

out vec4 FragmentColor;

vec2 mod289(vec2 x) 
    { return x - floor(x * (1.0f / 289.0f)) * 289.0f; }
vec3 mod289(vec3 x) 
    { return x - floor(x * (1.0f / 289.0f)) * 289.0f; }


vec3 permute(vec3 x) 
    { return mod289(((x * 34.0f) + 1.0f) * x); }

float snoise(vec2 v)
{
    const vec4 C = vec4(0.211324865405187f, 0.366025403784439f, -0.577350269189626f, 0.024390243902439f);       
    vec2 i  = floor(v + dot(v, C.yy));                                                                          
    vec2 x0 = v - i + dot(i, C.xx);
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);                                                  
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod289(i);                                                                                              
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 )) + i.x + vec3(0.0, i1.x, 1.0 ));
    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m * m;
    m = m * m;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;                                                                      
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= (1.79284291400159f - 0.85373472095314f * (a0 * a0 + h * h));                                                                       
    vec3 g = vec3(a0.x  * x0.x  + h.x  * x0.y, a0.yz * x12.xz + h.yz * x12.yw);                                 
    return 130.0 * dot(m, g);
}

float fbm(vec2 uv)
{
    mat2 m = mat2(1.2, 1.6, -1.6, 1.2);
    float q = 0.0;
    float amp = 0.47;
    for(int i = 0; i < 8; ++i)
    {
        float c = snoise(uv);
        q += amp * abs(c * c - 0.875);
        amp *= 0.57;
        uv = m * uv;
    }
    float l = 0.5 + 0.5 * sin(0.45 * time);
    return smoothstep(l, 1.0, q * q) - l;
}

vec2 grad(vec2 uv)
{
    float delta = 0.0075;
    float inv_delta = 1.0 / delta;
    vec2 dp = vec2(0.5 * delta, 0.0);
    return inv_delta * vec2(fbm(uv + dp.xy) - fbm(uv - dp.xy), fbm(uv + dp.yx) - fbm(uv - dp.yx));
}

vec3 ambient(vec2 uv)
{
    mat2 m = mat2(1.2, 1.6, -1.6, 1.2);
    vec3 q = vec3(0.0);
    float a = 0.37;
    for(int i = 0; i < 8; ++i)
    {
        vec3 c = 0.5 + 0.5 * vec3(snoise(uv - 0.47 * time)); // uncomment to animate background
        //vec3 c = 0.5 + 0.5 * vec3(snoise(uv));
        q += a * vec3(c.x, c.x * c.y, c.x * c.y * c.z);
        a *= 0.67;
        uv = m * uv;
    }
    return q;
}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 uv = (2.0 * fragCoord.xy - resolution.xy) / resolution.xy;
    uv.x *= (float(resolution.x) / resolution.y);

    vec3 camera = vec3(0.0, 0.0, -1.0);
    vec3 pos = vec3(uv, 0);
    vec3 view = pos - camera;
    
    float f = clamp(fbm(uv), 0.0, 1.0);
    vec2 g = grad(uv);
    vec3 n = normalize(vec3(g, 0.5));
    
    vec3 refracted = -refract(view, n, 0.83);

    vec3 amb = ambient(uv - 0.25 * refracted.xy / refracted.z);
    
    vec3 ICE_COLOR = vec3(0.81, 0.87, 1.07);
    vec3 s = mix(amb, ICE_COLOR, f);

    FragmentColor = vec4(s, 1.0);
}
