#version 330 core

uniform float time;
uniform vec3 mouse;
uniform vec2 resolution;

out vec4 FragmentColor;

//#define REFLECTIONS

#define CSize vec3(0.808f, 0.8f, 1.137f)
#define FogColour vec3(0.055f, 0.045f, 0.035f)

vec3  lightPos;
float intensity;

float Hash(float n)
{
    return fract(sin(n) * 43758.5453123);
}

float Noise(in float x)
{
    float p = floor(x);
    float f = fract(x);
    f = f * f * (3.0f - 2.0f * f);
    return mix(Hash(p), Hash(p+1.0), f);
}

float Map(vec3 p)
{
    float scale = 1.0f;
    float add = sin(time) * 0.2f + 0.1f;

    for(int i = 0; i < 9; i++)
    {
        p = 2.0 * clamp(p, -CSize, CSize) - p;
        float r2 = dot(p, p);
        float k = max((1.15f) / r2, 1.0f);
        p *= k;
        scale *= k;
    }
    float l = length(p.xy);
    float rxy = l - 4.0f;
    float n = l * p.z;
    rxy = max(rxy, -(n) / (length(p)) - 0.07f + sin(time * 2.0f + p.x + p.y + 23.5f * p.z) * 0.02f);
    return (rxy) / abs(scale);
}

vec3 pal(in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d)
{
    return a + b * cos(6.28318f * (c * t + d));
}

vec3 Colour( vec3 p)
{
    float col = 0.0f;
    float r2 = dot(p,p);
    float add = sin(time) * 0.2f + 0.1f;
    
    for(int i = 0; i < 10;i++ )
    {
        vec3 p1 = 2.0f * clamp(p, -CSize, CSize) - p;
        col += abs(p.z - p1.z);
        p = p1;
        r2 = dot(p, p);
        float k = max((1.15) / r2, 1.0f);
        p *= k;
    }
    return (0.5f + 0.5f * sin(col * vec3(0.6f, -0.9f, 4.9f))) * 0.75f + 0.15f;
    //return pal(0.5+0.5*sin(col), vec3(0.5,0.5,0.5),vec3(0.5,0.5,0.5),vec3(1.0,1.0,1.0),vec3(0.0,0.33,0.67) );
    //return pal(0.5+0.5*cos(col), vec3(0.8,0.5,0.4),vec3(0.2,0.4,0.2),vec3(2.0,1.0,1.0),vec3(0.0,0.25,0.25) );
}

float RayMarch(in vec3 ro, in vec3 rd)
{
    float precis = 0.001f;
    float h = 0.0f;
    float t = 0.0f;
    float res = 200.0f;
    bool hit = false;
    for(int i = 0; i < 120; i++)
    {
        if (!hit && (t < 12.0f))
        {
            h = Map(ro + rd * t);
            if (h < precis)
            {
                res = t;
                hit = true;
            }
            t += h * 0.83f;
        }
    }
    return res;
}

float Shadow(in vec3 ro, in vec3 rd, float dist)
{
    float res = 1.0f;
    float t = 0.02f;
    float h = 0.0f;
    
    for (int i = 0; i < 14; i++)
    {
        // Don't run past the point light source...
        if(t < dist)
        {
            h = Map(ro + rd * t);
            res = min(4.0f * h / t, res);
            t += 0.0f + h * 0.4f;
        }
    }
    return clamp(res, 0.0f, 1.0f);
}

vec3 Normal(in vec3 pos, in float t)
{
    vec2 eps = vec2(t * t * 0.0075f, 0.0f);
    vec3 nor = vec3(Map(pos + eps.xyy) - Map(pos - eps.xyy),
                    Map(pos + eps.yxy) - Map(pos - eps.yxy),
                    Map(pos + eps.yyx) - Map(pos - eps.yyx));
    return normalize(nor);
}

float LightGlow(vec3 light, vec3 ray, float t)
{
    float ret = 0.0;
    if (length(light) < t)
    {
        light = normalize(light);
        ret = pow(max(dot(light, ray), 0.0f), 2000.0f) * 0.5f;
        float a = atan(light.x - ray.x, light.z - ray.z);
        ret = (1.0f + (sin(a * 10.0f - time * 4.3f) + sin(a * 13.141f + time * 3.141f))) * (sqrt(ret)) * 0.05f + ret;
        ret *= 3.0f;
    }       
    return ret;
}

vec3 RenderPosition(vec3 pos, vec3 ray, vec3 nor, float t)
{
    vec3 col = vec3(0.0f);
    vec3 lPos = lightPos - pos;
    float lightDist = length(lPos);
    vec3 lightDir = normalize(lPos);
    float bri = max(dot(lightDir, nor), 0.0f) * intensity;
    float spe = max(dot(reflect(ray, nor), lightDir), 0.0f);
    float amb = max(abs(nor.z) * 0.04f, 0.025f);
    float sha = Shadow(pos, lightDir, lightDist);
    col = Colour(pos);   
    col = col * bri * sha + pow(spe, 15.0f) * sha * 0.7f + amb * col;
    return col;
}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 q = fragCoord.xy / resolution.xy;
    vec2 p = -1.0f + 2.0f * q;
    p.x *= resolution.x / resolution.y;
    
    float time00 = sin(1.6f + time*.05 + mouse.x*.005)*12.5;
    float height = (smoothstep(9.4, 11.5, abs(time00))*.5);
    vec3 origin = vec3( 1.2, time00+1.0, 2.5+height);
    vec3 target = vec3(.0+sin(time00), 0.0, 2.5-height*4.0);
    lightPos = origin+vec3(-0.56-cos(time00*2.0+2.8)*.3, -1.4, .24+cos(time00*2.0+1.5)*.3);
    intensity = 0.8f + 0.3f * Noise(time * 5.0f);
    
    vec3 cw = normalize( target-origin);
    vec3 cp = normalize(vec3(0.0, sin(time*.25 + mouse.x*.005), 1.80));
    vec3 cu = normalize( cross(cw,cp) );
    vec3 cv = cross(cu,cw);
    vec3 ray = normalize( p.x*cu + p.y*cv + 2.6*cw );   

    vec3 col = vec3(0.0);
    float t = 0.0;
    t = RayMarch(origin, ray);

    if(t < 199.0f)
    {
        vec3 pos = origin + t * ray;
        vec3 nor = Normal(pos, t);
        col = RenderPosition(pos, ray, nor, t);
        
      #ifdef REFLECTIONS
        vec3 ray2    = reflect(ray, nor);
        vec3 origin2 = pos + nor*.01;
        float d = RayMarch(origin2, ray2);
        if(d < 199.0)
        {
            pos = origin2 + d * ray2;
            nor = Normal(pos, d);
            col += RenderPosition(pos, ray, nor, d) * .2;
        }
      #endif
    }
    
    // post-processing effects...

    col = mix(FogColour, col, exp(-0.6f * max(t - 3.0f, 0.0f)));
    col = clamp(mix(col, vec3(0.333f), -0.07f), 0.0f, 1.0f);
    col = pow(col, vec3(0.45f));
    col += LightGlow(lightPos - origin, ray, t) * intensity;

    FragmentColor = vec4(clamp(col, 0.0f, 1.0f), 1.0f);
}