#version 330 core

uniform float time;
uniform vec3 mouse;
uniform vec2 resolution;

uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;

out vec4 FragmentColor;

float treeLine = 0.0;
float treeCol = 0.0;


vec3 sunLight  = normalize(vec3(0.4f, 0.4f, 0.48f));
vec3 sunColour = vec3(1.0f, 0.9f, 0.83f);
float specular = 0.0f;
vec3 cameraPos;
float ambient;
vec2 add = vec2(1.0f, 0.0f);

#define MOD2 vec2(3.07965f, 7.4235f)
#define MOD3 vec3(3.07965f, 7.1235f, 4.998784f)

// This peturbs the fractal positions for each iteration down...
// Helps make nice twisted landscapes...
const mat2 rotate2D = mat2(1.3623f, 1.7531f, -1.7131f, 1.4623f);

// Alternative rotation:-
// const mat2 rotate2D = mat2(1.2323, 1.999231, -1.999231, 1.22);

float Hash12(vec2 p)
{
    p = fract(p / MOD2);
    p += dot(p.xy, p.yx + 19.19f);
    return fract(p.x * p.y);
}

vec2 Hash22(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) / MOD3);
    p3 += dot(p3.zxy, p3.yxz + 19.19f);
    return fract(vec2(p3.x * p3.y, p3.z * p3.x));
}

float Noise(in vec2 x)
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f * f * (3.0f - 2.0f * f);
    return mix(mix(Hash12(p),          Hash12(p + add.xy), f.x),
               mix(Hash12(p + add.yx), Hash12(p + add.xx), f.x), f.y);
}

vec2 Noise2(in vec2 x)
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f * f * (3.0f - 2.0f * f);
    float n = p.x + p.y * 57.0;
    return mix(mix(Hash22(p),          Hash22(p + add.xy), f.x),
               mix(Hash22(p + add.yx), Hash22(p + add.xx), f.x), f.y);
}

float Trees(vec2 p)
{
    //return (texture(iChannel1, 0.04 * p).x * treeLine);
    return Noise(p * 13.0) * treeLine;
}


// Low def version for ray-marching through the height field...
float Terrain(in vec2 p)
{
    vec2 pos = p * 0.05f;
    float w = (Noise(pos * 0.25f) * 0.75f + 0.15f);
    w = 66.0f * w * w;
    vec2 dxy = vec2(0.0f, 0.0f);
    float f = 0.0f;
    for (int i = 0; i < 5; i++)
    {
        f += w * Noise(pos);
        w = -w * 0.4f;                                                                                                  // ...Flip negative and positive for variation
        pos = rotate2D * pos;
    }
    float ff = Noise(pos * 0.002f);    
    f += pow(abs(ff), 5.0f) * 275.0f - 5.0f;
    return f;
}

// Map to lower resolution for height field mapping for Scene function...
float Map(in vec3 p)
{
    float h = Terrain(p.xz);
    float ff = Noise(p.xz * 0.3f) + Noise(p.xz * 3.3f) * 0.5f;
    treeLine = smoothstep(ff, 0.0f + ff * 2.0f, h) * smoothstep(1.0f + ff * 3.0f, 0.4f + ff, h);
    treeCol = Trees(p.xz);
    h += treeCol;
    return p.y - h;
}

// High def version only used for grabbing normal information.
float Terrain2( in vec2 p)
{
    vec2 pos = p * 0.05f;                                                                                               // There's some real magic numbers in here! 
    float w = (Noise(pos * 0.25f) * 0.75f + 0.15f);                                                                     // The Noise calls add large mountain ranges for more variation over distances...
    w = 66.0f * w * w;
    vec2 dxy = vec2(0.0f, 0.0f);
    float f = 0.0f;
    for (int i = 0; i < 5; i++)
    {
        f += w * Noise(pos);
        w = -w * 0.4f;                                                                                                  // ...Flip negative and positive for varition       
        pos = rotate2D * pos;
    }
    float ff = Noise(pos * 0.002f);
    f += pow(abs(ff), 5.0f) * 275.0f - 5.0f;
    treeCol = Trees(p);
    f += treeCol;
    if (treeCol > 0.0f) return f;
    for (int i = 0; i < 6; i++)                                                                                         // That's the last of the low resolution, now go down further for the Normal data...
    {
        f += w * Noise(pos);
        w = -w * 0.4f;
        pos = rotate2D * pos;
    }
    return f;
}

