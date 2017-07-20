#version 330 core

in vec3 position_ws;
in vec3 normal_ws;

uniform sampler2D tb_tex;
uniform sampler2D value_tex;

uniform sampler3D sdf_tex;

uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform float inv_scale;

out vec4 FragmentColor;

//==============================================================================================================================================================
// 3d value noise function
//==============================================================================================================================================================
const float TEXEL_SIZE = 1.0 / 256.0;
const float HALF_TEXEL = 0.5 * TEXEL_SIZE;

vec3 hermite5(vec3 x)
    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

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
    { return abs(fract(x) - 0.5); }

//==============================================================================================================================================================
// trilinear blend
//==============================================================================================================================================================
vec3 tex3d(vec3 p)
{
    vec3 q = max(abs(normalize(p)) - 0.35, 0.0);
    q /= dot(q, vec3(1.0));
    vec3 tx = texture(tb_tex, p.zy).rgb;
    vec3 ty = texture(tb_tex, p.xz).rgb;
    vec3 tz = texture(tb_tex, p.xy).rgb;
    return tx * tx * q.x + ty * ty * q.y + tz * tz * q.z;
}

//==============================================================================================================================================================
// volume marcher/blender function
//==============================================================================================================================================================
vec2 csqr(vec2 a)
    { return vec2(a.x * a.x - a.y * a.y, 2.0f * a.x * a.y); }

float map(in vec3 p)
{
    float res = 0.0f;
    vec3 c = p;
    for (int i = 0; i < 6; ++i) 
    {
        p = 0.7f * (abs(p) / dot(p, p)) - 0.7f;
        p.yz = csqr(p.yz);
        p = p.zxy;
        res += exp(-19.0f * abs(dot(p, c)));
        
    }
    return 0.5f * res;
}

float distance_field(vec3 p)
{
    vec3 q = 0.5f * inv_scale * p + 0.5f;
    return texture(sdf_tex, q);
}

vec4 crystal_march(vec3 position, vec3 ray)
{
    float alpha = 0.0;
    float t = 0.0f;
    float dt = 0.125f;
    vec3 color = vec3(0.0f);
    float c = 0.0f;
    for(int i = 0; i < 40; i++)
    {
        t += dt * exp(-2.0f * c);
        vec3 p = position + t * ray;
        if (distance_field(p) > 0.0)
            break;        
        float q = pow(0.4 * length(p), 1.4);
        c = q * map(0.2 * p);
        color = 0.975f * color + vec3(0.1f, 0.08f, 0.06f) * pow(vec3(c), vec3(0.75, 1.21, 2.11));
        alpha = max(alpha, q);
    }    
    return vec4(log(1.0 + 1.25 * color), sqrt(alpha));
}

void main()
{
    vec3 n = normalize(normal_ws);
    vec3 light = light_ws - position_ws;
    vec3 view = camera_ws - position_ws;
    vec3 l = normalize(light);
    vec3 v = normalize(view);

    vec4 color = crystal_march(position_ws, -v);
    vec3 diffuse = (0.6f + 0.4f * dot(n, l)) * color.rgb;

    vec3 h = normalize(l + v);
    const float Ks = 0.85f;
    const float Ns = 70.0f;
    float specular = Ks * pow(max(dot(n, h), 0.0), Ns);

    vec3 c = diffuse + vec3(specular);
    FragmentColor = vec4(c, clamp(0.88 * color.a * sqrt(length(color.rgb)), 0.0, 1.0));

}