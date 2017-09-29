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

layout (rgba8, binding = 0) uniform image2D scene_image;

uniform mat3 camera_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform vec2 focal_scale;                                                   // camera focal scale
uniform float pixel_size;                                                        // NDC / pixel uv - ratio
uniform float z_near;                                                       // distance to near z-plane

uniform sampler2D tb_tex;

uniform int split_screen;

//==============================================================================================================================================================
// smoothing hermite interpolation polynomial
//==============================================================================================================================================================
float hermite5(float x)
    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

vec3 hermite5(vec3 x)
    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

float hermite5(float a, float b, float x)
{
    float y = clamp((x - a) / (b - a), 0.0, 1.0);
    return y * y * y * (10.0 + y * (6.0 * y - 15.0));
}



const float pi = 3.14159265359;
const float HORIZON = 200.0;

//==============================================================================================================================================================
// trilinear blend texture
//==============================================================================================================================================================
const float tex_scale = 0.5;

vec3 tex3D(vec3 p, vec3 n)
{
    vec3 q = tex_scale * p;
    n = max(abs(n) - 0.35f, 0.0f);
    n /= dot(n, vec3(1.0));
    vec3 tx = texture(tb_tex, q.yz).rgb;
    vec3 ty = texture(tb_tex, q.zx).rgb;
    vec3 tz = texture(tb_tex, q.xy).rgb;
    return tx * n.x + ty * n.y + tz * n.z;
}

float luminosity(vec3 p, vec3 n)
{
    vec3 q = tex_scale * p;
    n = max(abs(n) - 0.35f, 0.0f);

    vec3 l = vec3(dot(texture(tb_tex, q.yz).rgb, RGB_SPECTRAL_POWER),
                  dot(texture(tb_tex, q.zx).rgb, RGB_SPECTRAL_POWER),
                  dot(texture(tb_tex, q.xy).rgb, RGB_SPECTRAL_POWER));

    return dot(l, n) / dot(n, vec3(1.0));
}

vec3 tex3D_AA(vec3 p, vec3 n, vec3 v, float l, float t)
{
    vec3 q = tex_scale * p;
    n = max(abs(n) - 0.35f, 0.0f);
    n /= dot(n, vec3(1.0));

    float der_factor = tex_scale * pixel_size * t / l;
    float dp = dot(v, n);

    vec3 cX = camera_matrix[0];
    vec3 cY = camera_matrix[1];

    float qX = dot(cX, n) / dp;
    float qY = dot(cY, n) / dp;

    vec3 dq_dx = der_factor * (cX - qX * v);
    vec3 dq_dy = der_factor * (cY - qY * v);

    vec3 tx = textureGrad(tb_tex, q.yz, dq_dx.yz, dq_dx.yz).rgb;
    vec3 ty = textureGrad(tb_tex, q.zx, dq_dx.zx, dq_dx.zx).rgb;
    vec3 tz = textureGrad(tb_tex, q.xy, dq_dx.xy, dq_dx.xy).rgb;

    vec3 b = tx * n.x + ty * n.y + tz * n.z;
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

//    return abs(fract(p) - 0.5);
}


float sdf(vec3 p)
{
    p *= 0.77;
    vec3 op = tri_smooth(1.1f * p + tri_sharp(1.1f * p.zxy));
    float ground = p.z + 0.5 + dot(op, vec3(0.067));
    p += (op - 0.25) * 0.3;
    p = cos(0.444f * p + sin(1.112f * p.zxy));
    float canyon = (length(p) - 1.05) * 0.95;
    return 1.41 * min(ground, canyon);
}


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
float DERIVATIVE_SCALE = 0.0093717;
const vec3 RGB_SPECTRAL_POWER = vec3(0.299, 0.587, 0.114);

vec3 normal(vec3 p)
{
    float scale = DERIVATIVE_SCALE;
    vec2 delta = vec2(scale, -scale); 
    return normalize(delta.xyy * sdf(p + delta.xyy) 
                   + delta.yyx * sdf(p + delta.yyx) 
                   + delta.yxy * sdf(p + delta.yxy) 
                   + delta.xxx * sdf(p + delta.xxx));
}

vec3 normal_AA(vec3 p, float t)
{
    float scale = DERIVATIVE_SCALE * t;
    vec2 delta = vec2(scale, -scale); 
    return normalize(delta.xyy * sdf(p + delta.xyy) 
                   + delta.yyx * sdf(p + delta.yyx) 
                   + delta.yxy * sdf(p + delta.yxy) 
                   + delta.xxx * sdf(p + delta.xxx));
}

