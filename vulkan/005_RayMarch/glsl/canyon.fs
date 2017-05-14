#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 view;

layout(binding = 0) uniform UniformBufferObject
{
    mat3 camera_matrix;
    vec3 camera_ws;
    vec3 light_ws;
    vec2 focal_scale;
    float time;
} ubo;

layout(binding = 1) uniform sampler2D stone_tex;

layout(location = 0) out vec4 FragmentColor;

const float pi = 3.14159265359;
const float HORIZON = 200.0;
const vec3 rgb_power = vec3(0.299, 0.587, 0.114);

//==============================================================================================================================================================
// helper functions
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

//==============================================================================================================================================================
// nvidia trilinear-blend texture
//==============================================================================================================================================================
vec3 tex3D(vec3 p, vec3 n)
{
    
    p *= 0.197;
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
    vec3 op = tri(1.1f * p + tri(1.1f * p.zxy));
    float ground = p.z + 3.5 + dot(op, vec3(0.067));
    p += (op - 0.25) * 0.3;
    p = cos(0.444f * p + sin(1.112f * p.zxy));
    float canyon = (length(p) - 1.05) * 0.95;
    return min(ground, canyon);
}

//==============================================================================================================================================================
// sdf gradient :: tetrahedral evaluation
//==============================================================================================================================================================
vec3 calc_normal(in vec3 p)
{
    vec2 e = vec2(0.0125, -0.0125);
    return normalize(e.xyy * sdf(p + e.xyy) + e.yyx * sdf(p + e.yyx) + e.yxy * sdf(p + e.yxy) + e.xxx * sdf(p + e.xxx));
}

//==============================================================================================================================================================
// bumped normal calculation
//==============================================================================================================================================================
vec3 bump_normal(in vec3 p, in vec3 n, float bf)
{
    const vec2 e = vec2(0.0025, 0);
    mat3 m = mat3(tex3D(p - e.xyy, n),
                  tex3D(p - e.yxy, n),
                  tex3D(p - e.yyx, n));
    vec3 g = rgb_power * m;
    g = (g - dot(tex3D(p, n), rgb_power)) / e.x;
    return normalize(n + g * bf);
}

//==============================================================================================================================================================
// basic raymarching loop
//==============================================================================================================================================================
float trace(in vec3 p, in vec3 v, out float accum)
{    
    accum = 0.0;
    float t = 0.0;
    for(int i = 0; i < 160; i++)
    {    
        float d = sdf(p + t * v);
        float d_abs = abs(d);
        if(d_abs < (0.001 + 0.00025 * t) || t > HORIZON) break;
        t += d;
        if(d_abs < 0.25) accum += 0.0417 * (0.25 - d_abs);
    }
    return min(t, HORIZON);
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
    return tex3D(rd, n);
}

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    vec3 v = normalize(view);
    float accum;
    float t = trace(ubo.camera_ws, v, accum);
    vec3 sceneCol = vec3(0.0);

    vec3 p = ubo.camera_ws + v * t;                                                 // fragment position
    vec3 n = calc_normal(p);                                                        // fragment normal
    vec3 b = bump_normal(p, n, 0.0175);                                             // bumped normal
    vec3 l = ubo.light_ws - p;                                                      // light direction :: from fragment to light source
    float d = length(l);                                                            // distance from fragment to light
    l /= d;                                                                         // normalized light direction
    float sf = soft_shadow_factor(p, b, l, d);                                      // calculate shadow factor

    float ao = calc_ao1(p, b);                                                      // calculate ambient occlusion factor
    float atten = 1.0 / (1.0 + d * 0.007);                                          // light attenuation, based on the light distance
    float diffuse = max(dot(b, l), 0.0);                                            // diffuse lighting factor
    float specular = pow(max(dot(reflect(-l, b), -v), 0.0), 32.0);                  // specular lighting factor
    float fresnel = pow(clamp(dot(b, v) + 1.0, 0.0, 1.0), 1.0);                     // Fresnel term for reflective glow
    float ambience = (0.35 * ao + fresnel * fresnel * 0.25);                          // ambient light factor, based on occlusion and fresnel factors
    vec3 texCol = tex3D(p, n);                                                      // trilinear blended color from input texture

    //==========================================================================================================================================================
    // white frost
    //==========================================================================================================================================================
    texCol = mix(texCol, vec3(0.35, 0.55, 1.0) * (texCol * 0.5 + 0.5) * vec3(2.0), ((n.z * 0.5 + b.z * 0.5) * 0.5 + 0.5) * pow(abs(b.z), 4.0) * texCol.r * fresnel * 4.0);

    sceneCol = texCol * (diffuse + specular + ambience);                                    // final color
    sceneCol += texCol * (0.5 + 0.5f * b.z) * min(vec3(1.0, 1.15, 1.5) * accum, 1.0);       // accumulated glow
    sceneCol += texCol * vec3(0.4, 0., 1.0) * pow(fresnel, 4.0) * 0.5;                      // Fresnel glow
    vec3 nn = 0.5 * (n + b);                                                                // environment mapping
    vec3 ref = reflect(v, nn);
    vec3 em1 = envMap(0.500 * ref, nn);
    ref = refract(v, nn, 0.76);
    vec3 em2 = envMap(0.125 * ref, nn);
    sceneCol += sceneCol * (1.5 + 0.5 * b.z) * mix(em2, em1, pow(fresnel, 4.0));
    sceneCol *= atten * min(sf + ao * 0.35, 1.0) * ao * (0.0625 + 0.9375 * sf);                              // shading + some ambient occlusion

    //==========================================================================================================================================================
    // add a bit of light fog for atmospheric effect
    //==========================================================================================================================================================
    vec3 fog = vec3(0.6, 0.8, 1.2) * (v.z * 0.5 + 0.5);
    sceneCol = mix(sceneCol, fog, smoothstep(0.0f, 0.95f, t / HORIZON));
    
    FragmentColor = vec4(pow(clamp(sceneCol, 0.0f, 1.0f), vec3(0.66f)), 1.0f);
    //FragmentColor = vec4(vec3(sf), 1.0f);
}