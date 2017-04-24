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

vec2 hermite3(vec2 x) { return x * x * (3.0 - 2.0 * x); }
vec3 hermite3(vec3 x) { return x * x * (3.0 - 2.0 * x); }

vec2 hermite5(vec2 x) { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }
vec3 hermite5(vec3 x) { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

vec2 hermite7(vec2 x) { vec2 sqr = x * x; return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x)); }
vec3 hermite7(vec3 x) { vec3 sqr = x * x; return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x)); }

//==============================================================================================================================================================
// Texture-base 2d and 3d value noise implementation
// naming :: vnoise - value noise, 
//           vnoise2 - value noise in both components, of the same frequency
//           ...
//           gnoise - gradient noise
//           gnoise2 - gradient noise in both components
//           ...
//           snoise - simplex noise
//           snoise2 - simplex noise
//           ...
//==============================================================================================================================================================

const float TEXEL_SIZE = 1.0f / 256.0f;
const float HALF_TEXEL = 1.0f / 512.0f;

float vnoise(vec2 x)
{
    vec2 p = floor(x);
    vec2 f = x - p;
    vec2 uv = p + hermite5(f);
    return texture(value_texture, TEXEL_SIZE * uv + HALF_TEXEL).r;
}

vec2 vnoise2(vec2 x)
{
    vec2 p = floor(x);
    vec2 f = x - p;
    vec2 uv = p + hermite5(f);
    return texture(value_texture, TEXEL_SIZE * uv + HALF_TEXEL).rb;
}

