#version 330 core

uniform float time;
uniform vec3 mouse;
uniform vec2 resolution;

out vec4 FragmentColor;



//==============================================================================================================================================================
// Helper functions
//==============================================================================================================================================================
vec3 mod289(vec3 x)
    { return x - floor(x * (1.0f / 289.0f)) * 289.0f; }
vec4 mod289(vec4 x)
    { return x - floor(x * (1.0f / 289.0f)) * 289.0f; }

float permute(float x) 
    { return mod((34.0f * x + 1.0f) * x, 289.0f); }
vec3 permute(vec3 x) 
    { return mod((34.0f * x + 1.0f) * x, 289.0f); }
vec4 permute(vec4 x) 
    { return mod289(((x * 34.0f) + 1.0f) * x); }
vec4 taylorInvSqrt(vec4 r) 
    { return 1.792843f - 0.853735f * r; }

//==============================================================================================================================================================
// 3D simplex noise
//==============================================================================================================================================================
float snoise(vec3 v)
{
    const vec2 C = vec2(1.0f / 6.0f, 1.0f/3.0f);
    const vec4 D = vec4(0.0f, 0.5f, 1.0f, 2.0f);

    // First corner
    vec3 i = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx) ;

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0f - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy;                                                      // 2.0f * C.x = 1.0f / 3.0f = C.y
    vec3 x3 = x0 - D.yyy;                                                           // -1.0f + 3.0f * C.x = -0.5f = -D.y

    // Permutations
    i = mod289(i);
    vec4 p = permute(permute(permute(i.z + vec4(0.0f, i1.z, i2.z, 1.0f)) + i.y + vec4(0.0f, i1.y, i2.y, 1.0f)) + i.x + vec4(0.0f, i1.x, i2.x, 1.0f));

    // Gradients: 7x7 points over a square, mapped onto an octahedron. The ring size 17 * 17 = 289 is close to a multiple of 49 (49 * 6 = 294)
    float n_ = 0.142857142857f;                                                     // 1.0f / 7.0f
    vec3 ns = n_ * D.wyz - D.xzx;
    vec4 j = p - 49.0f * floor(p * ns.z * ns.z);                                    // mod(p, 49)
    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0f * x_ );                                                // mod(j, N)
    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0f - abs(x) - abs(y);
    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);
    vec4 s0 = floor(b0) * 2.0f + 1.0f;
    vec4 s1 = floor(b1) * 2.0f + 1.0f;
    vec4 sh = -step(h, vec4(0.0f));
    vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy ;
    vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww ;
    vec3 p0 = vec3(a0.xy, h.x);
    vec3 p1 = vec3(a0.zw, h.y);
    vec3 p2 = vec3(a1.xy, h.z);
    vec3 p3 = vec3(a1.zw, h.w);

    // Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.51f - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0f);
    m = m * m;
    return 93.0f * dot(m * m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}


vec3 marble(vec3 position)
{
    const vec3 marble_color0 = vec3(0.1114f, 0.1431f, 0.0126f);
    const vec3 marble_color1 = vec3(0.9147f, 0.6987f, 0.3947f);

    float n = 0.0f;
    float frequency = 1.87f;
    float weight = 0.77f;
    float frequency_factor = 2.07f;
    float weight_factor = 0.47f;

    for (int i = 0; i < 4; ++i)
    {
        n += weight * snoise(vec3(position * frequency));
        frequency *= frequency_factor;
        weight *= weight_factor;
    };

    n = (1.0f - weight_factor) * abs(n);
    n = pow(n, 0.45f);

    return mix(marble_color0, marble_color1, n);
}

vec3 marble_color(vec3 position)
{
    const vec3 marble_color0 = vec3(0.125f, 0.065f, 0.057f);
    const vec3 marble_color1 = vec3(0.344f, 0.231f, 0.111f);
    const vec3 marble_color2 = vec3(0.444f, 0.331f, 0.081f);
    const vec3 marble_color3 = vec3(0.344f, 0.411f, 0.071f);

    float n = 0.0f;
    float frequency = 1.17f;
    float frequency_factor = 2.34f;
    float weight_factor = 0.67f;
    float weight = 1.0f;

    for (int i = 0; i < 6; ++i)
    {
        n += weight * snoise(vec3(position * frequency));
        frequency *= frequency_factor;
        weight *= weight_factor;
    };

    n = (1.0f - weight_factor) * abs(n);
    n = pow(n, 0.85f);

    vec3 color = (n >= 0.6666667f) ? mix(marble_color2, marble_color3, 3.0f * (n - 0.6666667f)) : 
                 (n >= 0.3333333f) ? mix(marble_color1, marble_color2, 3.0f * (n - 0.3333333f)) :
                                     mix(marble_color0, marble_color1, 3.0f * (n - 0.0000000f)) ;

    return 1.55f * color;
}

