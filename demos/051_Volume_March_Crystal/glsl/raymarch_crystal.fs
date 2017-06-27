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

vec3 tex3d(in vec3 p)
{
    vec3 w = vec3(0.331);
    mat3 rgb_samples = mat3(tex2d(p.yz), tex2d(p.zx), tex2d(p.xy));
    return pow(rgb_samples * w, vec3(1.5));
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

vec2 csqr(vec2 a)
{
    return vec2(a.x * a.x - a.y * a.y, 2.0f * a.x * a.y); 
}

float map(vec3 p)
{
    p *= 0.341;
    float res = 0.0f;    
    vec3 c = p;
    for (int i = 0; i < 4; ++i)
    {
        p = 0.7f * abs(p) / dot(p, p) - 0.7f;
        p.yz = csqr(p.yz);
        p = p.zxy;
        res += exp(-18.0f * abs(dot(p, c)));
        
    }
    return 0.5f * res;
}

//==============================================================================================================================================================
// Tetrahedral normal
//==============================================================================================================================================================
vec4 map_bump(in vec3 p)
{
    vec3 e = vec3(0.001, -0.001, 0.25);
    vec4 b = e.xyyz * map(p + e.xyy) + e.yyxz * map(p + e.yyx) + e.yxyz * map(p + e.yxy) + e.xxxz * map(p + e.xxx);
    b.xyz = normalize(b.xyz);
    return b;
}

vec4 raymarch(in vec3 front, in vec3 back)
{
    float alpha = 0.25;
    vec3 col = vec3(0.0f);
    vec3 dir = front - back;
    float l = min(length(dir), 6.0f);

    dir = dir / l;
    
    int iterations = 40;
    float step = l / iterations;

    for(int i = 0; i <= iterations; ++i)
    {
        vec3 p = back + i * step * dir;
        float c = noise_map(4.67 * p);
        float d = abs(c) - 0.117f;

        if (d > 0.0)
        {
            float q = smoothstep(0.0, 1.0, d);
            q = pow(q, 0.41);
            vec3 b = sqrt(sqrt(tex3d(p)));

            col = mix(col, b, q);
            alpha = max(alpha, q);

        }
    }

    return vec4(col, alpha);
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

    vec4 color = raymarch(position_ws, backface_ws);
    vec3 diffuse = (0.6 + 0.4 * dot(n, l)) * color.rgb;

    vec3 h = normalize(l + v);  
    const float Ks = 0.85f;
    const float Ns = 70.0f;
    float specular = Ks * pow(max(dot(n, h), 0.0), Ns);

    vec3 c = diffuse + vec3(specular);
    FragmentColor = vec4(c, color.a);
}





