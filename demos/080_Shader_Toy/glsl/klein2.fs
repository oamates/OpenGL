#version 330 core

uniform float time;
uniform vec3 mouse;
uniform vec2 resolution;

out vec4 FragmentColor;

#define AA 1

vec4 orb; 

float map(vec3 p, float s)
{
    float scale = 1.0;
    orb = vec4(1000.0);
    for(int i = 0; i < 8; i++)
    {
        p = -1.0f + 2.0f * fract(0.5f * p + 0.5f);
        float r2 = dot(p, p);        
        orb = min(orb, vec4(abs(p), r2));
        float k = s / r2;
        p *= k;
        scale *= k;
    }
    return 0.25f * abs(p.y) / scale;
}

float trace(in vec3 ro, in vec3 rd, float s)
{
    float maxd = 30.0f;
    float t = 0.01f;
    for(int i = 0; i < 200; i++)
    {
        float precis = 0.001f * t;        
        float h = map(ro + rd * t, s);
        if((h < precis) || (t > maxd)) break;
        t += h;
    }
    if(t > maxd) t = -1.0f;
    return t;
}

vec3 calcNormal( in vec3 pos, in float t, in float s )
{
    float precis = 0.001 * t;
    vec2 e = vec2(1.0, -1.0) * precis;
    return normalize(e.xyy * map(pos + e.xyy, s) + 
                     e.yyx * map(pos + e.yyx, s) + 
                     e.yxy * map(pos + e.yxy, s) + 
                     e.xxx * map(pos + e.xxx, s));
}

vec3 render( in vec3 ro, in vec3 rd, in float anim )
{    
    vec3 col = vec3(0.0f);                                                                                  // trace
    float t = trace(ro, rd, anim);
    if(t > 0.0)
    {
        vec4 tra = orb;
        vec3 pos = ro + t*rd;
        vec3 nor = calcNormal( pos, t, anim );
        vec3 light1 = vec3( 0.577f, 0.577f, -0.577f);                                                       // lighting
        vec3 light2 = vec3(-0.707f, 0.000f,  0.707f);
        float key = clamp(dot(light1, nor), 0.0f, 1.0f);
        float bac = clamp(0.2f + 0.8f * dot(light2, nor), 0.0f, 1.0f);
        float amb = (0.7f + 0.3f * nor.y);
        float ao = pow(clamp(tra.w * 2.0f, 0.0f, 1.0f), 1.2f);

        vec3 brdf  = vec3(0.4f, 0.4f, 0.4f) * amb * ao;
             brdf += vec3(1.0f, 1.0f, 1.0f) * key * ao;
             brdf += vec3(0.4f, 0.4f, 0.4f) * bac * ao;

        vec3 rgb = vec3(1.0);                                                                               // material
        rgb = mix(rgb, vec3(1.0f, 0.80f, 0.2f), clamp(6.0f * tra.y, 0.0f, 1.0f));
        rgb = mix(rgb, vec3(1.0f, 0.55f, 0.0f), pow(clamp(1.0f - 2.0f * tra.z, 0.0f, 1.0f), 8.0f));
        col = rgb * brdf * exp(-0.2f * t);                                                                  // color
    }

    return sqrt(col);
}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    float time00 = time * 0.25 + 0.01 * mouse.x;
    float anim = 1.1 + 0.5 * smoothstep(-0.3, 0.3, cos(0.1 * time));
    
    vec3 tot = vec3(0.0);
  #if AA>1
    for(int jj = 0; jj < AA; jj++)
        for(int ii = 0; ii < AA; ii++)
  #else
    int ii = 1;
    int jj = 1;
  #endif
    {
        vec2 q = fragCoord.xy + vec2(float(ii), float(jj)) / float(AA);
        vec2 p = (2.0 * q - resolution.xy) / resolution.y;

        // camera
        vec3 ro = vec3(2.8 * cos(0.1f + 0.33f * time00), 0.4f + 0.30f * cos(0.37f * time00), 2.8f * cos(0.5f + 0.35f * time00));
        vec3 ta = vec3(1.9 * cos(1.2f + 0.41f * time00), 0.4f + 0.10f * cos(0.27f * time00), 1.9f * cos(2.0f + 0.38f * time00));
        float roll = 0.2f * cos(0.1f * time00);
        vec3 cw = normalize(ta - ro);
        vec3 cp = vec3(sin(roll), cos(roll), 0.0f);
        vec3 cu = normalize(cross(cw, cp));
        vec3 cv = normalize(cross(cu, cw));
        vec3 rd = normalize(p.x * cu + p.y * cv + 2.0f * cw);

        tot += render(ro, rd, anim);
    }
    tot = tot / float(AA * AA);
    FragmentColor = vec4(tot, 1.0);
}