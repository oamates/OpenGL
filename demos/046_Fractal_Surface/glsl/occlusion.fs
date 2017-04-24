#version 330 core

in vec3 position_ws;  
in vec3 normal_ws;  
in float occlusion;

const vec3 light_ws = vec3(2.0f, 3.0f, 4.0f);
uniform vec3 camera_ws;

out vec4 FragmentColor;



//==============================================================================================================================================================
// Helper functions
//==============================================================================================================================================================
vec2 mod289(vec2 x) { return x - floor(x * (1.0f / 289.0f)) * 289.0f; }
vec3 mod289(vec3 x) { return x - floor(x * (1.0f / 289.0f)) * 289.0f; }
vec4 mod289(vec4 x) { return x - floor(x * (1.0f / 289.0f)) * 289.0f; }

float permute(float x) { return mod((34.0f * x + 1.0f) * x, 289.0f); }
vec3  permute(vec3 x)  { return mod((34.0f * x + 1.0f) * x, 289.0f); }
vec4  permute(vec4 x)  { return mod((34.0f * x + 1.0f) * x, 289.0f); }

vec4 taylorInvSqrt(vec4 r) { return 1.792843f - 0.853735f * r; }

float smootherstep(float edge0, float edge1, float x)
{
    x = clamp((x - edge0)/(edge1 - edge0), 0.0f, 1.0f);
    return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
}

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
    vec4 y_ = floor(j - 7.0f * x_);                                         // mod(j, N)
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

//==============================================================================================================================================================
//  Plain cellular noise
//==============================================================================================================================================================
vec2 cellular(vec2 P) 
{
    const float K = 0.142857142857f;                                        // 1.0f / 7.0f
    const float Ko = 0.428571428571f;                                       // 3.0f / 7.0f
    const float jitter = 1.0f;                                              // Less gives more regular pattern

    // Permutation polynomial: (34x^2 + x) mod 289. Standard 3x3 search window for good F1 and F2 values
    vec2 Pi = mod(floor(P), 289.0f);
    vec2 Pf = fract(P);
    vec3 oi = vec3(-1.0f, 0.0f, 1.0f);
    vec3 of = vec3(-0.5f, 0.5f, 1.5f);
    vec3 px = permute(Pi.x + oi);
    vec3 p = permute(px.x + Pi.y + oi);                                     // p11, p12, p13
    vec3 ox = fract(p*K) - Ko;
    vec3 oy = mod(floor(p*K), 7.0f) * K - Ko;
    vec3 dx = Pf.x + 0.5 + jitter * ox;
    vec3 dy = Pf.y - of + jitter * oy;
    vec3 d1 = dx * dx + dy * dy;                                            // d11, d12 and d13, squared
    p = permute(px.y + Pi.y + oi);                                          // p21, p22, p23
    ox = fract(p * K) - Ko;
    oy = mod(floor(p * K), 7.0f) * K - Ko;
    dx = Pf.x - 0.5 + jitter * ox;
    dy = Pf.y - of + jitter * oy;
    vec3 d2 = dx * dx + dy * dy;                                            // d21, d22 and d23, squared
    p = permute(px.z + Pi.y + oi);                                          // p31, p32, p33
    ox = fract(p * K) - Ko;
    oy = mod(floor(p * K), 7.0f) * K - Ko;
    dx = Pf.x - 1.5 + jitter * ox;
    dy = Pf.y - of + jitter * oy;
    vec3 d3 = dx * dx + dy * dy;                                            // d31, d32 and d33, squared

    // Sort out the two smallest distances (F1, F2)
    vec3 d1a = min(d1, d2);
    d2 = max(d1, d2);                                                       // Swap to keep candidates for F2
    d2 = min(d2, d3);                                                       // neither F1 nor F2 are now in d3
    d1 = min(d1a, d2);                                                      // F1 is now in d1
    d2 = max(d1a, d2);                                                      // Swap to keep candidates for F2
    d1.xy = (d1.x < d1.y) ? d1.xy : d1.yx;                                  // Swap if smaller
    d1.xz = (d1.x < d1.z) ? d1.xz : d1.zx;                                  // F1 is in d1.x
    d1.yz = min(d1.yz, d2.yz);                                              // F2 is now not in d2.yz
    d1.y = min(d1.y, d1.z);                                                 // nor in d1.z
    d1.y = min(d1.y, d2.x);                                                 // F2 is in d1.y, we're done.
    return sqrt(d1.xy);
}

//==============================================================================================================================================================
// Procedural color functions
//==============================================================================================================================================================
vec3 simplex_turbulence(in vec3 position)
{
    const int iterations = 6;
    const vec3 base_color[iterations] = vec3[iterations] 
    (
        vec3(0.770f, 0.441f, 0.16f),
        vec3(0.555f, 0.593f, 0.21f),
        vec3(0.344f, 0.675f, 0.27f),
        vec3(0.232f, 0.732f, 0.31f),
        vec3(0.125f, 0.796f, 0.33f),
        vec3(0.125f, 0.876f, 0.35f)
    );

    const float frequency_factor = 2.25f;
    const float weight_factor = 0.46f; 

    float frequency = 3.0f;
    float weight = 1.0f;
    vec3 color = vec3(0.0f);

    for (int i = 0; i < iterations; ++i)
    {
        color += weight * abs(snoise(position * frequency)) * base_color[i];
        frequency *= frequency_factor;
        weight *= weight_factor;
    };
    return (1.0f - weight_factor) * color;
}

vec3 marble_light(in vec3 position)
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

vec3 marble_complex(in vec3 position)
{
    const vec3 marble_color0 = 1.5 * vec3(0.125f, 0.065f, 0.057f * 6.0f);
    const vec3 marble_color1 = 1.5 * vec3(0.344f, 0.231f, 0.111f * 6.0f);
    const vec3 marble_color2 = 1.5 * vec3(0.444f, 0.331f, 0.081f * 6.0f);
    const vec3 marble_color3 = 1.5 * vec3(0.344f, 0.411f, 0.071f * 6.0f);

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
    vec3 v = normalize(camera_ws - position_ws);
    vec3 light = light_ws - position_ws;
    float dist = length(light);
    vec3 l = light / dist;
    vec3 n = normalize(normal_ws);

    vec3 diffuse_color = marble_complex(10.0 * position_ws);


    float cos_theta = dot(n, l);

    vec3 color = diffuse_color * 0.25 * (0.4f + 0.6f * occlusion);                                      // ambient component
    vec3 specular_color = vec3(0.707f, 0.707f, 0.707f);
    const float Ns = 64.0f;

    if (cos_theta > 0.0f) 
    {
        float factor = (0.6f + 0.4f * occlusion) / (1.0 + 0.21 * dist);
        color += cos_theta * diffuse_color * factor;

        // Phong lighting
//        vec3 r = reflect(-l, n);
//        float cos_alpha = max(dot(v, r), 0.0f);
//        float exponent = 0.25f * specular_exponent();
        
        // Blinn - Phong lighting
        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, n), 0.0f);

        color += pow(cos_alpha, Ns) * specular_color * factor;
    }
    FragmentColor = vec4(color, 1.0f);
}

 