vec4 vnoise4(vec2 x)
{
    vec2 p = floor(x);
    vec2 f = x - p;
    vec2 uv = p + hermite5(f);
    return texture(value_texture, TEXEL_SIZE * uv + HALF_TEXEL).rgba;
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

vec2 vnoise2(vec3 x)
{
    vec3 p = floor(x);
    vec3 f = x - p;
    f = hermite5(f);
    vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec4 values = texture(value_texture, TEXEL_SIZE * uv + HALF_TEXEL);
    return mix(values.ga, values.rb, f.z);
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
}

//==============================================================================================================================================================
// Rotation matrices constructors
//==============================================================================================================================================================

mat2 rotation2d(float angle)
{
    float cs = cos(angle);
    float sn = sin(angle);
    return mat2(cs, sn, -sn, cs);  
}

mat3 rotation_euler(vec3 angles)
{
    vec3 sn = sin(angles);
    vec3 cs = cos(angles);
    return mat3(
            vec3( cs.x * cs.z + sn.x * sn.y * sn.z, cs.x * sn.y * sn.z + cs.z * sn.x, -cs.y * sn.z),
            vec3(-cs.y * sn.x, cs.x * cs.y, sn.y),
            vec3( cs.z * sn.x * sn.y + cs.x * sn.z, sn.x * sn.z - cs.x * cs.z * sn.y, cs.y * cs.z)
           );
}

//==============================================================================================================================================================
// Minkowski norm functions
//==============================================================================================================================================================

float minkowski_norm(vec2 p, float n)
{
    vec2 pa = pow(abs(p), vec2(n));
    return pow(pa.x + pa.y, 1.0f / n);
}

float minkowski_norm(vec3 p, float n)
{
    vec3 pa = pow(abs(p), vec3(n));
    return pow(pa.x + pa.y + pa.z, 1.0f / n);
}

//==============================================================================================================================================================
// Min and max on signed distance functions produce union and intersection of the corresponding geometric objects.
// Smooth versions of these functions do the operations in a smooth way.
//==============================================================================================================================================================

float smooth_min(float p, float q, float delta)
{
    float h = clamp(0.5f + 0.5f * (q - p) / delta, 0.0f, 1.0f);
    return mix(q, p, h) - delta * h * (1.0f - h);
}

float smooth_min(float p, float q, float r, float delta)
{
    return 0.0;
}

float smooth_max(float p, float q, float delta)
{
    float h = clamp(0.5f + 0.5f * (p - q) / delta, 0.0f, 1.0f);
    return mix(q, p, h) + delta * h * (1.0f - h);
}

float smooth_max(float p, float q, float r, float delta)
{
    return 0.0;
}



//==============================================================================================================================================================
// Elementary geometric distance functions
//==============================================================================================================================================================

float sd_sphere(vec3 p, float radius)
{
    return length(p) - radius;
}

float sd_torus(vec3 p, float R, float r)
{
    vec2 q = vec2(length(p.xy) - R, p.z);
    return length(q) - r;
}

float sd_minkowski_torus(vec3 p, float R, float r, float n)
{
    vec2 q = vec2(minkowski_norm(p.xy, n) - R, p.z);
    return minkowski_norm(q, n) - r;
}

float sd_segment(vec3 p, vec3 A, vec3 B, float radius)
{
    vec3 AB = B - A;
    float l = length(AB);
    vec3 nAB = AB / l;
    vec3 AP = p - A;
    float dp = dot(AP, nAB);
    float q = clamp(dp / l, 0.0f, 1.0f);
    vec3 C = mix(A, B, q);
    return length(p - C) - radius;
}

float sd_lattice(vec3 p, float size, float period)
{
    vec3 q = p - period * round(p / period);
    vec3 aq = abs(q);
    vec3 m = max(aq, aq.yzx);
    return min(m.x, min(m.y, m.z)) - size;
}

float sd_smooth_lattice(vec3 p, float size, float period, float smooth_scale)
{       
    vec3 q = p - period * round(p / period);

    vec3 d = abs(q) - vec3(size);
    vec3 d1 = min(max(d, d.yzx), 0.0f);
    vec3 d2 = max(d, 0.0);
    vec3 d3 = d2.yzx;
    d1 = d1 + sqrt(d2 * d2 + d3 * d3);

    return smooth_min(d1.x, smooth_min(d1.y, d1.z, smooth_scale), smooth_scale) - smooth_scale;
}

//==============================================================================================================================================================
// Plato solids signed distance functions
//==============================================================================================================================================================

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

float sd_cube_exact(vec3 p, float size)
{
    vec3 q = abs(p) - vec3(size);
    return min(max(q.x, max(q.y, q.z)), 0.0f) + length(max(q, 0.0f));
}


//==============================================================================================================================================================
// Scene 1 :: smooth infinite periodic lattice
//==============================================================================================================================================================

float sd_1(vec3 p)
{
    const vec3 rgb_power = vec3(0.2126, 0.7152, 0.0722);
    float lsd = sd_smooth_lattice(p, 10.0, 80.0, 2.5);                                      // lattice signed distance, not LSD
    p *= 0.34; 

    vec2 vn1 = vnoise2(p);
    vec2 vn2 = vnoise2(2.03 * p);
    vec2 vn3 = vnoise2(4.39 * p);
    float bump = 0.459 * vn1.x * vn1.y + 0.316 * vn2.x * vn2.y + 0.271 * vn3.x * vn3.y;
    return 0.65 * lsd - 0.65 * bump;
}

vec3 grad_1(vec3 p)
{
    vec2 e = vec2(0.01f, 0.0f);
    vec3 n = vec3(sd_1(p + e.xyy) - sd_1(p - e.xyy),
                  sd_1(p + e.yxy) - sd_1(p - e.yxy),
                  sd_1(p + e.yyx) - sd_1(p - e.yyx));
    return normalize(n);
}

vec3 ambient_1(vec3 p)
{
    p *= 0.34; 
    vec2 vn1 = vnoise2(p);
    vec2 vn2 = vnoise2(2.03 * p);
    vec2 vn3 = vnoise2(4.39 * p);
    vec2 vn4 = vnoise2(8.97 * p);

    return
        vn1.x * vn1.y * vec3(0.754, 0.414, 0.047) + 
        vn2.x * vn2.y * vec3(0.554, 0.274, 0.037) + 
        vn3.x * vn3.y * vec3(0.417, 0.261, 0.012) + 
        vn4.x * vn4.y * vec3(0.331, 0.123, 0.008);    
}

vec3 raymarch_1(vec3 position, vec3 direction)
{
    const vec3 fog_color = vec3(0.3751f, 0.1713f, 0.071f);
    const vec3 specular_color = vec3(0.707f);
    const float epsilon = 0.0125f;
    const int maxSteps = 128;
    float t = sd_1(position);
    int i = 0;
    while(i < maxSteps) 
    {
        float sd = sd_1(position + direction * t);
        if(abs(sd) < epsilon)
        {
            vec3 ipoint = position + t * direction;                                                     // intersection point
            float q = exp(-0.025 * i);                                                                  // simplest possible ambient occlusion term
    
            vec3 n = grad_1(ipoint);
            vec3 ambient = ambient_1(ipoint);                                                           // procedural color

            const vec3 l = normalize(vec3(0.707, 0.707, 0.0));                                          // fixed diagonal light direction
            float cos_theta = dot(n, l);
            float lambert_factor = 0.25f * (1 + cos_theta) * (1 + cos_theta) * (2.0 - cos_theta);        // = smoothstep(-1, 1, cos_theta) to leave back faces a bit lit
            
            vec3 h = normalize(l - direction);                                                          // specular term, Blinn - Phong lighting
            float cos_alpha = max(dot(h, n), 0.0f);
            return mix(lambert_factor * ambient + pow(cos_alpha, 40.0) * specular_color, fog_color, q);

        }
        t += sd;
        if(t > 500.0f) break;
        ++i;
    }
    return fog_color;
}

//==============================================================================================================================================================
// Scene 2 :: ocean
//==============================================================================================================================================================

const float PI = 3.1415926;
const float EPSILON = 1e-3;
float EPSILON_NRM   = 0.1 / 1920.0;

const int ITER_GEOMETRY = 3;
const int ITER_FRAGMENT = 5;
const float SEA_HEIGHT = 0.6;
const float SEA_CHOPPY = 4.0;
const float SEA_SPEED = 0.8;
const float SEA_FREQ = 0.16;
const vec3 SEA_BASE = vec3(0.1, 0.19, 0.22);
const vec3 SEA_WATER_COLOR = vec3(0.8, 0.9, 0.6);
float SEA_TIME = time * SEA_SPEED;

float hash(vec2 p)
{
    float h = dot(p, vec2(127.1, 311.7)); 
    return fract(sin(h) * 43758.5453123);
}

float noise(vec2 p) 
{
    vec2 i = floor(p);
    vec2 f = fract(p);    
    vec2 u = f * f * (3.0 - 2.0 * f);
    return -1.0 + 2.0 * mix(mix(hash(i + vec2(0.0, 0.0)), hash(i + vec2(1.0, 0.0)), u.x),
                            mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

float diffuse(vec3 n, vec3 l, float p)
{
    return pow(dot(n,l) * 0.4 + 0.6, p);
}

float specular(vec3 n, vec3 l, vec3 e, float s)
{    
    float nrm = (s + 8.0) / (3.1415 * 8.0);
    return pow(max(dot(reflect(e, n), l), 0.0), s) * nrm;
}

vec3 getSkyColor(vec3 e)
{
    e.y = max(e.y, 0.0);
    vec3 ret;
    ret.x = pow(1.0 - e.y, 2.0);
    ret.y = 1.0 - e.y;
    ret.z = 1.0 - 0.4 * e.y;
    return ret;
}

float sea_octave(vec2 uv, float choppy)
{
    uv += noise(uv);        
    vec2 wv = 1.0 - abs(sin(uv));
    vec2 swv = abs(cos(uv));    
    wv = mix(wv, swv, wv);
    return pow(1.0 - pow(wv.x * wv.y, 0.65), choppy);
}

const mat2 octave_matrix = mat2(1.6, 1.2, -1.2, 1.6);

float map2(vec3 p)
{
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    float h = 0.0;    
    for(int i = 0; i < ITER_GEOMETRY; i++)
    { 
        float d = sea_octave((uv + SEA_TIME) * freq, choppy) + sea_octave((uv - SEA_TIME) * freq, choppy);
        h += d * amp;        
        uv *= octave_matrix;
        freq *= 1.9;
        amp *= 0.22;
        choppy = mix(choppy, 1.0, 0.2);
    }
    return p.y - h;
}

float map_detailed(vec3 p)
{
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_FRAGMENT; i++)
    { 
        float d = sea_octave((uv + SEA_TIME) * freq, choppy) + sea_octave((uv - SEA_TIME) * freq, choppy);
        h += d * amp;        
        uv *= octave_matrix; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy, 1.0, 0.2);
    }
    return p.y - h;
}

vec3 getSeaColor(vec3 p, vec3 n, vec3 l, vec3 eye, float t)
{  
    float fresnel = 1.0 - max(dot(n, -eye), 0.0);
    fresnel = pow(fresnel, 3.0) * 0.65;
        
    vec3 reflected = getSkyColor(reflect(eye, n));    
    vec3 refracted = SEA_BASE + diffuse(n, l, 80.0) * SEA_WATER_COLOR * 0.12; 
    
    vec3 color = mix(refracted, reflected, fresnel);
    
    float atten = max(1.0 - 0.001 * t * t, 0.0);
    color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;
    color += vec3(specular(n,l,eye,60.0));
    return color;
}

vec3 getNormal(vec3 p, float eps)
{
    vec3 n;
    n.y = map_detailed(p);    
    n.x = map_detailed(vec3(p.x + eps, p.y, p.z)) - n.y;
    n.z = map_detailed(vec3(p.x, p.y, p.z + eps)) - n.y;
    n.y = eps;
    return normalize(n);
}

float heightMapTracing(vec3 ori, vec3 dir)
{  
    float tm = 0.0;
    float tx = 1000.0;
    float hx = map2(ori + dir * tx);
    if (hx > 0.0) return tx;   
    float hm = map2(ori + dir * tm);    
    float tmid;
    for(int i = 0; i < 8; i++)
    {
        tmid = mix(tm, tx, hm / (hm - hx));                   
        float hmid = map2(ori + dir * tmid);
        if(hmid < 0.0)
        {
            tx = tmid;
            hx = hmid;
        }
        else
        {
            tm = tmid;
            hm = hmid;
        }
    }
    return tmid;
}

//==============================================================================================================================================================
// Scene 3 :: crystal
//==============================================================================================================================================================

vec2 csqr(vec2 a )
{
    return vec2(a.x * a.x - a.y * a.y, 2.0f * a.x * a.y); 
}


vec2 iSphere(vec3 ro, vec3 rd, float radius)
{
    float b = dot(ro, rd);
    float c = dot(ro, ro) - radius * radius;
    float h = b * b - c;
    if(h < 0.0f) return vec2(-1.0f);
    h = sqrt(h);
    return vec2(-b - h, -b + h);
}

float map(in vec3 p) 
{
    float res = 0.0f;    
    vec3 c = p;
    for (int i = 0; i < 8; ++i) 
    {
        p = 0.7f * abs(p + cos(time * 0.15f + 1.6f) * 0.15f) / dot(p, p) - 0.7f + cos(time * 0.15f) * 0.15f;
        p.yz = csqr(p.yz);
        p = p.zxy;
        res += exp(-19.0f * abs(dot(p,c)));
        
    }
    return 0.5f * res;
}

vec3 raymarch(in vec3 ro, vec3 rd, vec2 tminmax)
{
    ro *= 0.25;
    tminmax *= 0.25;
    float t = tminmax.x;
    //float dt = 0.1f;
    float dt = 0.1f - 0.075f * cos(time * 0.025f);                                      // animated
    vec3 col= vec3(0.0f);
    float c = 0.0f;
    for(int i = 0; i < 32; i++)
    {
        t += dt * exp(-2.0f * c);
        if(t > tminmax.y)
            break;        
        c = map(ro + t * rd);                       
        col = 0.99f * col + 0.08f * vec3(c * c + c, c, c * c * c);                          // blue
    }    
    return col;
}

void main()
{
    vec3 direction = normalize(view);

    // Scene 1 :: smooth infinite periodic lattice
    // vec3 color = raymarch_1(position, direction);

    // Scene 2 :: ocean
    /*
    vec3 p;
    heightMapTracing(position, direction, p);
    vec3 dist = p - position;
    vec3 n = getNormal(p, dot(dist, dist) * EPSILON_NRM);
    vec3 light = normalize(vec3(0.0, 1.0, 0.6));              
    vec3 color = mix(getSkyColor(direction), getSeaColor(p, n, light, direction, dist), pow(smoothstep(0.0, -0.05, direction.y), 0.3));
    color = pow(color, vec3(0.95));
    */

    // Scene 3 :: crystal

    vec3 ro = position;
    vec3 rd = direction;
    vec2 tmm = iSphere(position, direction, 8.0);

    // raymarch
    const vec3 ENV_COLOR = SEA_WATER_COLOR;
    vec3 color1 = raymarch(position, direction, tmm);
    if (tmm.x >= 0.0f)
    {
        vec3 normal = 0.5f * (position + tmm.x * direction);
        normal = reflect(direction, normal);        
        float fre = pow(0.5f + clamp(dot(normal, direction), 0.0f, 1.0f), 3.0f) * 1.3f;
        color1 += ENV_COLOR * fre;                     

    }
    else
        color1 = ENV_COLOR;
    color1 = 0.5f * log(1.0f + color1);                     // normalize
    color1 = clamp(color1, 0.0f, 1.0f);





    
    float t = heightMapTracing(position, direction);
    vec3 p = position + t * direction;
    vec3 n = getNormal(p, t * t * EPSILON_NRM);
    vec3 light = normalize(vec3(0.0, 1.0, 0.6));
    vec3 sky_color = getSkyColor(direction);
    vec3 sea_color = getSeaColor(p, n, light, direction, t);         
    vec3 color2 = mix(sky_color, sea_color, pow(smoothstep(0.0, -0.05, direction.y), 0.3));
    color2 = pow(color2, vec3(0.95));

    vec3 color;
    if (tmm.x >= 0)
    {
        // sphere is intersected
        if (t < tmm.x)
            color = mix(color1, color2, 0.9);
        else
            color = color1;
            
    }
    else
        color = color2;






    FragmentColor = vec4(color, 1.0);

}