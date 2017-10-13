#version 430 core
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

//==============================================================================================================================================================
// Predefined compute shader inputs :: 
//  const uvec3 gl_WorkGroupSize      = uvec3(local_size_x, local_size_y, local_size_z)
//  in uvec3 gl_NumWorkGroups         ----- the number of work groups passed to the dispatch function
//  in uvec3 gl_WorkGroupID           ----- the current work group for this shader invocation
//  in uvec3 gl_LocalInvocationID     ----- the current invocation of the shader within the work group
//  in uvec3 gl_GlobalInvocationID    ----- unique identifier of this invocation of the compute shader among all invocations of this compute dispatch call
//==============================================================================================================================================================

layout (rgba32f, binding = 0) uniform image2D scene_image;

uniform vec2 inv_res;
uniform mat3 camera_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform vec2 focal_scale;                                                   // camera focal scale
uniform float pixel_size;                                                   // NDC / pixel uv - ratio
uniform float z_near;                                                       // distance to near z-plane

uniform sampler2D tb_tex;
uniform sampler2D bump_tb_tex;
uniform sampler2D value_tex;

uniform int split_screen;

//==============================================================================================================================================================
// 3D Value noise function
//==============================================================================================================================================================

#define VALUE_NOISE_TEXEL_SIZE 1.0 / 256.0
#define VALUE_NOISE_HALF_TEXEL 1.0 / 512.0

vec3 hermite5(vec3 x)
    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

float vnoise(vec3 x)
{
    vec3 p = floor(x);
    vec3 f = x - p;
    f = hermite5(f);
    vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg = texture(value_tex, VALUE_NOISE_TEXEL_SIZE * uv + VALUE_NOISE_HALF_TEXEL).rg;
    return mix(rg.g, rg.r, f.z);
}

const float pi = 3.14159265359;
const vec3 RGB_SPECTRAL_POWER = vec3(0.299, 0.587, 0.114);

//==============================================================================================================================================================
// trilinear blend texture
//==============================================================================================================================================================
const float NORMAL_CLAMP = 0.37f;
const float TEX_SCALE = 0.15;

vec3 tex3D_AA(vec3 q, vec3 dq_dx, vec3 dq_dy, vec3 n)
{
    vec3 w = pow(abs(n), vec3(4.8));
    w /= (w.x + w.y + w.z);

    vec3 tx = textureGrad(tb_tex, q.yz, dq_dx.yz, dq_dy.yz).rgb;
    vec3 ty = textureGrad(tb_tex, q.zx, dq_dx.zx, dq_dy.zx).rgb;
    vec3 tz = textureGrad(tb_tex, q.xy, dq_dx.xy, dq_dy.xy).rgb;
    return tx * tx * w.x + ty * ty * w.y + tz * tz * w.z;
}

vec3 bump3D_AA(vec3 q, vec3 dq_dx, vec3 dq_dy, vec3 n)
{
    vec3 w = n;
    vec3 tx = textureGrad(bump_tb_tex, q.yz, dq_dx.yz, dq_dy.yz).zxy;
    vec3 ty = textureGrad(bump_tb_tex, q.zx, dq_dx.zx, dq_dy.zx).yzx;
    vec3 tz = textureGrad(bump_tb_tex, q.xy, dq_dx.xy, dq_dy.xy).xyz;
    vec3 b = tx * w.x + ty * w.y + tz * w.z;
    b = normalize(b);
    return b;
}

//==============================================================================================================================================================
// canyon signed distance function
//==============================================================================================================================================================
vec3 tri_sharp(vec3 p)
{
    vec3 y = abs(fract(p) - 0.5);
    return y;
}

vec3 tri_smooth(vec3 p)
{
    const float k = 0.04;
    vec3 y = abs(fract(p) - 0.5);
    vec3 z = clamp(y, k, 0.5 - k);
    return z;
}

float sdf(vec3 p)
{
    const float SDF_SCALE = 3.0;
    const float INV_SDF_SCALE = 1.0 / SDF_SCALE;
    p *= INV_SDF_SCALE;
    vec3 op = tri_smooth(1.1f * p + tri_smooth(1.1f * p.zxy));
    p += 0.317 * (op - 0.25);
    p = cos(0.444f * p + sin(1.112f * p.zxy));
    float canyon = 0.941 * (length(p) - 1.05);
    return SDF_SCALE * canyon;
}

