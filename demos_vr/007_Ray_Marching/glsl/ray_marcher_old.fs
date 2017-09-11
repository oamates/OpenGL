#version 330 core

in vec2 uv;
in vec3 view;
flat in vec3 position;

uniform mat4 camera_matrix;
uniform float time;
uniform sampler2D value_texture;

out vec4 FragmentColor;

//==============================================================================================================================================================
// Hermite interpolants of degree 3, 5, and 7
//==============================================================================================================================================================
vec2 hermite3(vec2 x)
    { return x * x * (3.0 - 2.0 * x); }

vec2 hermite5(vec2 x)
    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

vec2 hermite7(vec2 x)
{
    vec2 sqr = x * x;
    return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x));
}

vec3 hermite3(vec3 x)
    { return x * x * (3.0 - 2.0 * x); }

vec3 hermite5(vec3 x)
    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

vec3 hermite7(vec3 x)
{
    vec3 sqr = x * x;
    return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x));
}

//==============================================================================================================================================================
// Minkowski norm functions
//==============================================================================================================================================================
float lengthMinkowski(vec2 q, float d)
{
    vec2 qq = pow(abs(q), vec2(0.5f * d));
    return pow(dot(qq, qq), 1.0f / d);
}

float lengthMinkowski(vec3 q, float d)
{
    vec3 qq = pow(abs(q), vec3(0.5f * d));
    return pow(dot(qq, qq), 1.0f / d);
}

//==============================================================================================================================================================
// Texture-base 2d and 3d value noise implementation
//==============================================================================================================================================================
const float TEXEL_SIZE = 1.0f / 256.0f;
const float HALF_TEXEL = 1.0f / 512.0f;

float vnoise(in vec2 x)
{
    vec2 p = floor(x);
    vec2 f = x - p;
    vec2 uv = p + hermite5(f);
    return texture(value_texture, TEXEL_SIZE * uv + HALF_TEXEL).x;
}

float vnoise(vec3 x)
{
    vec3 p = floor(x);
    vec3 f = x - p;
    f = hermite5(f);
    vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg = texture(value_texture, TEXEL_SIZE * uv + HALF_TEXEL).rg;
    return mix(rg.g, rg.r, f.z);
}

vec3 vnoise3(vec3 x)
{
    vec3 p = floor(x);
    vec3 f = x - p;
    f = hermite5(f);

    vec2 uv1 = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg1 = texture(value_texture, TEXEL_SIZE * uv1 + HALF_TEXEL).rg;

    vec2 uv2 = (vec2(49.0,  11.0) + p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg2 = texture(value_texture, TEXEL_SIZE * uv2 + HALF_TEXEL).rg;

    vec2 uv3 = (vec2(47.0, 113.0) + p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg3 = texture(value_texture, TEXEL_SIZE * uv3 + HALF_TEXEL).rg;



    return vec3(
            mix(rg1.g, rg1.r, f.z),
            mix(rg2.g, rg2.r, f.z),
            mix(rg3.g, rg3.r, f.z)
            );

/*
    vec3 p = floor(x);
    vec3 f = x - p;
    f = hermite5(f);
    vec2 uv1 = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 uv2 = uv1 + vec2(49.0,  11.0) * p.z;
    vec2 uv3 = uv1 + vec2(47.0, 113.0) * p.z;
    vec2 _1mfz_fz = vec2(f.z, 1.0 - f.z);

    mat2x3 m = mat2x3(texture(value_texture, TEXEL_SIZE * uv1 + HALF_TEXEL).rg,
                      texture(value_texture, TEXEL_SIZE * uv2 + HALF_TEXEL).rg,
                      texture(value_texture, TEXEL_SIZE * uv3 + HALF_TEXEL).rg);
    return m * _1mfz_fz;    
    */
}

//==============================================================================================================================================================
// Procedural cellular and simplex noise
//==============================================================================================================================================================

vec2 mod289(vec2 x)
    { return x - floor(x * (1.0f / 289.0f)) * 289.0f; }
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


float smootherstep(float edge0, float edge1, float x)
{
    x = clamp((x - edge0)/(edge1 - edge0), 0.0, 1.0);
    return x*x*x*(x*(x*6 - 15) + 10);
}

vec2 cellular(vec3 P) 
{
    const float K = 0.142857142857f; // 1/7
    const float Ko = 0.428571428571f; // 1/2-K/2
    const float K2 = 0.020408163265306f; // 1/(7*7)
    const float Kz = 0.166666666667f; // 1/6
    const float Kzo = 0.416666666667f; // 1/2-1/6*2
    const float jitter = 1.0f; // smaller jitter gives more regular pattern

    vec3 Pi = mod(floor(P), 289.0);
    vec3 Pf = fract(P) - 0.5;

    vec3 Pfx = Pf.x + vec3(1.0, 0.0, -1.0);
    vec3 Pfy = Pf.y + vec3(1.0, 0.0, -1.0);
    vec3 Pfz = Pf.z + vec3(1.0, 0.0, -1.0);

    vec3 p = permute(Pi.x + vec3(-1.0, 0.0, 1.0));
    vec3 p1 = permute(p + Pi.y - 1.0);
    vec3 p2 = permute(p + Pi.y);
    vec3 p3 = permute(p + Pi.y + 1.0);

    vec3 p11 = permute(p1 + Pi.z - 1.0);
    vec3 p12 = permute(p1 + Pi.z);
    vec3 p13 = permute(p1 + Pi.z + 1.0);

    vec3 p21 = permute(p2 + Pi.z - 1.0);
    vec3 p22 = permute(p2 + Pi.z);
    vec3 p23 = permute(p2 + Pi.z + 1.0);

    vec3 p31 = permute(p3 + Pi.z - 1.0);
    vec3 p32 = permute(p3 + Pi.z);
    vec3 p33 = permute(p3 + Pi.z + 1.0);

    vec3 ox11 = fract(p11*K) - Ko;
    vec3 oy11 = mod(floor(p11*K), 7.0)*K - Ko;
    vec3 oz11 = floor(p11*K2)*Kz - Kzo; // p11 < 289 guaranteed

    vec3 ox12 = fract(p12*K) - Ko;
    vec3 oy12 = mod(floor(p12*K), 7.0)*K - Ko;
    vec3 oz12 = floor(p12*K2)*Kz - Kzo;

    vec3 ox13 = fract(p13*K) - Ko;
    vec3 oy13 = mod(floor(p13*K), 7.0)*K - Ko;
    vec3 oz13 = floor(p13*K2)*Kz - Kzo;

    vec3 ox21 = fract(p21*K) - Ko;
    vec3 oy21 = mod(floor(p21*K), 7.0)*K - Ko;
    vec3 oz21 = floor(p21*K2)*Kz - Kzo;

    vec3 ox22 = fract(p22*K) - Ko;
    vec3 oy22 = mod(floor(p22*K), 7.0)*K - Ko;
    vec3 oz22 = floor(p22*K2)*Kz - Kzo;

    vec3 ox23 = fract(p23*K) - Ko;
    vec3 oy23 = mod(floor(p23*K), 7.0)*K - Ko;
    vec3 oz23 = floor(p23*K2)*Kz - Kzo;

    vec3 ox31 = fract(p31*K) - Ko;
    vec3 oy31 = mod(floor(p31*K), 7.0)*K - Ko;
    vec3 oz31 = floor(p31*K2)*Kz - Kzo;

    vec3 ox32 = fract(p32*K) - Ko;
    vec3 oy32 = mod(floor(p32*K), 7.0)*K - Ko;
    vec3 oz32 = floor(p32*K2)*Kz - Kzo;

    vec3 ox33 = fract(p33*K) - Ko;
    vec3 oy33 = mod(floor(p33*K), 7.0)*K - Ko;
    vec3 oz33 = floor(p33*K2)*Kz - Kzo;

    vec3 dx11 = Pfx + jitter*ox11;
    vec3 dy11 = Pfy.x + jitter*oy11;
    vec3 dz11 = Pfz.x + jitter*oz11;

    vec3 dx12 = Pfx + jitter*ox12;
    vec3 dy12 = Pfy.x + jitter*oy12;
    vec3 dz12 = Pfz.y + jitter*oz12;

    vec3 dx13 = Pfx + jitter*ox13;
    vec3 dy13 = Pfy.x + jitter*oy13;
    vec3 dz13 = Pfz.z + jitter*oz13;

    vec3 dx21 = Pfx + jitter*ox21;
    vec3 dy21 = Pfy.y + jitter*oy21;
    vec3 dz21 = Pfz.x + jitter*oz21;

    vec3 dx22 = Pfx + jitter*ox22;
    vec3 dy22 = Pfy.y + jitter*oy22;
    vec3 dz22 = Pfz.y + jitter*oz22;

    vec3 dx23 = Pfx + jitter*ox23;
    vec3 dy23 = Pfy.y + jitter*oy23;
    vec3 dz23 = Pfz.z + jitter*oz23;

    vec3 dx31 = Pfx + jitter*ox31;
    vec3 dy31 = Pfy.z + jitter*oy31;
    vec3 dz31 = Pfz.x + jitter*oz31;

    vec3 dx32 = Pfx + jitter*ox32;
    vec3 dy32 = Pfy.z + jitter*oy32;
    vec3 dz32 = Pfz.y + jitter*oz32;

    vec3 dx33 = Pfx + jitter*ox33;
    vec3 dy33 = Pfy.z + jitter*oy33;
    vec3 dz33 = Pfz.z + jitter*oz33;

    vec3 d11 = dx11 * dx11 + dy11 * dy11 + dz11 * dz11;
    vec3 d12 = dx12 * dx12 + dy12 * dy12 + dz12 * dz12;
    vec3 d13 = dx13 * dx13 + dy13 * dy13 + dz13 * dz13;
    vec3 d21 = dx21 * dx21 + dy21 * dy21 + dz21 * dz21;
    vec3 d22 = dx22 * dx22 + dy22 * dy22 + dz22 * dz22;
    vec3 d23 = dx23 * dx23 + dy23 * dy23 + dz23 * dz23;
    vec3 d31 = dx31 * dx31 + dy31 * dy31 + dz31 * dz31;
    vec3 d32 = dx32 * dx32 + dy32 * dy32 + dz32 * dz32;
    vec3 d33 = dx33 * dx33 + dy33 * dy33 + dz33 * dz33;

    // Sort out the two smallest distances (F1, F2)
    vec3 d1a = min(d11, d12);
    d12 = max(d11, d12);
    d11 = min(d1a, d13); // Smallest now not in d12 or d13
    d13 = max(d1a, d13);
    d12 = min(d12, d13); // 2nd smallest now not in d13
    vec3 d2a = min(d21, d22);
    d22 = max(d21, d22);
    d21 = min(d2a, d23); // Smallest now not in d22 or d23
    d23 = max(d2a, d23);
    d22 = min(d22, d23); // 2nd smallest now not in d23
    vec3 d3a = min(d31, d32);
    d32 = max(d31, d32);
    d31 = min(d3a, d33); // Smallest now not in d32 or d33
    d33 = max(d3a, d33);
    d32 = min(d32, d33); // 2nd smallest now not in d33
    vec3 da = min(d11, d21);
    d21 = max(d11, d21);
    d11 = min(da, d31); // Smallest now in d11
    d31 = max(da, d31); // 2nd smallest now not in d31
    d11.xy = (d11.x < d11.y) ? d11.xy : d11.yx;
    d11.xz = (d11.x < d11.z) ? d11.xz : d11.zx; // d11.x now smallest
    d12 = min(d12, d21); // 2nd smallest now not in d21
    d12 = min(d12, d22); // nor in d22
    d12 = min(d12, d31); // nor in d31
    d12 = min(d12, d32); // nor in d32
    d11.yz = min(d11.yz,d12.xy); // nor in d12.yz
    d11.y = min(d11.y,d12.z); // Only two more to go
    d11.y = min(d11.y,d11.z); // Done! (Phew!)
    return sqrt(d11.xy); // F1, F2
}

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
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy;                                              // 2.0f * C.x = 1.0f / 3.0f = C.y
    vec3 x3 = x0 - D.yyy;                                                   // -1.0f + 3.0f * C.x = -0.5f = -D.y

    // Permutations
    i = mod289(i);
    vec4 p = permute( permute(permute(i.z + vec4(0.0f, i1.z, i2.z, 1.0f)) + i.y + vec4(0.0f, i1.y, i2.y, 1.0f)) + i.x + vec4(0.0f, i1.x, i2.x, 1.0f));

    // Gradients: 7x7 points over a square, mapped onto an octahedron. The ring size 17 * 17 = 289 is close to a multiple of 49 (49 * 6 = 294)
    float n_ = 0.142857142857;                                              // 1.0f / 7.0f
    vec3 ns = n_ * D.wyz - D.xzx;
    vec4 j = p - 49.0f * floor(p * ns.z * ns.z);                            // mod(p, 49)
    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0f * x_ );                                        // mod(j, N)
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



//=================================================================================================================================
// Example1 :: sphere intersection
//=================================================================================================================================

const vec4 spheres[8] = vec4[8] 
(
    vec4(-10.89f,  -3.39f,   8.64f, 1.71f),
    vec4(  7.12f,   5.67f,  -2.87f, 2.13f),
    vec4( -6.41f,   8.54f, -10.09f, 4.41f),
    vec4( -1.37f,  -9.43f,   2.15f, 0.78f),
    vec4( -4.56f, -11.43f,   3.40f, 1.12f),
    vec4( -8.12f,   7.31f,   1.07f, 3.22f),
    vec4(  9.22f,   4.78f,  -8.31f, 3.61f),
    vec4(-15.71f,   6.45f,   8.12f, 1.78f)
);

vec3 intersect(in vec3 position, in vec3 direction)
{
    float d = 10000.0f;
    vec3 v = vec3(0.0f);
    for (int i = 0; i < 8; ++i)
    {
        vec4 sphere = spheres[i];
        vec3 center = sphere.xyz;
        float r = sphere.w;

        vec3 ray_center = position - sphere.xyz;
        float p = dot(direction, ray_center);
        float q = dot(ray_center, ray_center);
        float h = p * p - q + r * r;
        if (h < 0.0f) continue; // does not intersect
        h = sqrt(h);

        float d1 = -p - h;
        float d2 = -p + h;

        if ((d1 > 0.0f) && (d1 < d))
        {
            d = d1;
            vec3 intersection_point = position + d1 * direction;
            float q = 0.5f * vnoise(3.67 * intersection_point) + 0.25f * vnoise(7.67 *  intersection_point) + 0.125f * vnoise(9.89 * intersection_point);
            v = pow(clamp(h, 0.0f, 1.0f), 0.5f) * q * abs(normalize(center));
        }
    }
    return v;
}

//=================================================================================================================================
// Example2 :: simplest distance field marching
//=================================================================================================================================

float distance(vec3 position)
{
    float d = 10000.0f;
    for (int i = 0; i < 4; ++i)
    {
        vec4 sphere = spheres[i];
        vec3 center = sphere.xyz;
        float r = sphere.w;

        float d0 = length(center - position) - r;
        d = min(d0, d);
    }
    return d;
}

vec3 raymarch(vec3 position, vec3 direction)
{
    const float epsilon = 0.001;
    const int maxSteps = 64;
    float t = 0.0f;
    for(int i = 0; i < maxSteps; ++i) 
    {
        float d = distance(position + direction * t);
        if(d < epsilon) 
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;
            float q = 0.5f * vnoise(3.67 * intersection_point) + 0.25f * vnoise(7.67 *  intersection_point) + 0.125f * vnoise(9.89 * intersection_point);
            vec3 v = vec3(q);
            return v;
        }
        t += d;
    }
    return vec3(0.0f);
}