float FractalNoise(in vec2 xy)
{
    float w = 0.7f;
    float f = 0.0f;
    for (int i = 0; i < 4; i++)
    {
        f += Noise(xy) * w;
        w *= 0.5f;
        xy *= 2.7f;
    }
    return f;
}

// Simply Perlin clouds that fade to the horizon... 200 units above the ground
vec3 GetClouds(in vec3 sky, in vec3 rd)
{
    if (rd.y < 0.01f) return sky;
    float v = (200.0f - cameraPos.y) / rd.y;
    rd.xz *= v;
    rd.xz += cameraPos.xz;
    rd.xz *= 0.010f;
    float f = (FractalNoise(rd.xz) - 0.55f) * 5.0f;
    sky = mix(sky, vec3(0.55f, 0.55f, 0.52f), clamp(f * rd.y - 0.1f, 0.0f, 1.0f));                                      // Uses the ray's y component for horizon fade of fixed colour clouds...
    return sky;
}

vec3 GetSky(in vec3 rd)                                                                                                 // Grab all sky information for a given ray from camera
{
    float sunAmount = max(dot(rd, sunLight), 0.0f);
    float v = pow(1.0f - max(rd.y, 0.0f), 5.0f) * 0.5f;
    vec3  sky = vec3(v * sunColour.x * 0.4f + 0.18f, v * sunColour.y * 0.4f + 0.22f, v * sunColour.z * 0.4f + 0.4f);
    sky = sky + sunColour * pow(sunAmount, 6.5f) * 0.32f;                                                               // Wide glare effect...
    sky = sky + sunColour * min(pow(sunAmount, 1150.0f), 0.3f) * 0.65f;                                                 // Actual sun...
    return sky;
}

vec3 ApplyFog(in vec3 rgb, in float dis, in vec3 dir)                                                                   // Merge mountains into the sky background for correct disappearance...
{
    float fogAmount = exp(-dis * 0.00005f);
    return mix(GetSky(dir), rgb, fogAmount);
}

void DoLighting(inout vec3 mat, in vec3 pos, in vec3 normal, in vec3 eyeDir, in float dis)                              // Calculate sun light...
{
    float h = dot(sunLight, normal);
    float c = max(h, 0.0f) + ambient;
    mat = mat * sunColour * c ;
    if (h > 0.0f)                                                                                                       // Specular...
    {
        vec3 R = reflect(sunLight, normal);
        float specAmount = pow(max(dot(R, normalize(eyeDir)), 0.0f), 3.0f) * specular;
        mat = mix(mat, sunColour, specAmount);
    }
}


