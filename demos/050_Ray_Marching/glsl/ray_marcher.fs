#version 400 core

in vec2 uv;
in vec3 view;
flat in vec3 position;
uniform vec2 focal_scale;

uniform mat4 camera_matrix;
uniform float time;
uniform sampler2D value_texture;

out vec4 FragmentColor;

//==============================================================================================================================================================
// Hermite interpolants of degree 3, 5, and 7
//==============================================================================================================================================================

vec2 hermite3(vec2 x) { return x * x * (3.0 - 2.0 * x); }
vec3 hermite3(vec3 x) { return x * x * (3.0 - 2.0 * x); }

float hermite5(float x)   { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }
vec2  hermite5(vec2 x)    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }
vec3  hermite5(vec3 x)    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }
vec4  hermite5(vec4 x)    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

float hermite5_d(float x) { float q = x * (1.0 - x); return 30.0 * q * q; }
vec2  hermite5_d(vec2 x)  { vec2  q = x * (1.0 - x); return 30.0 * q * q; }
vec3  hermite5_d(vec3 x)  { vec3  q = x * (1.0 - x); return 30.0 * q * q; }
vec4  hermite5_d(vec4 x)  { vec4  q = x * (1.0 - x); return 30.0 * q * q; }

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

//===================================================================================================================================================================================================================
// 2D Value noise function with gradient
//===================================================================================================================================================================================================================
float vnoise_1(in vec2 x)
{
    vec2 p = floor(x);
    vec2 f = x - p;
    vec2 blend = hermite5(f);                                                               // interpolating coefficients
    vec4 v = textureGather(value_texture, TEXEL_SIZE * p + HALF_TEXEL, 0);                  // get 4 surrounding lattice values : v01, v11, v10, v00
    vec2 blend_h = mix(v.wx, v.zy, blend.x);                                                // horizontal blend : v.wx = (v00, v01), v.zy = (v10, v11)
    return mix(blend_h.x, blend_h.y, blend.y);                                              // vertical blend
}

float vnoise_1df(in vec2 x, out vec2 dF)
{
    vec2 p = floor(x);
    vec2 f = x - p;
    vec2 blend = hermite5(f);                                                               // interpolating coefficients
    vec2 blend_d = hermite5_d(f);                                                           // derivative of the interpolation polynomials
    vec4 v = textureGather(value_texture, TEXEL_SIZE * p + HALF_TEXEL, 0);                  // get 4 surrounding lattice values : v01, v11, v10, v00

    //===============================================================================================================================================================================================================
    // value = blend_y(blend_x(v00 + v10), blend_x(v01, v11))
    // d_x(value) = blend_y(v10 - v00, v11 - v01) * d_x(blend.x)
    // d_y(value) = blend_x(v01 - v00, v11 - v10) * d_y(blend.y)
    //===============================================================================================================================================================================================================

    dF = mix(v.zx - v.ww, v.yy - v.xz, blend.yx) * blend_d;                                 // v.zx = (v10, v01), v.ww = (v00, v00), v.yy = (v11, v11), v.xz = (v01, v10)
    vec2 blend_h = mix(v.wx, v.zy, blend.x);                                                // horizontal blend : v.wx = (v00, v01), v.zy = (v10, v11)
    return mix(blend_h.x, blend_h.y, blend.y);                                              // vertical blend
}