//==============================================================================================================================================================
// marble sphere
//==============================================================================================================================================================
#define INNER_COLOR vec3(0.2, 0.2, 0.6)
#define GLAS_COLOR vec3(0.5, 0.5, 0.9)

vec2 csqr(vec2 a)
{
    return vec2(a.x * a.x - a.y * a.y, 2.0f * a.x * a.y); 
}

mat2 rot(float a)
{
    return mat2( cos(a), sin(a),
                -sin(a), cos(a)); 
}

float map(in vec3 p) 
{
    float res = 0.0f;
    vec3 c = p;
    for (int i = 0; i < 7; ++i) 
    {
        p = 0.7f * abs(p) / dot(p, p) - 0.7f;
        p.yz += csqr(p.yz);
        p = p.zxy;
        res += exp(-19.0f * abs(dot(p, c)));
    }
    return res;
}

//---------------------------------------------------------
// intersect sphere: 
//   ro = rayOrigin, rd = rayDirection, 
//   sph.xyz = sphereCenter, sph.w = sphereRadius
// return intersection point distance
//---------------------------------------------------------
vec2 iSphere(in vec3 ro, in vec3 rd, in vec4 sph)
{
    vec3 oc = ro - sph.xyz;
    float b = dot(rd, oc);
    float c = dot(oc, oc);
    float h = b * b - c + sph.w * sph.w;
    if (h < 0.0f) return vec2(-1.0f);
    h = sqrt(h);
    return vec2(-b - h, -b + h);
}

vec3 raymarch(in vec3 ro, vec3 rd, vec2 tminmax)
{
    float t = tminmax.x;
    // float dt = 0.02;
    float dt = 0.02f + 0.01f * cos(time * 0.5f);                                    // animated
    vec3 col= vec3(0.0f);
    float c = 0.0f;
    for(int i = 0; i < 64; i++)
    {
        t += dt * exp(-2.0f * c);
        if (t > tminmax.y)
            break;
        c = 0.45f * map(ro + t * rd);               
        col = 0.98f * col + 0.08f * vec3(c * c, c, c * c * c);                      // green 
        // col = 0.99f * col + 0.08f * vec3(c * c * c, c * c, c);                   // blue
        col = 0.99f * col + 0.08f * vec3(c, c * c * c, c * c);                      // red
    }    
    return col;
}


void main()
{
    vec2 p = 1.25 * (-resolution + 2.0 * gl_FragCoord.xy) / resolution.xx;
    vec2 m = (mouse.xy / resolution - vec2(0.5f)) * 3.1415926f;

    // camera
    vec3 ro = vec3(4.0f);
    ro.yz *= rot(m.y);
    ro.xz *= rot(m.x);
    ro.xz *= rot(0.07f * time);                                                     // ROTATE scene
    vec3 ta = vec3(0.0f);
    vec3 ww = normalize(ta - ro);
    vec3 uu = normalize(cross(ww, vec3(0.0f, 1.0f, 0.0f)));
    vec3 vv = normalize(cross(uu, ww));
    vec3 rd = normalize(p.x * uu + p.y * vv + 4.0f * ww);
    vec2 tmm = iSphere(ro, rd, vec4(0.0f, 0.0f, 0.0f, 1.4f));                       // SPHERE radius = 1.4

    // raymarch
    vec3 col = raymarch(ro, rd, tmm);                                               // get sphere Color

    const float taa = 6.3f;                                                         // antialiasing limit
    float aa = 2.2f * (tmm.x - taa);
    if ((tmm.x < 0.0) || (tmm.x > taa))
    {
        vec3 bgCol = marble(3.0f * rd);                                             // background color
        col = (tmm.x > taa) ? mix(col, bgCol, aa) : bgCol;                          // antialasing
    }

    if (tmm.x > 0.0f)                                                               // add sphere reflection ?
    {           
        vec3 nor = reflect(rd, 0.5f * (ro + tmm.x * rd));                           // sphere normal
        float fre = pow(0.3f + clamp(dot(nor, rd), 0.0f, 1.0f), 3.0f) * 1.3f;
        col += GLAS_COLOR * fre * 0.1f * (1.0f - aa);                               // add some solid glas
        col += marble(3.0f * nor) * fre * 0.3f * (1.0f - aa);                       // add reflecting color
    }
    
    // shade
    col = 0.5f * log(1.0f + col);
    col = clamp(col, 0.0f, 1.0f);
    FragmentColor = vec4(col, 1.0f);
}