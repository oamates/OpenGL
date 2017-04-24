#version 330 core

in vec3 position_ws;

out vec4 FragmentColor;

//==============================================================================================================================================================
// Helper functions
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

//==============================================================================================================================================================
// 2D and 3D simplex noise
//==============================================================================================================================================================
float snoise (vec2 v)
{
    const vec4 C = vec4 (0.211324865405187f,                                // (3.0f - sqrt(3.0f)) / 6.0f
                         0.366025403784439f,                                //  0.5f * (sqrt(3.0f) - 1.0f)
                        -0.577350269189626f,                                // -1.0f + 2.0f * C.x
                         0.024390243902439f);                               //  1.0f / 41.0f
    // First corner
    vec2 i = floor (v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);
    // Other corners
    vec2 i1 = (x0.x > x0.y) ? vec2 (1.0f, 0.0f) : vec2 (0.0f, 1.0f);
    vec4 x12 = x0 . xyxy + C. xxzz ;
    x12.xy -= i1 ;
    // Permutations
    i = mod289 (i);                                                         // Avoid truncation effects in permutation
    vec3 p = permute(permute(i.y + vec3(0.0f, i1.y, 1.0f)) + i.x + vec3(0.0f, i1 .x , 1.0f));
    vec3 m = max (0.5f - vec3(dot(x0, x0) , dot(x12.xy, x12.xy), dot(x12.zw , x12.zw)), 0.0f);
    m = m*m;
    m = m *m;
    // Gradients
    vec3 x = 2.0 * fract (p * C. www ) - 1.0;
    vec3 h = abs (x) - 0.5;
    vec3 a0 = x - floor (x + 0.5) ;
    // Normalise gradients implicitly by scaling m
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
    // Compute final noise value at P
    vec3 g = vec3(a0.x * x0.x + h.x * x0.y, a0.yz * x12.xz + h.yz * x12.yw);
    return 130.0f * dot(m, g);
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
    return 93.0f * dot(m * m, vec4( dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}

vec3 marble(vec3 position)
{
    const vec3 marble_color0 = vec3(0.1114f, 0.1431f, 0.0126f);
    const vec3 marble_color1 = vec3(0.9147f, 0.6987f, 0.3947f);

    float n = 0.0f;
    float frequency = 1.87f;
    float weight = 0.77f;
    float frequency_factor = 2.07;
    float weight_factor = 0.47;

    for (int i = 0; i < 4; ++i)
    {
        n += weight * snoise(vec3(position * frequency));
        frequency *= frequency_factor;
        weight *= weight_factor;
    };

    n = (1.0f - weight_factor) * abs(n);
    n = pow(n, 0.45);

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
    float frequency_factor = 2.34;
    float weight_factor = 0.67;
    float weight = 1.0f;

    for (int i = 0; i < 6; ++i)
    {
        n += weight * snoise(vec3(position * frequency));
        frequency *= frequency_factor;
        weight *= weight_factor;
    };

    n = (1.0f - weight_factor) * abs(n);
    n = pow(n, 0.85);

    vec3 color = (n >= 0.6667) ? mix(marble_color2, marble_color3, 3.0f * (n - 0.666667f)) : 
                 (n >= 0.3333) ? mix(marble_color1, marble_color2, 3.0f * (n - 0.333333f)) :
                                 mix(marble_color0, marble_color1, 3.0f * (n - 0.000000f)) ;

    return 1.55f * color;
}


void main()
{    
    FragmentColor = 0.25 * vec4(marble_color(position_ws), 1.0f);
}