vec3 bump_normal_AA(vec3 p, vec3 n, float bf, float t)
{
    float scale = DERIVATIVE_SCALE * t;
    vec2 delta = vec2(scale, -scale);

    vec3 g = vec3(delta.xyy * dot(tex3D(p + delta.xyy, n), RGB_SPECTRAL_POWER) 
                + delta.yyx * dot(tex3D(p + delta.yyx, n), RGB_SPECTRAL_POWER) 
                + delta.yxy * dot(tex3D(p + delta.yxy, n), RGB_SPECTRAL_POWER) 
                + delta.xxx * dot(tex3D(p + delta.xxx, n), RGB_SPECTRAL_POWER));

    float inv_factor = 1.0 / (scale * scale);
    g = inv_factor * g;
    return normalize(n + bf * g);                                       // Bumped normal. "bf" - bump factor.
}


vec3 bump_normal_AA_AA(vec3 p, vec3 n, vec3 v, float bf, float l, float t)
{
    vec2 delta = vec2(DERIVATIVE_SCALE * t, 0.0f);
    mat3 m = mat3(tex3D_AA(p - delta.xyy, n, v, l, t), 
                  tex3D_AA(p - delta.yxy, n, v, l, t), 
                  tex3D_AA(p - delta.yyx, n, v, l, t));
    vec3 g = RGB_SPECTRAL_POWER * m;                                    // Converting to greyscale.
    g = (g - dot(tex3D_AA(p, n, v, l, t), RGB_SPECTRAL_POWER)) / delta.x;
    return normalize(n + g * bf);                                       // Bumped normal. "bf" - bump factor.
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

vec3 envMap(vec3 rd, vec3 n)
{    
    return tex3D(rd, n);
}

void main()
{

    vec2 uv = (vec2(gl_GlobalInvocationID.xy) + 0.5f) / vec2(1920.0f, 1080.0f);     // pixel half-integral coordinates
    vec2 ndc = 2.0f * uv - 1.0f;                                                    // normalized device coordinates
    vec3 z_uv = vec3(focal_scale * ndc, -z_near);
    float h = length(z_uv);
    float inv_h = 1.0f / h;

    vec3 view_ws = inv_h * (camera_matrix * z_uv);
    vec3 v = view_ws;

    float t = 0.0;
    for(int i = 0; i < 128; i++)
    {    
        float h = sdf(camera_ws + v * t);
        if(abs(h) < 0.001 * (t * 0.25 + 1.0) || t > HORIZON) break;
        t += h;
    }
    
    vec3 p = camera_ws + v * t;                                                     // fragment world-space position
    vec3 n = normal_AA(p, t);                                                       // antialiased fragment normal
    vec3 b = bump_normal_AA(p, n, 0.0471f, t);                                       // texture-based bump mapping
    //vec3 b = bump_normal_AA_AA(p, n, v, 0.111f, h, t);                                       // texture-based bump mapping

    vec3 l = light_ws - p;                                                          // from fragment to light
    float ld = max(length(l), 0.001);                                               // distance to light
    l /= ld;                                                                        // unit light direction

    float sf = hard_shadow_factor(p, l, 0.0, ld);                                    // calculate shadow factor
//    float ssf = soft_shadow_factor(p, l, 0.0, ld, 8.0);                                    // calculate shadow factor

    float ao = calc_ao1(p, b);                                                      // ambient occlusion factor
    float attenuation_factor = 1.0 / (1.0 + ld * 0.007);                            // attenuation w.r.t light proximity
    float diffuse_factor = max(dot(b, l), 0.0);                                     // diffuse component factor
    float specular_factor = pow(max(dot(reflect(-l, n), -v), 0.0), 32.0);           // specular light factor
    float fresnel_factor = pow(clamp(dot(b, v) + 1.0, 0.0, 1.0), 5.0);              // fresnel term, for reflective glow
    float ambient_factor = ao * (0.75 * ao + fresnel_factor * fresnel_factor * 0.15);                     // ambient light factor

    vec3 texCol = tex3D(p, n);                                                     // trilinear blended texture
    vec3 color = texCol * (diffuse_factor + specular_factor + ambient_factor);

    vec4 FragmentColor = (split_screen != 0) ? vec4(clamp(color, 0.0f, 1.0f), 1.0f) : 
                        ((uv.x < 0.5) ? ((uv.y < 0.5) ? vec4(texCol, 1.0f)
                                                      : vec4(ao, ao, ao, 1.0f))
                                      : ((uv.y < 0.5) ? vec4(abs(n), 1.0f)
                                                      : vec4(abs(b), 1.0f)));

    imageStore(scene_image, ivec2(gl_GlobalInvocationID.xy), FragmentColor);
}