//=================================================================================================================================
// Example3 :: infinite periodic pattern
//=================================================================================================================================

float distance3(vec3 position)
{
    position.xy = -vec2(8.0) + mod(position.xy, vec2(16.0));
    return length(position) - 7.75;
}

vec3 raymarch3(vec3 position, vec3 direction)
{
    const float epsilon = 0.001;
    const int maxSteps = 64;
    float t = 0.0f;
    for(int i = 0; i < maxSteps; ++i) 
    {
        float d = distance3(position + direction * t);
        if(d < epsilon) 
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;

            vec2 l = floor(intersection_point.xy / 16.0);
            vec3 ambient = vec3(vnoise(3.67 * l.xy), vnoise(3.67 * l.yx), vnoise(3.67 * l.yy));
            ambient = normalize(ambient);

            float q = 0.5f * vnoise(3.67 * intersection_point) + 0.25f * vnoise(7.67 *  intersection_point) + 0.125f * vnoise(9.89 * intersection_point);
            vec3 v = vec3(q) * ambient;
            return v;
        }
        t += d;
    }
    return vec3(0.0f);
}

//=================================================================================================================================
// Example4 :: cubes
//=================================================================================================================================

float sd_cube0(vec3 p, float size)
{
    p.xy = -vec2(8.0) + mod(p.xy, vec2(16.0));
    vec3 d = abs(p) - vec3(size);
    return min(max(d.x, max(d.y, d.z)), 0.0f) + length(max(d, 0.0f));
}

float distance4(vec3 position)
{
    position.xy = -vec2(8.0) + mod(position.xy, vec2(16.0));
    return length(max(abs(position) - 6.0f, 0.0f)) - 0.5f;                                  // subtraction smoothers out cube edges
}

vec3 raymarch4(vec3 position, vec3 direction)
{
    const float epsilon = 0.01;
    const int maxSteps = 64;
    float t = 0.0f;
    for(int i = 0; i < maxSteps; ++i) 
    {
        float d = sd_cube0(position + direction * t, 3.0f);
        if(d < epsilon) 
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;

            vec2 l = floor(intersection_point.xy / 16.0);
            vec3 ambient = vec3(vnoise(3.67 * l.xy), vnoise(3.67 * l.yx), vnoise(3.67 * l.yy));
            ambient = normalize(ambient);

            float q = 0.5f * vnoise(3.67 * intersection_point) + 0.25f * vnoise(7.67 *  intersection_point) + 0.125f * vnoise(9.89 * intersection_point);
            vec3 v = vec3(q) * ambient;
            return v;
        }
        t += d;
    }
    return vec3(0.0f);
}