/*
float sdf(vec3 p)
{
    const float SDF_SCALE = 5.0;
    const float INV_SDF_SCALE = 1.0 / SDF_SCALE;
    p *= INV_SDF_SCALE;
    const float G2 = 0.211324865f;
    p.xy += dot(p.xy, vec2(G2));
    vec3 op = tri_smooth(1.1f * p + tri_smooth(1.1f * p.zxy));
    float ground = 0.77 * p.z + 0.5 + 0.77 * dot(op, vec3(0.067));
    p += 0.45 * (op - 0.25);
    p = cos(0.444f * p + sin(1.112f * p.zxy));
    float canyon = 0.34865 * (length(p) - 1.05);
    return SDF_SCALE * min(ground, canyon);
}
*/

//==============================================================================================================================================================
// ambient occlusion, ver 1
//==============================================================================================================================================================
float calc_ao1(in vec3 p, in vec3 n)
{    
    const int NB_ITER = 8;

    const float magnitude_factor = 0.71f;
    const float hstep = 0.125f;

    float magnitude = 2.37f * (1.0f - magnitude_factor);

    float occlusion = 0.0f;
    float h = 0.01;

    for (int i = 0; i < NB_ITER; ++i)
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
    const int NB_ITER = 8;

    const float max_h = 3.75f;
    const float hstep = max_h / NB_ITER;

    float occlusion = 0.0f;
    float h = hstep;

    for(int i = 0; i < NB_ITER; ++i)
    {
        occlusion += (h - sdf(p + h * n)) / (0.5f + 1.0f * h);
        h += hstep;
    }

    occlusion /= NB_ITER;
    return clamp(1.0f - occlusion, 0.0f, 1.0f);
}

//==============================================================================================================================================================
// tetrahedral normal and bumped normal
//==============================================================================================================================================================
const float NORMAL_DERIVATIVE_SCALE = 0.014117;
const float BUMP_DERIVATIVE_SCALE = 0.002;
const float BUMP_FACTOR = 0.00625771f;

vec3 normal(vec3 p)
{
    float scale = NORMAL_DERIVATIVE_SCALE;
    vec2 delta = vec2(scale, -scale); 
    return normalize(delta.xyy * sdf(p + delta.xyy) 
                   + delta.yyx * sdf(p + delta.yyx) 
                   + delta.yxy * sdf(p + delta.yxy) 
                   + delta.xxx * sdf(p + delta.xxx));
}

vec3 normal_AA(vec3 p, float t)
{
    float scale = NORMAL_DERIVATIVE_SCALE * t;
    vec2 delta = vec2(scale, -scale); 
    return normalize(delta.xyy * sdf(p + delta.xyy) 
                   + delta.yyx * sdf(p + delta.yyx) 
                   + delta.yxy * sdf(p + delta.yxy) 
                   + delta.xxx * sdf(p + delta.xxx));
}

//==============================================================================================================================================================
// shadows calculation
//==============================================================================================================================================================
float smooth_min(float a, float b, float k)
{
    float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
    return mix(b, a, h) - k * h * (1.0 - h);
}

float smooth_max(float a, float b, float k)
{
    float h = clamp(0.5 - 0.5 * (b - a) / k, 0.0, 1.0);
    return mix(b, a, h) + k * h * (1.0 - h);
}

float hard_shadow_factor(vec3 p, vec3 l, float min_t, float max_t)
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

float soft_shadow_factor(vec3 p, vec3 l, float min_t, float max_t, float k)
{
    float res = 1.0;
    float t = min_t;
    while(t < max_t)
    {
        float h = sdf(p + t * l);
        if(h < 0.001)
            return 0.0;
        res = min(res, k * h / t);
        t += h;
    }
    return res;
}

bool lipshitz_test(vec3 p, vec3 v, float t, float s)
{
    float h = 0.125 * fract(dot(cos(gl_GlobalInvocationID.xy), vec2(-117.4361, 9.15217)));
    float sd0 = sdf(p + (t - 0.5 * h) * v); 
    float sd1 = sdf(p + (t + 0.5 * h) * v); 

    float l = abs(sd1 - sd0) / h;

    if (l <= 1.0)
        return true;
    imageStore(scene_image, ivec2(gl_GlobalInvocationID.xy), vec4(0.0, 0.0, 1.0, 1.0));
    return false;
}

