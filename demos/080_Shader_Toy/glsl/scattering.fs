#version 330 core

uniform float time;
uniform vec3 mouse;
uniform vec2 resolution;

uniform sampler2D iChannel0;


out vec4 FragmentColor;

const float tau = 6.28318530717958647692;
float Noise(in vec3 x);

bool toggleR = false;
bool toggleS = false;

const float epsilon = 0.003f;
const float normalPrecision = 0.1f;
const float shadowOffset = 0.1f;
const int traceDepth = 500;
const float drawDistance = 100.0f;

const vec3 CamPos = vec3(0.0f, 40.0f, -40.0f);
const vec3 CamLook = vec3(0.0f, 0.0f, 0.0f);

const vec3 lightDir = vec3(0.7f, 1.0f, -0.1f);
const vec3 fillLightDir = vec3(0.0f, 0.0f, -1.0f);
const vec3 lightColour = vec3(1.1f, 1.05f, 1.0f);
const vec3 fillLightColour = vec3(0.38f, 0.4f, 0.42f);
    
// This should return continuous positive values when outside and negative values inside,
// which roughly indicate the distance of the nearest surface.
float Isosurface( vec3 ipos )
{
    // animate the object rotating
    float ang = time * tau / 25.0f;
    float ang2 = time * tau / 125.0f;
    float s = sin(ang); 
    float c = cos(ang);
    float s2 = sin(ang2); 
    float c2 = cos(ang2);
    vec3 pos;
    pos.y = c * ipos.y - s * ipos.z;
    pos.z = c * ipos.z + s * ipos.y;
    pos.x = ipos.x * c2 + pos.z * s2;
    pos.z = pos.z * c2 - ipos.x * s2;


    // smooth csg
    float smoothing = 0.9f - 0.65f * cos(time * 0.05f);

    return
        log(                                                    // intersection
            1.0 / (                                             // union
                    1.0 / (                                     // intersection
                    
                            exp((length(pos.xz) - 10.0f) / smoothing) +
                            exp((-(length(pos.xz) - 7.0f)) / smoothing) +
                            exp((-(length(vec2(8.0f, 0.0f) + pos.zy) - 5.0f)) / smoothing) +
                            exp((pos.y - 10.0f) / smoothing) +
                            exp((-pos.y - 10.0f) / smoothing)) + 
                            exp(-(length(pos + 15.0f * vec3(sin(time * 0.07f),
                                                            sin(time * 0.13f),
                                                            sin(time * 0.10f))) - 5.0f))
                          )
            // trim it with a plane
            //+ exp((dot(pos,normalize(vec3(-1,-1,1)))-10.0-10.0*sin(iGlobalTime*.17))/smoothing)
        )*smoothing;
        //+ Noise(pos*16.0)*.08/16.0; // add some subtle texture
}


// alpha controls reflection
vec4 Shading( vec3 pos, vec3 norm, vec3 visibility, vec3 rd )
{
    vec3 albedo = vec3(1.0f); //mix( vec3(1,.8,.7), vec3(.5,.2,.1), Noise(pos*vec3(1,10,1)) );
    vec3 l = lightColour * mix(visibility, vec3(1.0f) * max(0.0f, dot(norm, normalize(lightDir))), 0.0f);
    vec3 fl = fillLightColour * (dot(norm, normalize(fillLightDir)) * 0.5f + 0.5f);
    
    vec3 view = normalize(-rd);
    vec3 h = normalize(view + lightDir);
    float specular = pow(max(0.0f, dot(h, norm)), 2000.0f);
    
    float fresnel = pow(1.0f - dot(view, norm), 5.0f);
    fresnel = mix(0.01f, 1.0f, min(1.0f, fresnel));
    
    if (toggleR) fresnel = 0.0;
    return vec4(albedo * (l + fl) * (1.0f - fresnel) + visibility * specular * 32.0f * lightColour, fresnel);
}

const vec3 FogColour = vec3(0.1f, 0.2f, 0.5f);

vec3 SkyColour(vec3 rd)
{
    return vec3(0.031f, 0.011f, 0.076f);
}

float Noise(in vec3 x)
{
    vec3 p = floor(x.xzy);
    vec3 f = fract(x.xzy);
    vec3 f2 = f * f; 
    f = f * f2 * (10.0f - 15.0f * f + 6.0f * f2);
    vec2 uv = (p.xy + vec2(37.0f, 17.0f) * p.z) + f.xy;
    vec2 rg = texture(iChannel0, (uv + 0.5f) / 256.0f).rg;
    return mix(rg.y, rg.x, f.z) - 0.5f;
}