//=================================================================================================================================
// Example5 :: union operation cube + sphere
//=================================================================================================================================

float distance5(vec3 position)
{
    const vec3 center = vec3(-11.0f, 1.0f, 2.0f);
    const float radius = 6.0f;
    float dist_to_cube = length(max(abs(position) - 6.0f, 0.0f)) - 0.5f;
    float dist_to_sphere = length(position - center) - 5.0f;
    return min(dist_to_cube, dist_to_sphere);
}

vec3 raymarch5(vec3 position, vec3 direction)
{
    const float epsilon = 0.01;
    const int maxSteps = 64;
    float t = 0.0f;
    for(int i = 0; i < maxSteps; ++i) 
    {
        float d = distance5(position + direction * t);
        if(d < epsilon) 
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;

            vec2 l = floor(intersection_point.xy / 16.0);
            vec3 ambient = vec3(vnoise(3.67 * l.xy), vnoise(3.67 * l.yx), vnoise(3.67 * l.yy));
            ambient = normalize(ambient);

            float q = 0.5f * vnoise(3.67 * intersection_point) + 0.25f * vnoise(7.67 *  intersection_point) + 0.125f * vnoise(9.89 * intersection_point);
            vec3 v = vec3(q) * ambient;
            return v;
        }
        t += d;
    }
    return vec3(0.0f);
}

//=================================================================================================================================
// Example6 :: torus
//=================================================================================================================================

float distance6(vec3 position)
{
    const vec2 radii = vec2(5.0, 2.25);
    vec2 q = vec2(length(position.xz) - radii.x, position.y);
    return length(q) - radii.y;
}

vec3 raymarch6(vec3 position, vec3 direction)
{
    const float epsilon = 0.01;
    const int maxSteps = 64;
    float t = 0.0f;
    for(int i = 0; i < maxSteps; ++i) 
    {
        float d = distance6(position + direction * t);
        if(d < epsilon) 
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;

            vec2 l = floor(intersection_point.xy / 16.0);
            vec3 ambient = vec3(vnoise(3.67 * l.xy), vnoise(3.67 * l.yx), vnoise(3.67 * l.yy));
            ambient = normalize(ambient);

            float q = 0.5f * vnoise(3.67 * intersection_point) + 0.25f * vnoise(7.67 *  intersection_point) + 0.125f * vnoise(9.89 * intersection_point);
            vec3 v = vec3(q) * ambient;
            return v;
        }
        t += d;
    }
    return vec3(0.0f);
}

//=================================================================================================================================
// Example7 :: torus + cube + sphere
//=================================================================================================================================

float distance7(vec3 position)
{
    const vec2 radii = vec2(5.0, 2.25);
    vec2 q = vec2(length(position.xz - vec2(6.0f, 11.37f)) - radii.x, position.y + 3.34f);
    float dist_to_torus = length(q) - radii.y;
    const vec3 center = vec3(-11.0f, 1.0f, 2.0f);
    const float radius = 6.0f;
    float dist_to_sphere = length(position - center) - 5.0f;
    float dist_to_cube = length(max(abs(position) - 6.0f, 0.0f)) - 0.5f;
    return min(min(dist_to_cube, dist_to_sphere), dist_to_torus);
}


vec3 raymarch7(vec3 position, vec3 direction)
{
    const float epsilon = 0.01;
    const int maxSteps = 64;
    float t = 0.0f;
    for(int i = 0; i < maxSteps; ++i) 
    {
        float d = distance7(position + direction * t);
        if(d < epsilon) 
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;

            vec3 ambient = vec3(
                0.5f * vnoise(1.67 * intersection_point) + 0.25f * vnoise(7.64 *  intersection_point) + 0.125f * vnoise(11.77 * intersection_point),
                0.5f * vnoise(3.47 * intersection_point) + 0.25f * vnoise(8.17 *  intersection_point) + 0.125f * vnoise(10.49 * intersection_point),
                0.5f * vnoise(2.78 * intersection_point) + 0.25f * vnoise(9.38 *  intersection_point) + 0.125f * vnoise(12.19 * intersection_point));
            return ambient;
        }
        t += d;
    }
    return vec3(0.0f);
}

//=================================================================================================================================
// Example8 :: Minkowski torus
//=================================================================================================================================

float distance8(vec3 position)
{
    float d = 4.0f;
    const vec2 radii = vec2(5.0, 2.25);
    vec2 q = vec2(lengthMinkowski(position.xz, d) - radii.x, position.y);
    return lengthMinkowski(q, d) - radii.y;
}

vec3 raymarch8(vec3 position, vec3 direction)
{
    const float epsilon = 0.01;
    const int maxSteps = 64;
    float t = 0.0f;
    for(int i = 0; i < maxSteps; ++i) 
    {
        float d = distance8(position + direction * t);
        if(d < epsilon) 
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;

            vec3 ambient = vec3(
                0.5f * vnoise(1.67 * intersection_point) + 0.25f * vnoise(7.64 *  intersection_point) + 0.125f * vnoise(11.77 * intersection_point),
                0.5f * vnoise(3.47 * intersection_point) + 0.25f * vnoise(8.17 *  intersection_point) + 0.125f * vnoise(10.49 * intersection_point),
                0.5f * vnoise(2.78 * intersection_point) + 0.25f * vnoise(9.38 *  intersection_point) + 0.125f * vnoise(12.19 * intersection_point));
            return ambient;
        }
        t += d;
    }
    return vec3(0.0f);
}

//=================================================================================================================================
// Example9 :: stick
//=================================================================================================================================

float distance9(vec3 position)
{
    const float radius = 2.5f;
    const vec3 A = vec3(-1.11f, 6.72f, 3.37f);
    const vec3 B = vec3( 5.43f, 2.63f, 0.74f);

    const vec3 AB = B - A;
    const float l = length(AB);
    const vec3 nAB = AB / l;

    vec3 AP = position - A;
    float dp = dot(AP, nAB);
    float q = clamp(dp / l, 0.0f, 1.0f);
    vec3 C = mix(A, B, q);
    return length(position - C) - radius;
}

vec3 raymarch9(vec3 position, vec3 direction)
{
    const float epsilon = 0.01;
    const int maxSteps = 64;
    float t = 0.0f;
    for(int i = 0; i < maxSteps; ++i) 
    {
        float d = distance9(position + direction * t);
        if(d < epsilon) 
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;

            vec3 ambient = vec3(
                0.5f * vnoise(1.67 * intersection_point) + 0.25f * vnoise(7.64 *  intersection_point) + 0.125f * vnoise(11.77 * intersection_point),
                0.5f * vnoise(3.47 * intersection_point) + 0.25f * vnoise(8.17 *  intersection_point) + 0.125f * vnoise(10.49 * intersection_point),
                0.5f * vnoise(2.78 * intersection_point) + 0.25f * vnoise(9.38 *  intersection_point) + 0.125f * vnoise(12.19 * intersection_point));
            return ambient;
        }
        t += d;
    }
    return vec3(0.0f);
}

//=================================================================================================================================
// Example10 :: deformed stick
//=================================================================================================================================

float distance10(vec3 position)
{
    const float radius = 2.5f;
    const vec3 A = vec3(-1.11f, 6.72f, 3.37f);
    const vec3 B = vec3( 5.43f, 2.63f, 0.74f);

    const vec3 AB = B - A;
    const float l = length(AB);
    const vec3 nAB = AB / l;

    vec3 AP = position - A;
    float dp = dot(AP, nAB);
    float q = clamp(dp / l, 0.0f, 1.0f);
    vec3 C = mix(A, B, q);
    float displacement = 0.5 * vnoise(1.67 * position) + 0.25f * vnoise(7.64 *  position) + 0.125f * vnoise(11.77 * position);
    return length(position - C) - radius + 0.5f * displacement;
}

vec3 raymarch10(vec3 position, vec3 direction)
{
    const float epsilon = 0.01;
    const int maxSteps = 64;
    float t = 0.0f;
    for(int i = 0; i < maxSteps; ++i) 
    {
        float d = distance10(position + direction * t);
        if(d < epsilon) 
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;

            vec3 ambient = vec3(
                0.5f * vnoise(1.67 * intersection_point) + 0.25f * vnoise(7.64 *  intersection_point) + 0.125f * vnoise(11.77 * intersection_point),
                0.5f * vnoise(3.47 * intersection_point) + 0.25f * vnoise(8.17 *  intersection_point) + 0.125f * vnoise(10.49 * intersection_point),
                0.5f * vnoise(2.78 * intersection_point) + 0.25f * vnoise(9.38 *  intersection_point) + 0.125f * vnoise(12.19 * intersection_point));
            return ambient;
        }
        t += d;
    }
    return vec3(0.0f);
}

//=================================================================================================================================
// Example11 :: raymarch number of steps
//=================================================================================================================================