//===================================================================================================================================================================================================================
// 2D Simplex noise function
//===================================================================================================================================================================================================================
float snoise(in vec2 p)
{
    const float F2 = 0.36602540378443864676372317075294;                                    // (sqrt(3) - 1) / 2
    const float G2 = 0.21132486540518711774542560974902;                                    // (3 - sqrt(3)) / 6
    const float H2 = 0.57735026918962576450914878050196;                                    // 1 / sqrt(3)
    const float NORMALIZER = 94.0;

    const vec2 F = vec2(F2, NORMALIZER);
    const vec4 dp = vec4(G2, -H2, G2 - 1.0, 0.0);

    vec2 i = floor(p + dot(p, F.xx));                                                       // Skew the (x, y) space to determine which cell of 2 simplices we're in
    vec2 p0 = i - dot(i, dp.xx);                                                            // move the cell origin back to (x, y) space
    vec2 uv = TEXEL_SIZE * i + HALF_TEXEL;                                                  // integral part for texture lookup

    vec2 dp0 = p - p0;                                                                      // The (x, y) distances from the cell origin. For the 2D case, the simplex shape is an equilateral triangle.

    vec4 a = textureGather(value_texture, uv, 0) - 0.5;                                     // get 4 surrounding gradient x-components : a01, a11, a10, a00
    vec4 b = textureGather(value_texture, uv, 2) - 0.5;                                     // get 4 surrounding gradient y-components : b01, b11, b10, b00

    vec4 inv_norm = inversesqrt(a * a + b * b);

    vec4 dx = dp0.xxxx + dp;                                                                //    x + G2    : x - H2 : x + G2 - 1.0 : x
    vec4 dy = dp0.yyyy + dp.zyxw;                                                           // y + G2 - 1.0 : y - H2 :    y + G2    : y

    vec4 n = max(0.5 - dx * dx - dy * dy, 0.0);        
    n *= n; n *= n;

    vec4 v = n * (a * dx + b * dy);
    return NORMALIZER * dot(v, inv_norm);                                                   // sum up the contributions and scale the result to fit into [-1, 1]
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

mat3 rotationX (float angle)
{
    float cs = cos(angle);
    float sn = sin(angle);

    return mat3(vec3(1.0, 0.0, 0.0),
                vec3(0.0,  cs,  sn),
                vec3(0.0, -sn,  cs));    
}

mat3 rotationY(float angle)
{
    float cs = cos(angle);
    float sn = sin(angle);
    
    return mat3(vec3( cs, 0.0, -sn),
                vec3(0.0, 1.0, 0.0),
                vec3( sn, 0.0,  cs));
}

mat3 rotationZ(float angle)
{
    float cs = cos(angle);
    float sn = sin(angle);
    
    return mat3(vec3( cs, -sn, 0.0),
                vec3( sn,  cs, 0.0),
                vec3(0.0, 0.0, 1.0));
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

float vmin(vec3 v) { return min(v.x, min(v.y, v.z)); }
float vmax(vec3 v) { return max(v.x, max(v.y, v.z)); }

float smin_exp(float a, float b, float k) { return -log(exp(-k * a) + exp(-k * b)) / k; }
float smax_exp(float a, float b, float k) { return  log(exp( k * a) + exp( k * b)) / k; }


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
// Ocean rendering functions
//==============================================================================================================================================================
const float OCEAN_BASE_AMPLITUDE = 0.6;
const vec3 FOG_COLOR = vec3(0.91f, 0.95f, 0.97f);
const float FOG_RADIUS = 1536.0;

float noise(vec2 P) 
{
    const float FACTOR_X = 127;
    const float FACTOR_Y = 311;
    const vec4 hash = vec4(0, FACTOR_X, FACTOR_Y, FACTOR_X + FACTOR_Y);

    vec2 Pi = floor(P);
    vec2 Pf = P - Pi;

    vec4 h = dot(Pi, hash.yz) + hash;
    vec2 Ps = hermite3(Pf);
    h = fract(sin(h) * 43758.5453123);

    vec2 val = mix(h.xy, h.zw, Ps.y);
    return -1.0 + 2.0 * mix(val.x, val.y, Ps.x);
}

vec3 sky_color(vec3 e)
{
    const mat2 octave_matrix = mat2(1.272, 1.696, -1.696, 1.272);

    const vec3 CLOUD_COLOR = vec3(0.81, 0.89, 0.97);
    vec2 uv = e.xy / (abs(e.z) + 0.001);
    uv *= 2.313;
    uv += vec2(0.01527, 0.00948) * time;

    float fbm = noise(uv);

    float amplutide = 1.0;
    for (int i = 0; i < 6; ++i)
    {
        uv *= octave_matrix;
        amplutide *= 0.45f;
        fbm += amplutide * noise(uv);
    }

    float cloud_factor = smoothstep(0.0, 1.3, fbm) * exp(-0.00125 * length(uv));

    float q = abs(e.z);
    vec3 qq = vec3(1.0f - q, 1.0f - q, 1.0f - 0.5 * q);

    return pow(FOG_COLOR * qq, vec3(2.0f, 1.5f, 1.0f)) + cloud_factor * CLOUD_COLOR;
}

float ocean_level(vec2 uv, int lod)
{
    const mat2 octave_matrix = mat2(1.6, 1.2, -1.2, 1.6);
    float frequency = 0.16;
    float amplitude = OCEAN_BASE_AMPLITUDE;
    float sharpness = 4.0;
    
    float level = 0.0;
    for(int i = 0; i < lod; i++)
    {
        vec2 uv1 = (uv + 0.8 * time) * frequency;
        vec2 uv2 = (uv - 0.8 * time) * frequency;

        vec4 uv0 = vec4(uv1 + noise(uv1), uv2 + noise(uv2));
        vec4 wv = 1.0f - abs(sin(uv0));
        vec4 swv = abs(cos(uv0));
        wv = mix(wv, swv, wv);

        float d = pow(1.0f - pow(wv.x * wv.y, 0.65f), sharpness) + 
                  pow(1.0f - pow(wv.z * wv.w, 0.65f), sharpness);

        level += d * amplitude;        
        uv *= octave_matrix;
        frequency *= 1.9;
        amplitude *= 0.22;
        sharpness = mix(sharpness, 1.0, 0.2);
    }
    return level;
}

const vec3 light = normalize(vec3(0.0, 0.707, 0.707)); 

vec3 ocean_color(vec3 p, vec3 n, vec3 e, float t)
{  
    const vec3 OCEAN_DEPTH_COLOR = vec3(0.032f, 0.113f, 0.138f);
    const vec3 OCEAN_WATER_COLOR = vec3(0.247f, 0.317f, 0.296f);
    const vec3 OCEAN_SPECULAR_COLOR = vec3(0.79f, 0.91f, 0.93f);

    //==========================================================================================================================================================
    // reflected sky color
    //==========================================================================================================================================================
    float dp = dot(e, n);
    vec3 l = e - 2.0 * dp * n;						// direction of the reflected light coming to the sky
    vec3 reflected_sky = sky_color(l);

    //==========================================================================================================================================================
    // refracted water color
    //==========================================================================================================================================================
	dp = min(dp, 0.0f);
    const float eta = 0.75f;									// air - water refraction coefficient
    float ss = 1.0f - dp * dp;									// sine squared of the incidence angle
    float k = 1.0f - ss * eta * eta;  							// sine squared of the refraction angle
    float g = sqrt(k) / eta;									// fresnel koefficient
    float f = 0.5 * pow((g + dp) / (g - dp), 2.0f) * (1.0 + pow((dp * (g - dp) + 1.0f) / (dp * (g + dp) - 1.0f), 2.0f));

    //==========================================================================================================================================================
    // the larger the angle between normal and refracted ray, the lighter the water 
    //==========================================================================================================================================================
    float lambda = pow(smoothstep(1.0f - eta * eta, 1.0, k), 0.36);
    vec3 refracted_water = mix(OCEAN_WATER_COLOR, OCEAN_DEPTH_COLOR, lambda);
    vec3 color = mix(refracted_water, reflected_sky, f);
    
    //==========================================================================================================================================================
    // attenuate with exponential decay water above average ocean level 
    //==========================================================================================================================================================
    float attenuation = 0.427 * exp(-abs(0.07 * t));
    color += OCEAN_WATER_COLOR * (p.z - OCEAN_BASE_AMPLITUDE) * attenuation;

    //==========================================================================================================================================================
    // add specular component : standard Phong term
    //==========================================================================================================================================================
    float specular_power = 0.727f * pow(max(dot(l, light), 0.0), 80.0);
    color += specular_power * OCEAN_SPECULAR_COLOR;
    return color;
}

vec3 ocean_normal(vec2 uv, float t)
{
    float delta = 0.125 / 2048.0 * t * t;
    vec3 n = vec3(
                ocean_level(vec2(uv.x - delta, uv.y), 5) - ocean_level(vec2(uv.x + delta, uv.y), 5),
                ocean_level(vec2(uv.x, uv.y - delta), 5) - ocean_level(vec2(uv.x, uv.y + delta), 5),
                2.0 * delta
             );
    return normalize(n);
}

float heightmap_trace(vec3 position, vec3 direction, out vec3 p)
{
    const int TRACE_LEVEL = 4;
    float tmax = FOG_RADIUS;
    p = position + direction * tmax;
    float hmax = p.z - ocean_level(p.xy, TRACE_LEVEL);
    if (hmax > 0.0) return tmax;

    float tmin = 0.0;
    float hmin = position.z - ocean_level(position.xy, TRACE_LEVEL);
    float t;
    for(int i = 0; i < 8; i++)
    {
        t = mix(tmin, tmax, hmin / (hmin - hmax));                   
        p = position + direction * t;
        float h = p.z - ocean_level(p.xy, TRACE_LEVEL);
        if(h < 0.0)
        {
            tmax = t;
            hmax = h;
        }
        else
        {
            tmin = t;
            hmin = h;
        }
    }
    return t;
}

//==============================================================================================================================================================
// Rendering glass bulb
//==============================================================================================================================================================
const float EPSY = 0.001;

float sd_bulb(vec3 p)
{
    return length(p - vec3(0.0, 0.0, 9.0)) - 8.3;
}

vec3 bulb_normal(vec3 p)
{
    vec2 dp = vec2(0.00625f, 0.0f);
    vec3 q = vec3(sd_bulb(p + dp.xyy) - sd_bulb(p - dp.xyy), 
                  sd_bulb(p + dp.yxy) - sd_bulb(p - dp.yxy), 
                  sd_bulb(p + dp.yyx) - sd_bulb(p - dp.yyx));
    return normalize(q);
}

vec3 bulb_march(vec3 position, vec3 direction)
{
    const int MAX_STEPS = 128;
    for(int i = 0; i < MAX_STEPS; i++)
    {
        float sd = sd_bulb(position);
        if(sd < EPSY)
            break;
        else
            position += sd * direction;
    }
    return position;
}

vec3 ocean_trace(vec3 position, vec3 direction)
{
    vec3 p;
    float t = heightmap_trace(position, direction, p);
    vec3 n = ocean_normal(p.xy, t);
    float dh = 0.025 * exp(-abs(position.z));
    float fog_factor = 1.0 - exp(-0.525 * abs(t) / FOG_RADIUS);
    vec3 water = (t > FOG_RADIUS) ? FOG_COLOR : mix(ocean_color(p, n, direction, t), FOG_COLOR, fog_factor);
    vec3 sky = sky_color(direction);
    vec3 color = mix(water, sky, smoothstep(0.0, -0.005, -direction.z));
    return color;
}

void main()
{
    vec3 direction = normalize(view);

    vec3 p = bulb_march(position, direction);
    vec3 color_g;

    if(sd_bulb(p) < EPSY)
    {
        vec3 n = bulb_normal(p);

        //======================================================================================================================================================
        // reflected environment color
        //======================================================================================================================================================
        float dp = dot(direction, n);
        vec3 l = direction - 2.0 * dp * n;
        vec3 reflected_env = ocean_trace(p, l);

        //======================================================================================================================================================
        // refracted environment color
        //======================================================================================================================================================
        dp = min(dp, 0.0f);
        const float eta = 0.75f;                                    // air - crystal refraction coefficient
        float ss = 1.0f - dp * dp;                                  // sine squared of the incidence angle
        float k = 1.0f - ss * eta * eta;                            // sine squared of the refraction angle
        float g = sqrt(k) / eta;                                    // fresnel koefficient
        float f = 0.5 * pow((g + dp) / (g - dp), 2.0f) * (1.0 + pow((dp * (g - dp) + 1.0f) / (dp * (g + dp) - 1.0f), 2.0f));
        vec3 r = refract(direction, n, eta);
        vec3 refracted_env = ocean_trace(p, r);

        //======================================================================================================================================================
        // mix reflected and refracted light
        //======================================================================================================================================================
        color_g = mix(reflected_env, refracted_env, sqrt(sqrt(f)));

        //======================================================================================================================================================
        // add specular
        //======================================================================================================================================================
        float specular_power = 1.769f * pow(max(dot(l, light), 0.0), 100.0);
        color_g *= vec3(0.651, 0.547, 1.439);
        color_g += specular_power * vec3(0.5, 0.5, 1.0);

        if (-dp < 0.175)                                              // ball antialiasing
        {
            vec3 ocean_background = ocean_trace(position, direction);
            color_g = mix(ocean_background, color_g, smoothstep(0.0, 0.175, -dp));
        }
    }
    else
        color_g = ocean_trace(position, direction);

    FragmentColor = vec4(pow(color_g, vec3(0.75)), 1.0);
}