vec3 TerrainColour(vec3 pos, vec3 normal, float dis)                                                                    // Hack the height, position, and normal data to create the coloured landscape
{
    vec3 mat;
    specular = 0.0f;
    ambient = 0.1f;
    vec3 dir = normalize(pos-cameraPos);
    vec3 matPos = pos * 2.0f;                                                                                           // ... I had change scale halfway though, this lazy multiply allow me to keep the graphic scales I had

    float disSqrd = dis * dis;                                                                                          // Squaring it gives better distance scales.

    float f = clamp(Noise(matPos.xz * 0.05f), 0.0f, 1.0f);//*10.8;
    f += Noise(matPos.xz * 0.1f + normal.yz * 1.08f) * 0.85f;
    f *= 0.55f;
    vec3 m = mix(vec3(0.63f * f + 0.2f, 0.7f * f + 0.1f, 0.7f * f + 0.1f), vec3(f * 0.43f + 0.1f, f * 0.3f + 0.2f, f * 0.35f + 0.1f), f * 0.65f);
    mat = m * vec3(f * m.x + 0.36f, f * m.y + 0.30f, f * m.z + 0.28f);
    
    if (normal.y < 0.5f)                                                                                                // Should have used smoothstep to add colours, but left it using 'if' for sanity...
    {
        float v = normal.y;
        float c = (0.5f - normal.y) * 4.0f;
        c = clamp(c * c, 0.1f, 1.0f);
        f = Noise(vec2(matPos.x * 0.09f, matPos.z * 0.095f + matPos.yy * 0.15f));
        f += Noise(vec2(matPos.x * 2.233f, matPos.z * 2.23f)) * 0.5f;
        mat = mix(mat, vec3(0.4f * f), c);
        specular += 0.1f;
    }
    
    if (matPos.y < 45.35f && normal.y > 0.65f)                                                                          // Grass. Use the normal to decide when to plonk grass down...
    {

        m = vec3(Noise(matPos.xz * 0.023f) * 0.5f + 0.15f, Noise(matPos.xz * 0.03f) * 0.6f + 0.25f, 0.0f);
        m *= (normal.y - 0.65f) * 0.6f;
        mat = mix(mat, m, clamp((normal.y - 0.65f) * 1.3f * (45.35f - matPos.y) * 0.1f, 0.0f, 1.0f));
    }

    if (treeCol > 0.0)
    {
        mat = vec3(0.02f + Noise(matPos.xz * 5.0f) * 0.03f, 0.05f, 0.0f);
        normal = normalize(normal+vec3(Noise(matPos.xz * 33.0f) * 1.0f - 0.5f, 0.0f, Noise(matPos.xz * 33.0f) * 1.0f - 0.5f));
        specular = .0;
    }
    
    
    if ((matPos.y > 80.0f) && (normal.y > .42))                                                                              // Snow topped mountains...
    {
        float snow = clamp((matPos.y - 80.0 - Noise(matPos.xz * .1)*28.0) * 0.035, 0.0, 1.0);
        mat = mix(mat, vec3(.7,.7,.8), snow);
        specular += snow;
        ambient+=snow *.3;
    }
    // Beach effect...
    if (matPos.y < 1.45)
    {
        if (normal.y > .4)
        {
            f = Noise(matPos.xz * .084)*1.5;
            f = clamp((1.45-f-matPos.y) * 1.34, 0.0, .67);
            float t = (normal.y-.4);
            t = (t*t);
            mat = mix(mat, vec3(.09+t, .07+t, .03+t), f);
        }
        // Cheap under water darkening...it's wet after all...
        if (matPos.y < 0.0)
        {
            mat *= .5;
        }
    }

    DoLighting(mat, pos, normal,dir, disSqrd);
    
    // Do the water...
    if (matPos.y < 0.0)
    {
        // Pull back along the ray direction to get water surface point at y = 0.0 ...
        float time00 = (time)*.03;
        vec3 watPos = matPos;
        watPos += -dir * (watPos.y/dir.y);
        // Make some dodgy waves...
        float tx = cos(watPos.x*.052) *4.5;
        float tz = sin(watPos.z*.072) *4.5;
        vec2 co = Noise2(vec2(watPos.x*4.7+1.3+tz, watPos.z*4.69+time00*35.0-tx));
        co += Noise2(vec2(watPos.z*8.6+time00*13.0-tx, watPos.x*8.712+tz))*.4;
        vec3 nor = normalize(vec3(co.x, 20.0, co.y));
        nor = normalize(reflect(dir, nor));//normalize((-2.0*(dot(dir, nor))*nor)+dir);
        // Mix it in at depth transparancy to give beach cues..
        tx = watPos.y-matPos.y;
        mat = mix(mat, GetClouds(GetSky(nor)*vec3(.5,.7,1.0), nor)*.1+vec3(.0,.06,.05), clamp((tx)*.4, .6, 1.));
        // Add some extra water glint...
        mat += vec3(.1)*clamp(1.-pow(tx+.5, 3.)*texture(iChannel3, watPos.xz*.1, -2.).x, 0.,1.0);
        float sunAmount = max( dot(nor, sunLight), 0.0 );
        mat = mat + sunColour * pow(sunAmount, 228.5)*.6;
        vec3 temp = (watPos-cameraPos*2.)*.5;
        disSqrd = dot(temp, temp);
    }
    mat = ApplyFog(mat, disSqrd, dir);
    return mat;
}

float BinarySubdivision(in vec3 rO, in vec3 rD, vec2 t)
{
    // Home in on the surface by dividing by two and split...
    float halfwayT;
    for (int n = 0; n < 4; n++)
    {
        halfwayT = (t.x + t.y) * 0.5f;
        vec3 p = rO + halfwayT*rD;
        if (Map(p) < 0.5)
            t.x = halfwayT;
        else
            t.y = halfwayT;
    }
    return t.x;
}