float distance11(vec3 position)
{
    position.xy = -vec2(16.0) + mod(position.xy, vec2(32.0));

    const float radius = 2.5f;
    const vec3 A = vec3(-1.11f, 6.72f, 3.37f);
    const vec3 B = vec3( 5.43f, 2.63f, 0.74f);

    const vec3 AB = B - A;
    const float l = length(AB);
    const vec3 nAB = AB / l;

    vec3 AP = position - A;
    float dp = dot(AP, nAB);
    float q = clamp(dp / l, 0.0f, 1.0f);
    vec3 C = mix(A, B, q);
    float displacement = 0.5 * vnoise(1.67 * position) + 0.25f * vnoise(7.64 *  position) + 0.125f * vnoise(11.77 * position);
    return length(position - C) - radius + 0.5f * displacement;
}

vec3 raymarch11(vec3 position, vec3 direction)
{
    const float epsilon = 0.01;
    const int maxSteps = 64;
    float t = 0.0f;
    for(int i = 0; i < maxSteps; ++i) 
    {
        float d = distance11(position + direction * t);
        if(d < epsilon) 
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;
            return vec3(float(i), 0.0f, 0.0f) / 32.0;
        }
        t += d;
    }
    return vec3(2.0f, 0.0f, 0.0f);
}

//=================================================================================================================================
// Example12 :: raymarch enhancement
//=================================================================================================================================

float distance12(vec3 position)
{
    position.xy = -vec2(16.0) + mod(position.xy, vec2(32.0));

    const float radius = 2.5f;
    const vec3 A = vec3(-1.11f, 6.72f, 3.37f);
    const vec3 B = vec3( 5.43f, 2.63f, 0.74f);

    const vec3 AB = B - A;
    const float l = length(AB);
    const vec3 nAB = AB / l;




    vec3 AP = position - A;
    float dp = dot(AP, nAB);
    float q = clamp(dp / l, 0.0f, 1.0f);
    vec3 C = mix(A, B, q);
    float displacement = 0.5 * vnoise(0.87 * position) + 0.25f * vnoise(1.94 *  position);
    return length(position - C) - radius + 0.5f * displacement;
}

vec3 raymarch12(vec3 position, vec3 direction)
{
    const float epsilon = 0.01;
    const int maxSteps = 128;
    float t = 0.0f;
    int i = 0;
    while(i < maxSteps)
    {
        float d = distance12(position + 0.75 * direction * t);

        if(d < epsilon)
        {
            // Do something with p
            return vec3(0.0, 1.0, 0.0);
            vec3 intersection_point = position + t * direction;
            vec3 ambient = vec3(
                0.5f * vnoise(1.67 * intersection_point) + 0.25f * vnoise(7.64 *  intersection_point) + 0.125f * vnoise(11.77 * intersection_point),
                0.5f * vnoise(3.47 * intersection_point) + 0.25f * vnoise(8.17 *  intersection_point) + 0.125f * vnoise(10.49 * intersection_point),
                0.5f * vnoise(2.78 * intersection_point) + 0.25f * vnoise(9.38 *  intersection_point) + 0.125f * vnoise(12.19 * intersection_point)
            );
            return ambient;
//            vec3 intersection_point = position + t * direction;
//            break;
        }
        t += d;
        if(t > 500.0f)
            break;
        ++i;
    }
    return vec3(float(i), 0.0f, 0.0f) / maxSteps;
}

//=================================================================================================================================
// Example13 :: infinite lattice
//=================================================================================================================================
float sd_cross_lattice(vec3 p, float size, float period)
{
    vec3 q = p - period * round(p / period);
    vec3 aq = abs(q);
    vec3 m = max(aq, aq.yzx);
    float distance_to_cross = min(m.x, min(m.y, m.z)) - size;
    return distance_to_cross - 0.25;
}

vec3 raymarch13(vec3 position, vec3 direction)
{
    const float epsilon = 0.025f;
    const int maxSteps = 128;
    float t = 0.0f;
    int i = 0;
    while(i < maxSteps) 
    {
        float d = sd_cross_lattice(position + direction * t, 2.5f, 16.0f);
        if(d < epsilon)
        {
            vec3 intersection_point = position + t * direction;
            float q = exp(-0.025 * i);
            vec3 ambient = vec3(
                0.5f * vnoise(1.67 * intersection_point) + 0.25f * vnoise(7.64 *  intersection_point) + 0.125f * vnoise(11.77 * intersection_point),
                0.5f * vnoise(3.47 * intersection_point) + 0.25f * vnoise(8.17 *  intersection_point) + 0.125f * vnoise(10.49 * intersection_point),
                0.5f * vnoise(2.78 * intersection_point) + 0.25f * vnoise(9.38 *  intersection_point) + 0.125f * vnoise(12.19 * intersection_point)
            );
            return q * ambient;
        }
        t += d;
        if(t > 500.0f)
            break;
        ++i;
    }
    return vec3(0.0f);
}

//=================================================================================================================================
// Example14 :: intersection
//=================================================================================================================================
/*
float sd_cross(vec3 p, float size)
{
    vec3 ap = abs(p);
    vec3 m = max(ap, ap.yzx);
    float distance_to_cross = min(m.x, min(m.y, m.z)) - size;
    return distance_to_cross;
}

float sd_holecube(vec3 p, float hole_size, float cube_size)
{
    vec3 ap = abs(p);
    vec3 m = max(ap, ap.yzx);
    float distance_to_cross = min(m.x, min(m.y, m.z)) - hole_size;
    float distance_to_cube = max(ap.x, m.y) - cube_size;
    return max(distance_to_cube, -distance_to_cross);
}

float sd_cross_lattice(vec3 p, float size, float period)
{       
    vec3 q = p - period * round(p / period);
    vec3 aq = abs(q);
    vec3 m = max(aq, aq.yzx);
    float distance_to_cross = min(m.x, min(m.y, m.z)) - size;
    return distance_to_cross;
}
*/

float sd_holed_lattice(vec3 p)
{
    float size = 3.0f;
    float period = 27.0f;

    float m = sd_cross_lattice(p, size, period);
    float m1 = sd_cross_lattice(p, size / 3.0f , period / 3.0f );
    float m2 = sd_cross_lattice(p, size / 9.0f , period / 9.0f );
    float m3 = sd_cross_lattice(p, size / 27.0f, period / 27.0f);
    float m4 = sd_cross_lattice(p, size / 81.0f, period / 81.0f);

    m = max(m, -m1);
    m = max(m, -m2);
    m = max(m, -m3);
    m = max(m, -m4);
    return m;
}

float sd_serpinski_cube(vec3 p)
{
    float cube_size = 81.0f;
    float size = 9.0f;
    float period = 81.0f;

    vec3 ap = abs(p);
    float m = max(max(ap.x, ap.y), ap.z) - cube_size;

    float m0 = sd_cross_lattice(p, size, period);
    float m1 = sd_cross_lattice(p, size / 3.0f , period / 3.0f );
    float m2 = sd_cross_lattice(p, size / 9.0f , period / 9.0f );
    float m3 = sd_cross_lattice(p, size / 27.0f, period / 27.0f);

    m = max(m, -m0);
    m = max(m, -m1);
    m = max(m, -m2);
    m = max(m, -m3);

    return m;
}

vec3 grad(vec3 p)
{
    vec3 dp = vec3(0.075f, 0.0f, -0.075f);
    vec3 q = vec3(sd_serpinski_cube(p + dp.xyy) - sd_serpinski_cube(p + dp.zyy), 
                  sd_serpinski_cube(p + dp.yxy) - sd_serpinski_cube(p + dp.yzy), 
                  sd_serpinski_cube(p + dp.yyx) - sd_serpinski_cube(p + dp.yyz));
    return normalize(q);
}

vec3 raymarch14(vec3 position, vec3 direction)
{
    vec3 fog_color = vec3(0.651f, 0.0f, 0.11f);
    const float epsilon = 0.025f;
    const int maxSteps = 128;
    float t = 0.0f;
    int i = 0;
    while(i < maxSteps) 
    {
        float d = sd_serpinski_cube(position + direction * t);

        if(d < epsilon)
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;

            float q = exp(-0.045 * i);

            vec3 n = grad(intersection_point);

            const vec3 l = vec3(0.707, 0.707, 0.0);

            float a = 0.5 + 0.5 * dot(n, l);

            vec3 ambient = vec3(
                0.125f * vnoise(0.67 * intersection_point) + 0.0625f * vnoise(3.64 *  intersection_point) + 0.03125f * vnoise(6.77 * intersection_point),
                0.25f * vnoise(1.47 * intersection_point) + 0.125f * vnoise(3.17 *  intersection_point) + 0.0625f * vnoise(6.49 * intersection_point),
                0.5f * vnoise(1.78 * intersection_point) + 0.25f * vnoise(4.38 *  intersection_point) + 0.125f * vnoise(7.19 * intersection_point)
            );

            // specular term, Blinn - Phong lighting
            vec3 h = normalize(l - direction);
            float cos_alpha = max(dot(h, n), 0.0f);
            return mix(fog_color, a * ambient + pow(cos_alpha, 40.0) * vec3(1.0f), q);

        }
        t += d;
        if(t > 500.0f)
            break;
        ++i;
    }
    return fog_color;
}