float Trace(vec3 ro, vec3 rd)
{
    float t = 0.0;
    float dist = 1.0;
    for (int i = 0; i < traceDepth; i++)
    {
        if (abs(dist) < epsilon || t > drawDistance || t < 0.0)
            continue;
        dist = Isosurface(ro + rd * t);
        t = t + dist;
    }
    
    // reduce edge sparkles, caused by reflections on failed positions
    if ( dist > epsilon )
        return drawDistance+1.0;
    return t;//vec4(ro+rd*t,dist);
}

vec3 SubsurfaceTrace( vec3 ro, vec3 rd )
{
    vec3 density = pow(vec3(0.7f, 0.5f, 0.4f), vec3(0.4f));
    const float confidence = 0.01f;
    vec3 visibility = vec3(1.0f);
    
    float lastVal = Isosurface(ro);
    float soft = 0.0;
    for (int i = 1; i < 50; i++)
    {
        if (visibility.x < confidence)
            continue;
        float val = Isosurface(ro);
        vec3 softened = pow(density, vec3(smoothstep(soft, -soft, val)));                   // tweak this to create soft shadows, by expanding with each step (linearly)
        if ((val - soft) * lastVal < 0.0)
        {
            float transition = -min(val - soft, lastVal) / abs(val - soft - lastVal);       // approximate position of the surface
            visibility *= pow(softened,vec3(transition));
        }
        else if (val - soft < 0.0)
            visibility *= softened;

        soft += 0.1f;
        lastVal = val + soft;
        ro += rd * 0.4f;
    }
    return visibility;
}

// get normal
vec3 GetNormal( vec3 pos )
{
    const vec2 delta = vec2(normalPrecision, 0.0f);
    vec3 n = vec3(
            Isosurface(pos + delta.xyy) - Isosurface(pos - delta.xyy),                      // it's important this is centred on the pos, it fixes a lot of errors
            Isosurface(pos + delta.yxy) - Isosurface(pos - delta.yxy),
            Isosurface(pos + delta.yyx) - Isosurface(pos - delta.yyx)
        );
    return normalize(n);
}               

// camera function by TekF compute ray from camera parameters
vec3 GetRay(vec3 dir, float zoom, vec2 uv)
{
    uv = uv - 0.5f;
    uv.x *= resolution.x / resolution.y;    
    dir = zoom * normalize(dir);
    vec3 right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), dir));
    vec3 up = normalize(cross(dir, right));
    return dir + right * uv.x + up * uv.y;
}


void Humbug(inout vec4 result, inout vec3 ro, inout vec3 rd)
{
    if (result.a < 0.01f) return;

    float t = Trace(ro, rd);
    vec4 samplev = vec4(SkyColour(rd), 0);
    
    vec3 norm;
    if (t < drawDistance)
    {
        ro = ro + t * rd;        
        norm = GetNormal(ro);
        vec3 subsurface;
        if (toggleS)
            subsurface = vec3(dot(norm, lightDir));
        else
            subsurface = SubsurfaceTrace(ro + rd * 1.0f, lightDir);
        samplev = Shading(ro, norm, subsurface, rd);
    }
    result.rgb += samplev.rgb * result.a;
    result.a *= samplev.a;
    result.a = clamp(result.a, 0.0f, 1.0f);                                                 // without this, chrome shows black!
    rd = reflect(rd, norm);    
    ro += rd * shadowOffset;
}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 uv = fragCoord.xy / resolution.xy;

    vec3 camPos = CamPos;
    vec3 camLook = CamLook;

    vec2 camRot = vec2(time * 0.1f, 0.0f) + 0.5f * tau * (mouse.xy - resolution.xy * 0.5f) / resolution.x;
    camPos.xz = cos(camRot.x) * camPos.xz + sin(camRot.x) * camPos.zx * vec2(1.0f, -1.0f);
    camPos.yz = cos(camRot.y) * camPos.yz + sin(camRot.y) * camPos.zy * vec2(1.0f, -1.0f);
    
    if (Isosurface(camPos) <= 0.0)                                                                              // camera inside ground
    {    
        FragmentColor = vec4(0.0f);
        return;
    }

    vec3 ro = camPos;
    vec3 rd;
    rd = GetRay(camLook - camPos, 2.0f, uv);
    rd = normalize(rd);
    
    vec4 result = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Humbug(result, ro, rd);
    if ( !toggleR)
    {
        Humbug(result, ro, rd);
        Humbug(result, ro, rd);
    }    
    FragmentColor = result;
}
