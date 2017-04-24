#version 330 core

uniform float time;
uniform vec3 mouse;
uniform vec2 resolution;

out vec4 FragmentColor;

#define MaxSteps 30
#define MinimumDistance 0.0009f
#define normalDistance 0.0002f

#define Iterations 7
#define PI 3.141592f
#define Scale 3.0f
#define FieldOfView 1.0f
#define Jitter 0.05f
#define FudgeFactor 0.7f
#define NonLinearPerspective 2.0f
#define DebugNonlinearPerspective false

#define Ambient 0.32184f
#define Diffuse 0.5f
#define LightDir vec3(1.0f)
#define LightColor vec3(1.0f, 1.0f, 0.858824f)
#define LightDir2 vec3(1.0f, -1.0f, 1.0f)
#define LightColor2 vec3(0.0f, 0.333333f, 1.0f)
#define Offset vec3(0.92858f, 0.92858f, 0.32858f)

vec2 rotate(vec2 v, float a)
{
    return vec2(cos(a) * v.x + sin(a) * v.y, -sin(a) * v.x + cos(a) * v.y);
}

// Two light sources. No specular 
vec3 getLight(in vec3 color, in vec3 normal, in vec3 dir)
{
    vec3 lightDir = normalize(LightDir);
    float diffuse = max(0.0,dot(-normal, lightDir)); // Lambertian
    
    vec3 lightDir2 = normalize(LightDir2);
    float diffuse2 = max(0.0,dot(-normal, lightDir2)); // Lambertian
    
    return
    (diffuse*Diffuse)*(LightColor*color) +
    (diffuse2*Diffuse)*(LightColor2*color);
}


// DE: Infinitely tiled Menger IFS.
// For more info on KIFS, see:
// http://www.fractalforums.com/3d-fractal-generation/kaleidoscopic-%28escape-time-ifs%29/
float DE(in vec3 z)
{
    // enable this to debug the non-linear perspective
    if (DebugNonlinearPerspective)
    {
        z = fract(z);
        float d = length(z.xy - vec2(0.5f));
        d = min(d, length(z.xz - vec2(0.5f)));
        d = min(d, length(z.yz - vec2(0.5f)));
        return d-0.01;
    }    
    z = abs(1.0f - mod(z, 2.0f));                                                       // Folding 'tiling' of 3D space;
    float d = 1000.0f;
    for (int n = 0; n < Iterations; n++)
    {
        z.xy = rotate(z.xy, 4.0f + 2.0f * cos(0.125f * time));
        z = abs(z);
        if (z.x < z.y) {z.xy = z.yx;}
        if (z.x < z.z) {z.xz = z.zx;}
        if (z.y < z.z) {z.yz = z.zy;}
        z = Scale * z - Offset * (Scale - 1.0f);
        if (z.z < -0.5f * Offset.z * (Scale - 1.0f)) z.z += Offset.z * (Scale - 1.0f);
        d = min(d, length(z) * pow(Scale, float(-n) - 1.0f));
    }
    return d - 0.001f;
}

vec3 getNormal(in vec3 pos)
{
    vec2 e = vec2(0.0,normalDistance);
    return normalize(vec3(DE(pos + e.yxx) - DE(pos - e.yxx),
                          DE(pos + e.xyx) - DE(pos - e.xyx),
                          DE(pos + e.xxy) - DE(pos - e.xxy)
                         )
                    );
}

vec3 getColor(vec3 normal, vec3 pos)
{
    return vec3(1.0);
}

float rand(vec2 co)
{
    return fract(cos(dot(co, vec2(4.898f, 7.23f))) * 23421.631f);
}

vec4 rayMarch(in vec3 from, in vec3 dir, in vec2 fragCoord)
{
    // Add some noise to prevent banding
    float totalDistance = Jitter * rand(fragCoord.xy + vec2(time));
    vec3 dir2 = dir;
    float distance;
    int steps = 0;
    vec3 pos;
    for (int i = 0; i < MaxSteps; i++)
    {
        // Non-linear perspective applied here.
        dir.zy = rotate(dir2.zy, totalDistance * cos(0.25f * time) * NonLinearPerspective);        
        pos = from + totalDistance * dir;
        distance = DE(pos)*FudgeFactor;
        totalDistance += distance;
        if (distance < MinimumDistance) break;
        steps = i;
    }
    
    // 'AO' is based on number of steps.
    // Try to smooth the count, to combat banding.
    float smoothStep = float(steps) + distance / MinimumDistance;
    float ao = 1.1f - smoothStep/float(MaxSteps);

    // Since our distance field is not signed,
    // backstep when calc'ing normal
    vec3 normal = getNormal(pos - dir * normalDistance * 3.0f);    
    vec3 color = getColor(normal, pos);
    vec3 light = getLight(color, normal, dir);
    color = (color * Ambient + light) * ao;
    return vec4(color, 1.0f);
}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec3 camPos = 0.5f * time * vec3(1.0f, 0.0f, 0.0f);
    vec3 target = camPos + vec3(1.0f, 0.0f * cos(time), 0.0f * sin(0.4f * time));
    vec3 camUp  = vec3(0.0f, 1.0f, 0.0f);
    vec3 camDir   = normalize(target - camPos);
    camUp = normalize(camUp - dot(camDir, camUp) * camDir);
    vec3 camRight = normalize(cross(camDir, camUp));
    vec2 coord = -1.0f + 2.0f * fragCoord.xy / resolution.xy;
    coord.x *= resolution.x / resolution.y;
    vec3 rayDir = normalize(camDir + (coord.x*camRight + coord.y*camUp) * FieldOfView);
    FragmentColor = rayMarch(camPos, rayDir, fragCoord );
}