//=================================================================================================================================
// Example15 :: smooth serpinski cube
//=================================================================================================================================
// exponential smooth min (k = 32);
float smin1(float a, float b, float k)
{
    float res = exp(-k*a) + exp(-k*b);
    return -log(res) / k;
}

    
// polynomial smooth min (k = 0.1);
float smin2(float a, float b, float k)
{
    float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
    return mix(b, a, h) - k * h * (1.0f - h);
}

// power smooth min (k = 8);
float smin3(float a, float b, float k)
{
    a = pow(a, k);
    b = pow(b, k);
    return pow((a * b) / (a + b), 1.0f / k);
}

float smin3(float a, float b, float c, float k)
{
    a = pow(a, k);
    b = pow(b, k);
    c = pow(c, k);
    return pow((a * b * c) / (a + b + c), 1.0f / k);
}

float sd_cross_lattice1(vec3 p, float size, float period)
{       
    vec3 q = p;
    q.xyz -= period * round(q.xyz / period);

    vec3 d = abs(q) - vec3(size);
    float d_xy = min(max(d.x, d.y), 0.0f) + length(max(d.xy, 0.0f));
    float d_yz = min(max(d.y, d.z), 0.0f) + length(max(d.yz, 0.0f));
    float d_zx = min(max(d.z, d.x), 0.0f) + length(max(d.zx, 0.0f));

    return min(d_xy, min(d_yz, d_zx)) - 2.75;
}

float sd_cross_lattice2(vec3 p, float size, float period, float smooth_scale)
{       
    vec3 q = p;
    q.xyz -= period * round(q.xyz / period);

    vec3 d = abs(q) - vec3(size);
    float d_xy = min(max(d.x, d.y), 0.0f) + length(max(d.xy, 0.0f));
    float d_yz = min(max(d.y, d.z), 0.0f) + length(max(d.yz, 0.0f));
    float d_zx = min(max(d.z, d.x), 0.0f) + length(max(d.zx, 0.0f));

    return smin2(d_xy, smin2(d_yz, d_zx, smooth_scale), smooth_scale) - 0.75 * (0.8 + 0.6 * cos(time));
}


float sd_serpinski_cube2(vec3 p)
{
    float cube_size = 128.0f;
    float size = 32.0f;
    float period = 128.0f;

    // signed distance function of the cube
    vec3 d = abs(p) - vec3(cube_size);
    float m = min(max(d.x, max(d.y, d.z)), 0.0f) + length(max(d, 0.0f));

    float m0 = sd_cross_lattice2(p, size,         period,         1.0f);
    float m1 = sd_cross_lattice2(p, size / 3.0f , period / 3.0f , 1.0f / 3.0f );
    float m2 = sd_cross_lattice2(p, size / 9.0f , period / 9.0f , 1.0f / 9.0f );
    float m3 = sd_cross_lattice2(p, size / 27.0f, period / 27.0f, 1.0f / 125.0f);

//    return m2;
    m = max(m, -m0);
    m = max(m, -m1);
    m = max(m, -m2);
    m = max(m, -m3);

    return m;
}

vec3 grad2(vec3 p)
{
    vec3 dp = vec3(0.075f, 0.0f, -0.075f);
    vec3 q = vec3(sd_serpinski_cube2(p + dp.xyy) - sd_serpinski_cube2(p + dp.zyy), 
                  sd_serpinski_cube2(p + dp.yxy) - sd_serpinski_cube2(p + dp.yzy), 
                  sd_serpinski_cube2(p + dp.yyx) - sd_serpinski_cube2(p + dp.yyz));
    return normalize(q);
}

vec3 raymarch15(vec3 position, vec3 direction)
{
    vec3 fog_color = vec3(0.651f, 0.0f, 0.11f);
    const float epsilon = 0.025f;
    const int maxSteps = 128;
    float t = 0.0f;
    int i = 0;
    while(i < maxSteps) 
    {
        float d = sd_serpinski_cube2(position + direction * t);

        if(d < epsilon)
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;

            float q = exp(-0.045 * i);

            vec3 n = grad2(intersection_point);

            const vec3 l = vec3(0.707, 0.707, 0.0);

            float a = 0.5 + 0.5 * dot(n, l);

            vec3 ambient = vec3(
                0.125f * vnoise(0.67 * intersection_point) + 0.0625f * vnoise(3.64 *  intersection_point) + 0.03125f * vnoise(6.77 * intersection_point),
                0.25f * vnoise(1.47 * intersection_point) + 0.125f * vnoise(3.17 *  intersection_point) + 0.0625f * vnoise(6.49 * intersection_point),
                0.5f * vnoise(1.78 * intersection_point) + 0.25f * vnoise(4.38 *  intersection_point) + 0.125f * vnoise(7.19 * intersection_point)
            );

            // specular term, Blinn - Phong lighting
            vec3 h = normalize(l - direction);
            float cos_alpha = max(dot(h, n), 0.0f);
            return mix(fog_color, a * ambient + pow(cos_alpha, 40.0) * vec3(1.0f), q);

        }
        t += d;
        if(t > 500.0f)
            break;
        ++i;
    }
    return fog_color;
}

//=================================================================================================================================
// Example16 :: smooth lattice
//=================================================================================================================================

vec3 cellular_mercury(vec3 position)
{
    float tt = 2.75f * time;
    vec3 q = vec3(vnoise(4.0f * position + vec3(tt,  1.45f,  33.1f)), 
                  vnoise(4.0f * position + vec3( 43.9f, tt, -11.5f)), 
                  vnoise(4.0f * position + vec3(-17.3f, -43.7f, tt)));

    const vec3 base_color1 = vec3(0.95f, 0.98f, 1.00f);
    const vec3 base_color2 = vec3(0.71f, 0.73f, 1.15f);

    vec2 F = cellular(4.0f * position + q);
    vec3 color = F.x * base_color1 + F.y * base_color2;
    return 0.28 * color;
}

float sd_cross_lattice16(vec3 p, float size, float period)
{       
    vec3 q = p;
    q.xyz -= period * round(q.xyz / period);

    vec3 d = abs(q) - vec3(size);
    float d_xy = min(max(d.x, d.y), 0.0f) + length(max(d.xy, 0.0f));
    float d_yz = min(max(d.y, d.z), 0.0f) + length(max(d.yz, 0.0f));
    float d_zx = min(max(d.z, d.x), 0.0f) + length(max(d.zx, 0.0f));

    float smooth_scale = 2.5;
    return smin2(d_xy, smin2(d_yz, d_zx, smooth_scale), smooth_scale) - smooth_scale;
}

float Map16(vec3 p)
{
    return sd_cross_lattice16(p, 10.0, 80.0);
}

vec3 grad16(in vec3 pos)
{
    vec2 eps = vec2(0.0075f, 0.0f);
    vec3 nor = vec3(Map16(pos + eps.xyy) - Map16(pos - eps.xyy),
                    Map16(pos + eps.yxy) - Map16(pos - eps.yxy),
                    Map16(pos + eps.yyx) - Map16(pos - eps.yyx));
    return normalize(nor);
}

vec3 plasma(vec3 position)
{
    vec3 tex_coord = position + 0.1f * vec3(snoise(position + vec3(0.0, 0.0, time)),
                                            snoise(position + vec3(43.0, 17.0, time)),
                                            snoise(position + vec3(-17.0, -43.0, time)));
    float n      = snoise(tex_coord - vec3(0.0, 0.0, time)) + 
        0.50000f * snoise(tex_coord *  2.0f - vec3(0.0f, 0.0f, time * 1.4f)) +  
        0.25000f * snoise(tex_coord *  4.0f - vec3(0.0f, 0.0f, time * 2.0f)) +
        0.12500f * snoise(tex_coord *  8.0f - vec3(0.0f, 0.0f, time * 2.8f)) +
        0.06250f * snoise(tex_coord * 16.0f - vec3(0.0f, 0.0f, time * 4.0f));

    return vec3(1.0, 0.5, 0.0) + vec3(0.7 * n);
}

vec3 raymarch16(vec3 position, vec3 direction)
{
    vec3 fog_color = vec3(0.3751f, 0.0213f, 0.071f);
    const float epsilon = 0.025f;
    const int maxSteps = 128;
    float t = 0.0f;
    int i = 0;
    while(i < maxSteps) 
    {
        float d = Map16(position + direction * t);

        if(d < epsilon)
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;
            float q = exp(-0.025 * i);
            vec3 n = grad16(intersection_point);
            vec3 ambient = cellular_mercury(0.055 * intersection_point);
            const vec3 l = vec3(0.707, 0.707, 0.0);
            float a = clamp(0.3 + 0.7 * dot(n, l), 0.0, 1.0);

            // specular term, Blinn - Phong lighting
            vec3 h = normalize(l - direction);
            float cos_alpha = max(dot(h, n), 0.0f);
            return mix(fog_color, a * ambient + pow(cos_alpha, 40.0) * vec3(1.0f), q);

        }
        t += d;
        if(t > 500.0f)
            break;
        ++i;
    }
    return fog_color;
}

