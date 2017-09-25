#version 330 core

in vec2 uv;
in vec3 view_cs;
in vec3 view_ws;

uniform float time;
uniform float z_near;
uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform vec2 focal_scale;
uniform float camera_shift;

uniform sampler2D value_tex;
uniform sampler2D stone_tex;
uniform sampler2D depth_tex;

uniform mat3 view_matrix;

out vec4 FragmentColor;

const float pi = 3.14159265359;
const float HORIZON = 200.0;
const vec3 rgb_power = vec3(0.299, 0.587, 0.114);

const int MAX_TRACE_ITER = 128;
const int MAX_AO1_ITER = 8;
const int MAX_AO2_ITER = 8;

//==============================================================================================================================================================
// 3D value noise function
//==============================================================================================================================================================
#define TEXEL_SIZE 1.0 / 256.0
#define HALF_TEXEL 1.0 / 512.0

float hermite5(float x)
    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

vec3 hermite5(vec3 x)
    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

float hermite5(float a, float b, float x)
{
    float y = clamp((x - a) / (b - a), 0.0, 1.0);
    return y * y * y * (10.0 + y * (6.0 * y - 15.0));
}

float vnoise(vec3 x)
{
    vec3 p = floor(x);
    vec3 f = x - p;
    f = hermite5(f);
    vec2 uv0 = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg = texture(value_tex, TEXEL_SIZE * uv0 + HALF_TEXEL).rg;
    return mix(rg.g, rg.r, f.z);
}

//==============================================================================================================================================================
// nvidia trilinear-blend texture
//==============================================================================================================================================================
vec3 tex3d(vec3 p, vec3 n)
{
    n = max(abs(n) - 0.35f, 0.0f);
    n /= dot(n, vec3(1.0f));
    vec3 tx = texture(stone_tex, p.zy).xyz;
    vec3 ty = texture(stone_tex, p.xz).xyz;
    vec3 tz = texture(stone_tex, p.xy).xyz;
    vec3 c = tx * tx * n.x + ty * ty * n.y + tz * tz * n.z;
    return pow(c, vec3(0.66));
}

//==============================================================================================================================================================
// canyon distance function
//==============================================================================================================================================================
vec3 tri(in vec3 x)
{
    vec3 q = abs(fract(x) - 0.5f);
    return q;
}

float sdf(vec3 p)
{    
    const float ground_level = -1.0f;
    vec3 op = tri(1.1f * p + tri(1.1f * p.zxy));
    float ground = p.z - ground_level + dot(op, vec3(0.067));
    p += (op - 0.25) * 0.3;
    p = cos(0.444f * p + sin(1.112f * p.zxy));
    float canyon = (length(p) - 1.05) * 0.95;
    return 0.461 * min(ground, canyon);
}

//==============================================================================================================================================================
// gradient, normal and directional derivative
//==============================================================================================================================================================
const float sigma = 0.015625;                                       // 1 / 64
//const float sigma = 0.0078125;                                  // 1 / 128
const vec2 delta = vec2(sigma, -sigma);

const float inv_derivative_factor = 0.5f / sigma;
const float inv_grad_factor = 0.25f / (sigma * sigma);

vec3 grad(vec3 p)                                                   // tetrahedral evaluation
{
    return inv_grad_factor * (delta.xyy * sdf(p + delta.xyy) 
                            + delta.yyx * sdf(p + delta.yyx) 
                            + delta.yxy * sdf(p + delta.yxy) 
                            + delta.xxx * sdf(p + delta.xxx));
}

vec3 calc_normal(vec3 p)                                            // tetrahedral evaluation
{
    return normalize(delta.xyy * sdf(p + delta.xyy) 
                   + delta.yyx * sdf(p + delta.yyx) 
                   + delta.yxy * sdf(p + delta.yxy) 
                   + delta.xxx * sdf(p + delta.xxx));
}

float directional_derivative(vec3 p, vec3 v)                        // symmetric difference
{
    return inv_derivative_factor * (sdf(p + sigma * v) - sdf(p - sigma * v));
}