void main()
{
    vec2 uv = inv_res * (vec2(gl_GlobalInvocationID.xy) + 0.5f);        // pixel half-integral coordinates
    vec2 ndc = 2.0f * uv - 1.0f;                                        // normalized device coordinates
    vec3 z_uv = vec3(focal_scale * ndc, -z_near);
    vec3 v = normalize(camera_matrix * z_uv);

//  if (!lipshitz_test(camera_ws, v, 13.0f, 0.125f)) return;

    float t0 = 0.0;
    float d0 = sdf(camera_ws + t0 * v);
    float t1;
    float d1;

    int it = 0;
    while (it < 256)
    {
        t1 = t0 + 0.0009765625 + 1.75 * d0;
        d1 = sdf(camera_ws + t1 * v); 
        if(d1 < 0.0) break;
        t0 = t1;
        d0 = d1;
        ++it;
    }

    float t;
    if (d1 < 0.0)
    {
        for(int i = 0; i < 8; ++i)
        {
            float t2 = (d0 * t1 - d1 * t0) / (d0 - d1);
            float d2 = sdf(camera_ws + t2 * v);
            if (d2 < 0.0)
            {
                t1 = t2;
                d1 = d2;
            }
            else
            {
                t0 = t2;
                d0 = d2;
            }
        }
        t = (d0 * t1 - d1 * t0) / (d0 - d1);
    }
    else
        t = d0;

    vec3 p = camera_ws + v * t;                                                     // fragment world-space position
    vec3 n = normal_AA(p, t);                                                       // antialiased fragment normal

    vec3 cX = camera_matrix[0];                                                     // projections of neighbor fragment displacement onto tangent plane
    vec3 cY = camera_matrix[1];
    float der_factor = pixel_size * TEX_SCALE * t;
    float dp_v = dot(v, n);
    float dp_cX = dot(cX, n);
    float dp_cY = dot(cY, n);

    vec3 q = TEX_SCALE * p;
    vec3 dq_dx = der_factor * (dp_v * cX - dp_cX * v) / (dp_v + pixel_size * dp_cX);
    vec3 dq_dy = der_factor * (dp_v * cY - dp_cY * v) / (dp_v + pixel_size * dp_cY);

    vec3 diffuse_color = tex3D_AA(q, dq_dx, dq_dy, n);                              // trilinear blended diffuse color texture
    vec3 b = bump3D_AA(q, dq_dx, dq_dy, n);                                         // trilinear blended bumped normal

    vec3 l = light_ws - p;                                                          // vector from fragment to light
    float ld = length(l);                                                           // distance to light
    l /= ld;                                                                        // unit light direction

//    float sf = hard_shadow_factor(p, l, 0.0, ld);                                    // calculate shadow factor
    float ssf = soft_shadow_factor(p, l, 0.0, ld, 8.0);                             // calculate shadow factor

    float ao = calc_ao1(p, b);                                                      // ambient occlusion factor
    float ambient_factor = 0.25 * ao;                                               // ambient light factor
    vec3 ambient_color = ambient_factor * diffuse_color;                            // ambient color

    float diffuse_factor = max(dot(b, l), 0.0);                                     // diffuse component factor

    vec3 specular_color = vec3(1.0);
    vec3 h = normalize(l - v);
    float cos_alpha = max(dot(h, b), 0.0f);
    float specular_factor = 0.75 * pow(cos_alpha, 20.0);

    vec3 color = ambient_color + diffuse_factor * diffuse_color + specular_factor * specular_color;

    vec4 FragmentColor = (split_screen != 0) ? vec4(clamp(color, 0.0f, 1.0f), t) : 
                        ((uv.x < 0.5) ? ((uv.y < 0.5) ? vec4(diffuse_color, t)
                                                      : vec4(vec3(ao), t))
                                      : ((uv.y < 0.5) ? vec4(abs(n), t)
                                                      : vec4(abs(b), t)));

    imageStore(scene_image, ivec2(gl_GlobalInvocationID.xy), FragmentColor);
}