//=================================================================================================================================
// Example17 :: group orbits
//=================================================================================================================================

float ApollonMap(vec3 p, float s)
{
    p *= 0.25;
    float scale = 1.0;
    for(int i = 0; i < 8; i++)
    {
        p -= 2.0f * round(0.5f * p);
        float r = dot(p, p);        
        float k = 1.125 / r;
        p *= k;
        scale *= k;
    }
    // return 0.25f * abs(p.y) / scale;
    return (0.25f * (abs(p.y) + abs(p.x) + abs(p.z)) / scale) + 0.0125;
}

float Map17(vec3 p)
{
    return ApollonMap(p, 1.25f);
}

vec3 grad17(in vec3 pos)
{
    vec2 eps = vec2(0.0075f, 0.0f);
    vec3 nor = vec3(Map17(pos + eps.xyy) - Map17(pos - eps.xyy),
                    Map17(pos + eps.yxy) - Map17(pos - eps.yxy),
                    Map17(pos + eps.yyx) - Map17(pos - eps.yyx));
    return normalize(nor);
}


vec3 raymarch17(vec3 position, vec3 direction)
{
    vec3 fog_color = vec3(0.3751f, 0.0213f, 0.071f);
    const float epsilon = 0.025f;
    const int maxSteps = 128;
    float t = 0.0f;
    int i = 0;
    while(i < maxSteps) 
    {
        float d = Map17(position + direction * t);

        if(d < epsilon)
        {
            // Do something with p
            vec3 intersection_point = position + t * direction;

            float q = exp(-0.035 * i);

            vec3 n = grad17(intersection_point);

            const vec3 l = vec3(0.707, 0.707, 0.0);

            float a = clamp(0.5 + 0.5 * dot(n, l), 0.0, 1.0);

            vec3 ambient = plasma(8.0f * intersection_point);

            // specular term, Blinn - Phong lighting
            vec3 h = normalize(l - direction);
            float cos_alpha = max(dot(h, n), 0.0f);
            return mix(fog_color, a * ambient + pow(cos_alpha, 40.0) * vec3(1.0f), q);

        }
        t += 0.75 * d;
        if(t > 500.0f)
            break;
        ++i;
    }
    return fog_color;
}

//=================================================================================================================================
// Example18 :: antialiasing by continued raymarch
//=================================================================================================================================
float sd_cube18(vec3 p, float size)
{
    p.xy = -vec2(8.0) + mod(p.xy, vec2(16.0));
    vec3 d = abs(p) - vec3(size);
    return min(max(d.x, max(d.y, d.z)), 0.0f) + length(max(d, 0.0f));
}

/*
float sd_cross_lattice18(vec3 p, float size, float period)
{
    vec3 q = p - period * round(p / period);
    vec3 aq = abs(q) - size;
    vec3 mq = max(aq, aq.yzx);
    vec3 d1 = min(mq, 0.0f);
    vec3 aq_xyz = max(aq, 0.0f);
    vec3 aq_yzx = aq_xyz.yzx;
    vec3 d2 = sqrt(aq_xyz * aq_xyz + aq_yzx * aq_yzx);
    vec3 d = d1 + d2;
    return min(d.x, min(d.y, d.z));
}
*/

float sd_func18(vec3 p)
{
    return sd_cube18(p, 7.0f);
}

vec3 ambient18(vec3 p)
{
    return vec3(
        0.5f * vnoise(1.67 * p) + 0.25f * vnoise(7.64 * p) + 0.125f * vnoise(11.77 * p),
        0.5f * vnoise(3.47 * p) + 0.25f * vnoise(8.17 * p) + 0.125f * vnoise(10.49 * p),
        0.5f * vnoise(2.78 * p) + 0.25f * vnoise(9.38 * p) + 0.125f * vnoise(12.19 * p)
      );
}

vec3 grad18(in vec3 pos)
{
    vec2 eps = vec2(0.0075f, 0.0f);
    vec3 nor = vec3(sd_func18(pos + eps.xyy) - sd_func18(pos - eps.xyy),
                    sd_func18(pos + eps.yxy) - sd_func18(pos - eps.yxy),
                    sd_func18(pos + eps.yyx) - sd_func18(pos - eps.yyx));
    return normalize(nor);
}


float smootherstep5(float edge0, float edge1, float x)
{
    x = clamp((x - edge0)/(edge1 - edge0), 0.0, 1.0);
    return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
}

vec3 raymarch18(vec3 position, vec3 direction)
{
    const vec3 fog_color = vec3(0.347, 0.031, 0.053);
    const float pixel_size = 0.001;
    const int maxSteps = 128;
    int i = 0;

    const float epsilon = 0.0015625;
    float t = sd_func18(position);

    while(i < maxSteps) 
    {
        vec3 np = position + t * direction;
        float d = sd_func18(np);
        if(d <= epsilon)
        {
            // np is the position we arrived at 
            // the probablitily that function is positive here is high
            // if sd_func19(position + (t + d + d) * direction)


            vec3 cp = np;
            int j = 0;
            float d1 = d + pixel_size * t;
            float csize = abs(d1);

            while((d1 > epsilon) && (j < 8))
            {
                t += d1;
                d1 = sd_func18(position + t * direction) + pixel_size * t;
                ++j;
                csize = min(abs(d1), csize);
            }

            float alpha = exp(-17.7f * csize);

            vec3 ambient = ambient18(cp);

//            if (alpha > 0.95)
  //              return vec3(0.0, 1.0, 0.0);

            // we are near the boundary, let us continue raymarching to get the 
            // underlying color for blending
            //if (alpha)
            {
                int k = 0;
                vec3 background_c = fog_color;
                while(k < maxSteps)
                {
                    vec3 np2 = position + t * direction;
                    d = sd_func18(np2);
                    if (d <= epsilon)
                    {
                        background_c = ambient18(np2);
                        break;
                    }
                    t += d;
                    if(t > 500.0f)
                        break;                    
                    ++k;
                }
                ambient = alpha * ambient + (1.0 - alpha) * background_c;
            }

            return exp(-0.05 * i) * ambient;
        }
        t += d;
        if(t > 500.0f)
            break;
        ++i;
    }
    return fog_color;
}

//=================================================================================================================================
// Example19 :: Plato solids
//=================================================================================================================================

const float sqrt3     = 1.732050807568877293527446341505872366942805253810380628055; // sqrt(3)
const float inv_sqrt3 = 0.577350269189625764509148780501957455647601751270126876018; // 1 / sqrt(3)
const float phi       = 1.618033988749894848204586834365638117720309179805762862135; // (sqrt(5) + 1) / 2
const float inv_phi   = 0.618033988749894848204586834365638117720309179805762862135; // (sqrt(5) - 1) / 2
const float mu        = 0.525731112119133606025669084847876607285497932243341781528; // (3 - sqrt(5))/(2 * sqrt(5 - 2 * sqrt(5)))
const float nu        = 0.850650808352039932181540497063011072240401403764816881836; // (sqrt(5) - 1)/(2 * sqrt(5 - 2 * sqrt(5)))
const float tau       = 0.934172358962715696451118623548045329629287826516995242440; // (sqrt(5) + 1) / (2 * sqrt(3))
const float psi       = 0.356822089773089931941969843046087873981686075246868366421; // (sqrt(5) - 1) / (2 * sqrt(3))

float sd_tetrahedron(vec3 p, float size)
{
    vec3 l = p - p.yzx - p.zxy;
    float s = p.x + p.y + p.z;
    float m = max(max(s, l.x), max(l.y, l.z));
    return inv_sqrt3 * m - size;
}

float sd_octahedron(vec3 p, float size)
{
    vec3 q = abs(p);
    return inv_sqrt3 * (q.x + q.y + q.z) - size;
}

float sd_cube(vec3 p, float size)
{
    vec3 q = abs(p);
    return max(q.x, max(q.y, q.z)) - size;
}

float sd_dodecahedron(vec3 p, float size)
{
    vec3 q = abs(p);
    vec3 l = mu * q + nu * q.yzx;
    return max(l.x, max(l.y, l.z)) - size;
}

float sd_icosahedron(vec3 p, float size)
{
    vec3 q = abs(p);
    float v = inv_sqrt3 * (q.x + q.y + q.z);
    vec3 l = tau * q + psi * q.yzx;
    return max(max(v, l.x), max(l.y, l.z)) - size;
}

float sd_dodecahedron1(vec3 p, float size)
{
    vec3 v1 = mu * p;
    vec3 v2 = nu * p.yzx;
    vec3 l = abs(v1 + v2) + abs(v1 - v2);
    float d = 0.31 * (l.x + l.y + l.z);
    return d - size;
}

float sd_icosahedron1(vec3 p, float size)
{
    vec3 w0 = tau * p;
    vec3 w1 = psi * p.yzx;
    vec3 w2 = psi * p.zxy;
    vec3 l1 = abs(w0 + w1) + abs(w0 - w1);
    vec3 l2 = abs(w0 + w2) + abs(w0 - w2);
    vec3 l = l1 + l2;
    float d = 0.15 * (l.x + l.y + l.z);
    return d - size;
}