//==============================================================================================================================================================
// bumped normal calculation
//==============================================================================================================================================================
vec3 bump_normal(vec3 p, vec3 n, float bf)
{
    const vec2 e = vec2(0.0025, 0);
    mat3 m = mat3(tex3d(p - e.xyy, n),
                  tex3d(p - e.yxy, n),
                  tex3d(p - e.yyx, n));
    vec3 g = rgb_power * m;
    g = (g - dot(tex3d(p, n), rgb_power)) / e.x;
    return normalize(n + g * bf);
}

//==============================================================================================================================================================
// ambient occlusion, ver 1
//==============================================================================================================================================================
float calc_ao1(in vec3 p, in vec3 n)
{    
    const float magnitude_factor = 0.71f;
    const float hstep = 0.125f;

    float magnitude = 2.37f * (1.0f - magnitude_factor);

    float occlusion = 0.0f;
    float h = 0.01;

    for (int i = 0; i < MAX_AO1_ITER; ++i)
    {
        float d = sdf(p + h * n);
        occlusion += magnitude * (h - d);
        magnitude *= magnitude_factor;
        h += hstep;
    }

    return clamp(1.0f - occlusion, 0.0f, 1.0f);    
}

//==============================================================================================================================================================
// ambient occlusion, ver 2
//==============================================================================================================================================================
float calc_ao2(in vec3 p, in vec3 n)
{
    const float max_h = 3.75f;
    const float hstep = max_h / MAX_AO2_ITER;

    float occlusion = 0.0f;
    float h = hstep;

    for(int i = 0; i < MAX_AO2_ITER; ++i)
    {
        occlusion += (h - sdf(p + h * n)) / (0.5f + 1.0f * h);
        h += hstep;
    }

    occlusion /= MAX_AO2_ITER;
    return clamp(1.0f - occlusion, 0.0f, 1.0f);
}

//==============================================================================================================================================================
// shadows calculation
//==============================================================================================================================================================
float smin2(float a, float b, float k)
{
    float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
    return mix(b, a, h) - k * h * (1.0f - h);
}

float hard_shadow_factor(in vec3 p, in vec3 l, in float min_t, in float max_t)
{
    float t = max_t;
    while (t > min_t)
    {
        float h = sdf(p + t * l);
        if (h < 0.001) return 0.0f;
        t -= h;
    }
    return 1.0f;
}

float soft_shadow_factor(in vec3 p, in vec3 n, in vec3 l, in float d)
{
    float dp = dot(n, l);
    if (dp < 0.0f) return 0.0f;

    float t = d;
    float f = 1.0f;

    while (t > 0.05f)
    {
        float h = sdf(p + t * l);
        if (h < 0.00125f) return 0.0f;
        f = min(f, hermite5(4.0f * h / t));
        t = t - clamp(0.0f, 0.7f, h);
    }

    return hermite5(dp) * f / (f + 0.125f);
}

//==============================================================================================================================================================
// pseudo environment mapping...
//==============================================================================================================================================================
vec3 envMap(vec3 rd, vec3 n)
{    
    return tex3d(rd, n);
}

//==============================================================================================================================================================
// basic raymarching loop
//==============================================================================================================================================================

float trace(vec3 p, vec3 v, float t0)
{    
    float t = t0;
    for(int i = 0; i < MAX_TRACE_ITER; i++)
    {    
        float d = sdf(p + t * v);
        float d_abs = abs(d);
        if(d_abs < (0.001 + 0.00025 * t) || t > HORIZON) break;
        t += d;
    }
    return min(t, HORIZON);
}

float lower_bound(vec2 pq, float lambda)
{
    const vec4 offset = vec4(1.0 / 1920.0, 1.0 / 1080.0, -1.0 / 1920.0, -1.0 / 1080.0);

    vec4 d0 = vec4(texture(depth_tex, lambda * pq + offset.xy).x,
                   texture(depth_tex, lambda * pq + offset.xw).x,
                   texture(depth_tex, lambda * pq + offset.zy).x,
                   texture(depth_tex, lambda * pq + offset.zw).x);

    vec4 z_aff = z_near / (1.0 - d0);
    z_aff.xy = min(z_aff.xy, z_aff.zw);
    return min(z_aff.x, z_aff.y);
}