bool Scene(in vec3 rO, in vec3 rD, out float resT, in vec2 fragCoord )
{
    float t = 1.2f + Hash12(fragCoord.xy);
    float oldT = 0.0f;
    float delta = 0.0f;
    bool fin = false;
    bool res = false;
    vec2 distances;
    for(int j = 0; j < 150; j++)
    {
        if (fin || t > 240.0f) break;
        vec3 p = rO + t * rD;
        //if (t > 240.0 || p.y > 195.0) break;
        float h = Map(p); // ...Get this positions height mapping.
        // Are we inside, and close enough to fudge a hit?...
        if(h < 0.5f)
        {
            fin = true;
            distances = vec2(t, oldT);
            break;
        }
        // Delta ray advance - a fudge between the height returned
        // and the distance already travelled.
        // It's a really fiddly compromise between speed and accuracy
        // Too large a step and the tops of ridges get missed.
        delta = max(0.01f, 0.3f * h) + 0.0065f * t;
        oldT = t;
        t += delta;
    }
    if (fin) resT = BinarySubdivision(rO, rD, distances);
    return fin;
}

//--------------------------------------------------------------------------
vec3 CameraPath(float t)
{
    float m = 1.0f + (mouse.x / resolution.x) * 300.0f;
    t = (time * 1.5f + m + 657.0f) * 0.006f + t;
    vec2 p = 476.0f * vec2(sin(3.5f * t), cos(1.5f * t));
    return vec3(335.0f - p.x, 0.6f, 308.0f + p.y);
}

//--------------------------------------------------------------------------
// Some would say, most of the magic is done in post! :D
vec3 PostEffects(vec3 rgb, vec2 uv)
{
    //#define CONTRAST 1.1
    //#define SATURATION 1.12
    //#define BRIGHTNESS 1.3
    //rgb = pow(abs(rgb), vec3(0.45));
    //rgb = mix(vec3(.5), mix(vec3(dot(vec3(.2125, .7154, .0721), rgb*BRIGHTNESS)), rgb*BRIGHTNESS, SATURATION), CONTRAST);
    rgb = (1.0 - exp(-rgb * 6.0)) * 1.0024;
    //rgb = clamp(rgb+hash12(fragCoord.xy*rgb.r)*0.1, 0.0, 1.0);
    return rgb;
}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 xy = -1.0f + 2.0f * fragCoord.xy / resolution.xy;
    vec2 uv = xy * vec2(resolution.x / resolution.y, 1.0f);
    vec3 camTar;

    // Use several forward heights, of decreasing influence with distance from the camera.
    float h = 0.0f;
    float f = 1.0f;
    for (int i = 0; i < 7; i++)
    {
        h += Terrain(CameraPath((0.6 - f) * 0.008f).xz) * f;
        f -= 0.1f;
    }
    cameraPos.xz = CameraPath(0.0f).xz;
    camTar.xz = CameraPath(0.005f).xz;
    camTar.y = cameraPos.y = max((h * 0.25f) + 3.5f, 1.0f);
    
    float roll = 0.15f * sin(time * 0.2f);
    vec3 cw = normalize(camTar - cameraPos);
    vec3 cp = vec3(sin(roll), cos(roll), 0.0f);
    vec3 cu = normalize(cross(cw, cp));
    vec3 cv = normalize(cross(cu, cw));
    vec3 rd = normalize(uv.x * cu + uv.y * cv + 1.5f * cw);

    vec3 col;
    float distance;
    if(!Scene(cameraPos, rd, distance, fragCoord))
    {
        // Missed scene, now just get the sky value...
        col = GetSky(rd);
        col = GetClouds(col, rd);
    }
    else
    {
        // Get world coordinate of landscape...
        // Get normal from sampling the high definition height map. Use the distance to sample larger gaps to help stop aliasing...
        vec3 pos = cameraPos + distance * rd;
        float p = min(0.3f, 0.0005f + 0.00005f * distance * distance);
        vec3 nor = vec3(0.0f, Terrain2(pos.xz), 0.0f);
        vec3 v2 = nor - vec3(p, Terrain2(pos.xz + vec2(p, 0.0f)), 0.0f);
        vec3 v3 = nor - vec3(0.0f, Terrain2(pos.xz+vec2(0.0f, -p)), -p);
        nor = cross(v2, v3);
        nor = normalize(nor);
        // Get the colour using all available data...
        col = TerrainColour(pos, nor, distance);
    }

    col = PostEffects(col, uv);    
    FragmentColor = vec4(col, 1.0f);
}