vec3 ambient19(vec3 p)
{
    float time0 = 2.2 * time;

    mat2 r = mat2(vec2( 0.540302305868139717400936607442976603732310420617922227670, 0.841470984807896506652502321630298999622563060798371065672),
                  vec2(-0.841470984807896506652502321630298999622563060798371065672, 0.540302305868139717400936607442976603732310420617922227670));
    vec3 q1 = vnoise3((1.67 + 0.94 * cos(0.049 * time0)) * p + vec3(-0.121, -0.013,  0.023) * time0);
    p.xy = r * p.xy;
    p.yz = r * p.yz;
    vec3 q2 = vnoise3((4.39 + 2.17 * sin(0.053 * time0)) * p + vec3(-0.033, -0.153,  0.146) * time0);
    p.xy = r * p.xy;
    p.yz = r * p.yz;
    vec3 q3 = vnoise3((6.97 + 4.41 * cos(0.051 * time0)) * p + vec3( 0.029,  0.027, -0.037) * time0);

    vec3 q = 0.63 * q1 + 0.29 * q2 + 0.08 * q3;
    vec3 qq = q * q;
    vec3 qqq = qq * q;

    vec3 xxx = vec3(q.x, qq.x, qqq.x);
    vec3 yyy = vec3(q.y, qq.y, qqq.y);
    vec3 zzz = vec3(q.z, qq.z, qqq.z);

    vec3 color = xxx * xxx + 0.67 * yyy * yyy + 0.34 * zzz * zzz;
    vec3 w = clamp(1.27 * vec3(1.92, 1.27, 0.94) * color, 0.0, 1.0);

    return w.zyx;
}

float sd_func19(vec3 p)
{
    const float period = 64.0;
    vec3 q = p; // - period * round(p / period);    
    //return sd_octahedron(p, 7.0);
    //return sd_cube(p, 7.0);
    //return sd_icosahedron(q, 16.0);

    vec3 h = ambient19(0.5 * p);
    float b = h.x + h.y + h.z;


    return 0.8 * max(sd_dodecahedron(q, 24.0), -sd_icosahedron(q, 24.0)) - 0.2 * b;
}

vec3 grad19(in vec3 pos)
{
    vec2 eps = vec2(0.0075f, 0.0f);
    vec3 nor = vec3(sd_func19(pos + eps.xyy) - sd_func19(pos - eps.xyy),
                    sd_func19(pos + eps.yxy) - sd_func19(pos - eps.yxy),
                    sd_func19(pos + eps.yyx) - sd_func19(pos - eps.yyx));
    return normalize(nor);
}

/*
vec3 ambient19(vec3 p)
{

    mat2 r = mat2(vec2( 0.540302305868139717400936607442976603732310420617922227670, 0.841470984807896506652502321630298999622563060798371065672),
                  vec2(-0.841470984807896506652502321630298999622563060798371065672, 0.540302305868139717400936607442976603732310420617922227670));

    float q1 = vnoise(1.67 * p + 5.234 + 0.21 * time);
    float q2 = vnoise(3.47 * p + 1.243 - 0.13 * time);
    float q3 = vnoise(2.78 * p + 3.567 - 0.23 * time);

    p.xy = r * p.xy;
    p.yz = r * p.yz;

    q1 += 0.63 * vnoise(6.38 * p - 8.324 - 0.33 * time);
    q2 += 0.63 * vnoise(3.64 * p - 5.856 - 0.53 * time);
    q3 += 0.63 * vnoise(5.17 * p + 7.148 + 0.46 * time);

    p.zx = r * p.zx;
    p.xy = r * p.xy;

    q1 += 0.31 * vnoise(7.19 * p - 9.215 + 0.29 * time);
    q2 += 0.31 * vnoise(9.77 * p + 4.573 + 0.27 * time);     
    q3 += 0.31 * vnoise(8.49 * p - 1.347 - 0.37 * time);

    vec3 a = vec3(q1 * q1, q1 * q1 * q1, q1);
    vec3 b = vec3(q2 * q2, q2 * q2 * q2, q2);
    vec3 c = vec3(q3 * q3, q3 * q3 * q3, q3);
    vec3 d = a + b * c + c * a;
    return 0.19 * d; //vec3(d.x + d.y + d.z);
}
*/



vec3 raymarch19(vec3 position, vec3 direction)
{
    const vec3 fog_color = vec3(0.137f, 0.007f, 0.013f);
    const int maxSteps = 160;
    int i = 0;

    const float epsilon = 0.00015625;
    float t = sd_func19(position);

    while(i < maxSteps) 
    {
        vec3 np = position + t * direction;
        float d = sd_func19(np);
        if(d <= epsilon)
        {
            vec3 intersection_point = position + t * direction;
            float q = exp(-0.035 * i);
            vec3 n = grad19(intersection_point);

            const vec3 l = vec3(0.707, 0.707, 0.0);

            float a = clamp(0.6 + 0.4 * dot(n, l), 0.0, 1.0);

            vec3 ambient = ambient19(0.5 * intersection_point);

            // specular term, Blinn - Phong lighting
            vec3 h = normalize(l - direction);
            float cos_alpha = max(dot(h, n), 0.0f);
            return mix(fog_color, a * ambient + 0.73 * pow(cos_alpha, 40.0) * vec3(1.0f), q);
        }
        t += d;
        if(t > 750.0f)
            break;
        ++i;
    }
    return fog_color;
}



/*

// p as usual, e exponent (p in the paper), r radius or something like that
float octahedral(vec3 p, float e, float r)
{
    float s = pow(abs(dot(p,n4)),e);
    s += pow(abs(dot(p,n5)),e);
    s += pow(abs(dot(p,n6)),e);
    s += pow(abs(dot(p,n7)),e);
    s = pow(s, 1./e);
    return s-r;
}


float dodecahedral(vec3 p, float e, float r)
{
    float s = pow(abs(dot(p,n14)),e);
    s += pow(abs(dot(p,n15)),e);
    s += pow(abs(dot(p,n16)),e);
    s += pow(abs(dot(p,n17)),e);
    s += pow(abs(dot(p,n18)),e);
    s += pow(abs(dot(p,n19)),e);
    s = pow(s, 1./e);
    return s-r;
}

float icosahedral(vec3 p, float e, float r) {
    float s = pow(abs(dot(p,n4)),e);
    s += pow(abs(dot(p,n5)),e);
    s += pow(abs(dot(p,n6)),e);
    s += pow(abs(dot(p,n7)),e);
    s += pow(abs(dot(p,n8)),e);
    s += pow(abs(dot(p,n9)),e);
    s += pow(abs(dot(p,n10)),e);
    s += pow(abs(dot(p,n11)),e);
    s += pow(abs(dot(p,n12)),e);
    s += pow(abs(dot(p,n13)),e);
    s = pow(s, 1./e);
    return s-r;
}

float toctahedral(vec3 p, float e, float r) {
    float s = pow(abs(dot(p,n1)),e);
    s += pow(abs(dot(p,n2)),e);
    s += pow(abs(dot(p,n3)),e);
    s += pow(abs(dot(p,n4)),e);
    s += pow(abs(dot(p,n5)),e);
    s += pow(abs(dot(p,n6)),e);
    s += pow(abs(dot(p,n7)),e);
    s = pow(s, 1./e);
    return s-r;
}

float ticosahedral(vec3 p, float e, float r) {
    float s = pow(abs(dot(p,n4)),e);
    s += pow(abs(dot(p,n5)),e);
    s += pow(abs(dot(p,n6)),e);
    s += pow(abs(dot(p,n7)),e);
    s += pow(abs(dot(p,n8)),e);
    s += pow(abs(dot(p,n9)),e);
    s += pow(abs(dot(p,n10)),e);
    s += pow(abs(dot(p,n11)),e);
    s += pow(abs(dot(p,n12)),e);
    s += pow(abs(dot(p,n13)),e);
    s += pow(abs(dot(p,n14)),e);
    s += pow(abs(dot(p,n15)),e);
    s += pow(abs(dot(p,n16)),e);
    s += pow(abs(dot(p,n17)),e);
    s += pow(abs(dot(p,n18)),e);
    s += pow(abs(dot(p,n19)),e);
    s = pow(s, 1./e);
    return s-r;
}

*/


//=================================================================================================================================
// Example20 :: cellular labyrinth
//=================================================================================================================================

