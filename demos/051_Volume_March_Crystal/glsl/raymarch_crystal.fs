#version 330 core

in vec3 position_ws;
in vec3 normal_ws;

uniform sampler2D backface_depth_tex;
uniform sampler2D tb_tex;
uniform sampler2D value_tex;

uniform mat3 camera_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform vec2 focal_scale;
uniform vec2 inv_resolution;

out vec4 FragmentColor;

const float diameter = 6.0f;

//==============================================================================================================================================================
// 3D Value noise function
//==============================================================================================================================================================
const float TEXEL_SIZE = 1.0 / 256.0;
const float HALF_TEXEL = 0.5 * TEXEL_SIZE;

vec3 hermite5(vec3 x)
{
    return x * x * x * (10.0 + x * (6.0 * x - 15.0));
}

float vnoise(vec3 x)
{
    vec3 p = floor(x);
    vec3 f = x - p;
    f = hermite5(f);
    vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg = texture(value_tex, TEXEL_SIZE * uv + HALF_TEXEL).rg;
    return mix(rg.g, rg.r, f.z);
}

float tri(float x)
{
    return abs(fract(x) - 0.5);
}

float noise_map(vec3 x)
{
    float q = tri(vnoise(x));
    q *= tri(vnoise(2.12 * x + vec3(3.41)));
    q *= tri(vnoise(4.41 * x - vec3(7.11)));
    q += 0.015 * tri(vnoise(6.41 * x));
    return 14.1 * q;
}

vec4 noise_map_bump(in vec3 p)
{
    vec3 e = vec3(0.03, -0.03, 0.25);
    vec4 b = e.xyyz * noise_map(p + e.xyy) + e.yyxz * noise_map(p + e.yyx) + e.yxyz * noise_map(p + e.yxy) + e.xxxz * noise_map(p + e.xxx);
    b.xyz = normalize(b.xyz);
    return b;
}

//==============================================================================================================================================================
// trilinear blend
//==============================================================================================================================================================
vec3 tex2d(vec2 uv)
{
    return texture(tb_tex, uv).rgb;
}

vec3 tex3d(vec3 p)
{
    p *= 0.75;
    vec3 q = max(abs(normalize(p)) - 0.35, 0.0);
    q /= dot(q, vec3(1.0));
    vec3 tx = tex2d(p.zy);
    vec3 ty = tex2d(p.xz);
    vec3 tz = tex2d(p.xy);
    return tx * tx * q.x + ty * ty * q.y + tz * tz * q.z;
}

float csZ(float depth)
{                                
    const float znear = 0.5;
    return znear / (depth - 1.0);
}

vec3 position_cs(vec2 uv)
{
    vec2 ndc = 2.0 * uv - 1.0;
    vec3 view = vec3(focal_scale * ndc, -1.0f);
    float depth = texture(backface_depth_tex, uv).r;
    float Z = csZ(depth);
    vec3 p = -Z * view;
    return p;
}

/*
//==============================================================================================================================================================
// Tetrahedral normal
//==============================================================================================================================================================
vec4 map_bump(vec3 p)
{
    vec3 e = vec3(0.001, -0.001, 0.25);
    vec4 b = e.xyyz * map(p + e.xyy) + e.yyxz * map(p + e.yyx) + e.yxyz * map(p + e.yxy) + e.xxxz * map(p + e.xxx);
    b.xyz = normalize(b.xyz);
    return b;
}
*/

float alpha_func(vec3 p, vec3 color)
{
    return max(1.0 - 0.61 * dot(color, vec3(1.0)), 0.0);
}

vec4 crystal_march(vec3 front, vec3 back)
{
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 ray = front - back;

    const int iterations = 16;
    const float inv_iterations = 1.0 / iterations;

    vec3 position = back;
    vec3 step = inv_iterations * ray;

    for(int i = 0; i <= iterations; ++i)
    {
        vec3 medium_color = tex3d(position);
        float alpha = alpha_func(position, medium_color);
        color.rgb = mix(color.rgb, medium_color, alpha);
        color.a *= 1.0 - alpha;
        position += step;
    }

    color.a = 1.0 - color.a;
    return color;
}

void main()
{
    vec2 uv = gl_FragCoord.xy * inv_resolution;

    vec3 n = normalize(normal_ws);
    vec3 light = light_ws - position_ws;
    vec3 view = camera_ws - position_ws;
    vec3 l = normalize(light);
    vec3 v = normalize(view);

    vec3 backface_ws = camera_ws + camera_matrix * position_cs(uv);

    vec4 color = crystal_march(position_ws, backface_ws);
    vec3 diffuse = (0.6 + 0.4 * dot(n, l)) * color.rgb;

    vec3 h = normalize(l + v);  
    const float Ks = 0.85f;
    const float Ns = 70.0f;
    float specular = Ks * pow(max(dot(n, h), 0.0), Ns);

    vec3 c = diffuse + vec3(specular);
    FragmentColor = vec4(c, color.a);
}