vec3 shade(vec3 p0, vec3 n, vec3 v, float t)
{
    vec3 p = p0 + v * t;                                                            // fragment position in world space
    vec3 b = bump_normal(p, n, 0.0175);                                             // bumped normal, third parameter is bump factor
    vec3 l = light_ws - p;                                                          // direction from fragment to light source
    float d = length(l);                                                            // distance from fragment to light
    l /= d;                                                                         // normalized light direction

//    float sf = soft_shadow_factor(p, b, l, d);                                      // soft shadow factor
//    float sf = hard_shadow_factor(p, b, l, d);                                      // hard shadow factor
//    float ao = calc_ao1(p, b);                                                      // ambient occlusion factor
//    float ao = calc_ao2(p, b);                                                      // ambient occlusion factor : second option

    float diffuse = 0.5 + 0.5 * max(dot(b, l), 0.0);                                            // diffuse lighting factor
    float specular = pow(max(dot(reflect(-l, b), -v), 0.0), 32.0);                  // specular lighting factor
    float fresnel = pow(clamp(dot(b, v) + 1.0, 0.0, 1.0), 1.0);                     // fresnel lighting factor for reflective glow
    float attenuation = 1.0 / (1.0 + d * 0.007);                                    // light attenuation, based on the light distance

    vec3 diffuse_color = tex3d(p, n);                                               // trilinear blended color from input texture

    vec3 color = diffuse_color;
    return color;
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
/*
    vec3 v = normalize(view_ws);
    vec3 p = camera_ws;
    float t = z_near;

    const int MAX_MARCH_ITERATIONS = 256;
    const float MAX_MARCH_DISTANCE = 200.0f;
    const float ALPHA_THRESHOLD = 0.00125;
    const float PIXEL_SIZE = 1.0 / 1080.0;
    const vec3 AMBIENT_COLOR = vec3(0.32, 0.17, 0.07);
*/

// o, d : ray origin, direction (normalized)
// t_min, t_max: minimum, maximum t values
// pixelRadius: radius of a pixel at t = 1
// forceHit: boolean enforcing to use the
//           candidate_t value as result
/*
    float omega = 1.97;
    float candidate_error = 100.0;
    float candidate_t = z_near;
    float previousRadius = 0.0f;
    float stepLength = 0.0f;

    for(int i = 0; i < MAX_MARCH_ITERATIONS; ++i)
    {
        float signedRadius = sdf(p + t * v);
        float radius = abs(signedRadius);
        bool sorFail = omega > 1 && (radius + previousRadius) < stepLength;
        if (sorFail)
        {
            stepLength -= omega * stepLength;
            omega = 1;
        }
        else
        {
            stepLength = signedRadius * omega;
        }
        previousRadius = radius;
        float error = radius / t;

        if (!sorFail && error < candidate_error)
        {
            candidate_t = t;
            candidate_error = error;
        }

        if (!sorFail && error < PIXEL_SIZE || t > MAX_MARCH_DISTANCE)
            break;
        t += stepLength;
    }
    t = candidate_t;

    vec3 n = calc_normal(p + t * v);
    vec3 c = shade(p, n, v, t);
    vec3 color = c;
*/
/*
    float d = sdf(p + t * v);

    vec3 acc_color = vec3(0.0);
    float acc_alpha = 1.0;

    for(int i = 0; i < MAX_MARCH_ITERATIONS; ++i)
    {
        float theta = PIXEL_SIZE_FACTOR * t;

        if(d < theta)
        {
            vec3 g = grad(p + t * v);
            float t_next = t + abs(d / dot(g, v));
            float d_next = sdf(p + t_next * v);

            if (d_next < PIXEL_SIZE_FACTOR * t_next)
            {
                //==============================================================================================================================================
                // at the next step we are going to get an invisible surface element
                // we can stop raymarching at this point
                //==============================================================================================================================================
                break;
            }
            else
            {
                //==============================================================================================================================================
                // surface element we just encountered only partially covers pixel area
                // lets shade it and accumulate its color, but continue marching
                // Since d_next < PIXEL_SIZE_FACTOR * t_next is not satisfied we can immediately do one more marching step
                //==============================================================================================================================================
                vec3 n = normalize(g);
                float cos_theta = dot(n, v);

                vec3 c = shade(p, n, v, t);
                float coverage = 1.0 - smoothstep(-theta, theta, d); //cos_theta * cos_theta;

                //FragmentColor = vec4(coverage, 0.0, 0.0, 1.0);
                //return;

                acc_color = acc_color + acc_alpha * coverage * c;
                acc_alpha = acc_alpha * (1 - coverage);

                t = t_next + abs(d_next);
                d = sdf(p + t * v);
                continue;
            }


        }

        t += abs(d);
        if (t > MAX_MARCH_DISTANCE)
            break;
        d = sdf(p + t * v);

    }

    vec3 n = calc_normal(p + t * v);
    vec3 c = shade(p, n, v, t);

    vec3 color = acc_color + acc_alpha * c;
*/


    vec3 v = normalize(view_ws);
    vec3 p = camera_ws;

    const int MAX_MARCH_ITERATIONS = 128;
    const float MAX_MARCH_DISTANCE = 200.0f;
    const float ALPHA_THRESHOLD = 0.00125;
    const float PIXEL_SIZE = 1.0 / 1080.0;
    const vec3 AMBIENT_COLOR = vec3(0.32, 0.17, 0.07);
    float gamma = 1.97;

    //==========================================================================================================================================================
    // the surface we are rendering is lsdf(x, eye) = sdf(x) - PIXEL_SIZE * dist(x, eye) = 0
    // the volume elements along the ray are classified as follows:
    // sdf(x) <= 0 -- the element is inside the surface, the marching loop can be stopped at this point
    // sdf(x) >= 2 * PIXEL_SIZE * dist(x, eye) -- the element is outside the surface, and its screen projection does not cover any of the pixel area
    // 0 < sdf(x) < 2 * PIXEL_SIZE * dist(x, eye) -- the element touches the surface and (possibly) partially covers the pixel area
    //==========================================================================================================================================================

    float t = z_near;
    float d = sdf(p + t * v);
    int it = 0;

    do
    {

        float delta1 = PIXEL_SIZE * t;
        float delta2 = 2.0f * delta1;




        if (d < delta2)
        {

            float hue = t / float(MAX_MARCH_DISTANCE);
            vec3 rgb = hsv2rgb(vec3(hue, 0.875, 0.875));
            FragmentColor = vec4(rgb, 1.0);
            return;

            // let us make one more iteration, 

            float coverage = 0.5f - 0.5f * (d - delta1) / delta1; // (1 to 0)
            // d = 0      : coverage = 1.0f;
            // d = delta2 : coverage = 0.0f;
        }

        //======================================================================================================================================================
        // at this point we have d > delta2, let us try to make big step and if it does not work then
        //======================================================================================================================================================
        float dt = gamma * d;
        float t1 = t + dt;
        float d1 = sdf(p + t1 * v);
        while (dt <= d + d1)
        {
            d = d1;
            t = t1;
            dt = gamma * d;
            t1 = t + dt;
            d1 = sdf(p + t1 * v);
        }

        //======================================================================================================================================================
        // at this point we know that last step failed, e.g  d + d1 < dt
        //======================================================================================================================================================



        t = t + max(d, 0.0025 * t);
        d = sdf(p + t * v);
        ++it;
    }
    while((d > 0) && (it < MAX_MARCH_ITERATIONS));

    vec3 n = calc_normal(p + t * v);
    vec3 c = shade(p, n, v, t);
    vec3 color = c;



    //==========================================================================================================================================================
    // view_cs is the view vector in camera space 
    // the location of the fragment in camera space is t * normalize(view_cs);
    // the projective value of fragment depth is thus 
    // z_proj = 1.0 + 2 * z_near / z_aff
    // the value in z-buffer is 0.5 + 0.5 * z_proj = 1.0 + z_near / z_aff
    //==========================================================================================================================================================
    vec3 position_cs = t * normalize(view_cs);
    float z_aff = position_cs.z;
    gl_FragDepth = 1.0 + z_near / z_aff;   

    FragmentColor = vec4(color, 1.0f);

}