// gradient 3D noise
float gnoise(vec3 P)
{
    vec2 zero_texel = vec2(0.0, TEXEL_SIZE);
    vec3 Pi = floor(P);
    vec3 Pf = P - Pi;                                                                   // Fractional part for interpolation
    vec2 uvz0 = Pi.xy + vec2(37.0, 17.0) * Pi.z;
    vec2 uvz1 = uvz0 + vec2(37.0, 17.0);
    uvz0 = TEXEL_SIZE * uvz0 + HALF_TEXEL;
    uvz1 = TEXEL_SIZE * uvz1 + HALF_TEXEL;
    vec3 Ps = hermite5(Pf);

    
    vec4 gz0 = vec4(                                                                    // g00 : g10 : g01 : g11
            dot(texture(value_texture, uvz0).rgb, Pf),
            dot(texture(value_texture, uvz0 + zero_texel.yx).rgb, Pf - zero_texel.yxx),
            dot(texture(value_texture, uvz0 + zero_texel.xy).rgb, Pf - zero_texel.xyx),
            dot(texture(value_texture, uvz0 + zero_texel.yy).rgb, Pf - zero_texel.yyx)
        );

    vec4 gz1 = vec4(
            dot(texture(value_texture, uvz1).rgb, Pf - zero_texel.xxy),
            dot(texture(value_texture, uvz1 + zero_texel.yx).rgb, Pf - zero_texel.yxy),
            dot(texture(value_texture, uvz1 + zero_texel.xy).rgb, Pf - zero_texel.xyy),
            dot(texture(value_texture, uvz1 + zero_texel.yy).rgb, Pf - zero_texel.yyy)
        );

    vec4 gz = mix(gz1, gz0, Ps.z);
    vec2 gy = mix(gz.xy, gz.zw, Ps.y);
    return mix(gy.x, gy.y, Ps.x);
}

vec3 fade(vec3 t)
    { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }

// gradient 3d noise
float cnoise(vec3 P)
{
    vec3 Pi0 = floor(P);                                                // Integer part for indexing
    vec3 Pi1 = Pi0 + vec3(1.0);                                         // Integer part + 1
    Pi0 = mod(Pi0, 289.0);
    Pi1 = mod(Pi1, 289.0);
    vec3 Pf0 = fract(P);                                                // Fractional part for interpolation
    vec3 Pf1 = Pf0 - vec3(1.0);                                         // Fractional part - 1.0
    vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
    vec4 iy = vec4(Pi0.yy, Pi1.yy);
    vec4 iz0 = Pi0.zzzz;
    vec4 iz1 = Pi1.zzzz;

    vec4 ixy = permute(permute(ix) + iy);
    vec4 ixy0 = permute(ixy + iz0);
    vec4 ixy1 = permute(ixy + iz1);

    vec4 gx0 = ixy0 / 7.0;
    vec4 gy0 = fract(floor(gx0) / 7.0) - 0.5;
    gx0 = fract(gx0);
    vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
    vec4 sz0 = step(gz0, vec4(0.0));
    gx0 -= sz0 * (step(0.0, gx0) - 0.5);
    gy0 -= sz0 * (step(0.0, gy0) - 0.5);

    vec4 gx1 = ixy1 / 7.0;
    vec4 gy1 = fract(floor(gx1) / 7.0) - 0.5;
    gx1 = fract(gx1);
    vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
    vec4 sz1 = step(gz1, vec4(0.0));
    gx1 -= sz1 * (step(0.0, gx1) - 0.5);
    gy1 -= sz1 * (step(0.0, gy1) - 0.5);
    
    vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
    vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
    vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
    vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
    vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
    vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
    vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
    vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);
    
    vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
    g000 *= norm0.x;
    g010 *= norm0.y;
    g100 *= norm0.z;
    g110 *= norm0.w;
    vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
    g001 *= norm1.x;
    g011 *= norm1.y;
    g101 *= norm1.z;
    g111 *= norm1.w;
    
    float n000 = dot(g000, Pf0);
    float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
    float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
    float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
    float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
    float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
    float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
    float n111 = dot(g111, Pf1);
    
    vec3 fade_xyz = fade(Pf0);
    vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
    vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
    float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x); 
    return 2.2 * n_xyz;
}


float deformed_cross20(out vec4 color, in vec3 p, in float size, in float period)
{       
    vec3 q = p;
    q.xyz -= period * round(q.xyz / period);

    vec3 d = abs(q) - vec3(size);
    float d_xy = min(max(d.x, d.y), 0.0f) + length(max(d.xy, 0.0f));
    float d_yz = min(max(d.y, d.z), 0.0f) + length(max(d.yz, 0.0f));
    float d_zx = min(max(d.z, d.x), 0.0f) + length(max(d.zx, 0.0f));

    float smooth_scale = 2.5;
    float v = smin2(d_xy, smin2(d_yz, d_zx, smooth_scale), smooth_scale) - smooth_scale;
//    float v = min(d_xy, min(d_yz, d_zx)) - smooth_scale;

    p *= 3.0;

    color = vec4(cnoise(0.167 * p),
                 cnoise(0.439 * p),
                 cnoise(0.697 * p),
                 cnoise(1.153 * p));

    float l = dot(color, vec4(0.59, 0.22, 0.10, 0.03));
    return v + 0.15 * l;    
}

float sd_func20(out vec4 color, in vec3 p)
{
    return deformed_cross20(color, p, 3.0, 30.0);
}

vec3 grad20(in vec3 pos)
{
    vec2 eps = vec2(0.0075f, 0.0f);
    vec4 color;
    vec3 nor = vec3(sd_func20(color, pos + eps.xyy) - sd_func20(color, pos - eps.xyy),
                    sd_func20(color, pos + eps.yxy) - sd_func20(color, pos - eps.yxy),
                    sd_func20(color, pos + eps.yyx) - sd_func20(color, pos - eps.yyx));
    return normalize(nor);
}

vec3 raymarch20(vec3 position, vec3 direction)
{
    const vec3 fog_color = vec3(0.47f, 0.51f, 0.95f);
    const int maxSteps = 160;
    int i = 0;

    const float epsilon = 0.0045625;

    vec4 color;
    float t = sd_func20(color, position);

    while(i < maxSteps) 
    {
        vec3 np = position + t * direction;
        float d = sd_func20(color, np);
        if(abs(d) <= epsilon)
        {
            vec3 intersection_point = position + t * direction;
            float q = exp(-0.035 * i);
            vec3 n = grad20(intersection_point);

            const vec3 l = vec3(0.707, 0.707, 0.0);

            float a = clamp(0.6 + 0.4 * dot(n, l), 0.0, 1.0);

            float u1 = abs(color.x);
            float u2 = abs(color.y);
            float u3 = abs(color.x);
            float u4 = abs(color.x);
            vec3 blue1 = vec3(u1 * u1 * u1, u1 * u1, u1);
            vec3 blue2 = vec3(u2 * u2 * u2, u2 * u2, u2);
            vec3 blue3 = vec3(u3 * u3 * u3, u3 * u3, u3);
            vec3 blue4 = vec3(u4 * u4 * u4, u4 * u4, u4);

            vec3 ambient = vec3(1.15, 1.25, 0.75) * pow(blue1 * blue2 * blue3 * blue4, vec3(0.28)); // color.xxx + color.yyy + color.zzz;

            // specular term, Blinn - Phong lighting
            vec3 h = normalize(l - direction);
            float cos_alpha = max(dot(h, n), 0.0f);
            return mix(fog_color, a * ambient + 0.73 * pow(cos_alpha, 40.0) * vec3(1.0f), q);
        }
        t += d;
        if(t > 250.0f)
            break;
        ++i;
    }
    return fog_color;
}
void main()
{
    vec3 direction = normalize(view);

    // Ex1 :: 8 spheres with explicit intersection calculation
    // vec3 n = intersect(position, direction);

    // Ex2 :: the same 8 spheres but with distance field marching
    // vec3 n = raymarch(position, direction);

    // Ex3 :: infinite periodic pattern
    // vec3 n = raymarch3(position, direction);

    // Ex4 :: cubes
    // vec3 n = raymarch4(position, direction);

    // Ex5 :: union operation
    // vec3 n = raymarch5(position, direction);

    // Ex6 :: torus
    // vec3 n = raymarch6(position, direction);

    // Ex7 :: torus + cube + sphere
    // vec3 n = raymarch7(position, direction);

    // Ex8 :: Minkowski torus
    // vec3 n = raymarch8(position, direction);

    // Ex9 :: stick
    // vec3 n = raymarch9(position, direction);

    // Ex10 :: deformed stick
    // vec3 n = raymarch10(position, direction);

    // Ex11 :: raymarch number of steps
    // vec3 n = raymarch11(position, direction);

    // Ex12 :: raymarch enhancement
    // vec3 n = raymarch12(position, direction);

    // Ex13 :: infinite lattice
    // vec3 n = raymarch13(position, direction);

    // Ex14 :: intersection
    // vec3 n = raymarch14(position, direction);

    // Ex15 :: smooth intersection
    // vec3 n = raymarch15(position, direction);

    // Ex16 :: smooth lattice
    // vec3 n = raymarch16(position, direction);

    // Ex17 :: orbital geometry
    // vec3 n = raymarch17(position, direction);

    // Ex18 :: antialiazing
    // vec3 n = raymarch18(position, direction);

    // Ex19 :: plato solids
    // vec3 n = raymarch19(position, direction);

    // Ex20 :: cellular labyrinth
    vec3 n = raymarch20(position, direction);



    FragmentColor = vec4(n, 1.0f);
}