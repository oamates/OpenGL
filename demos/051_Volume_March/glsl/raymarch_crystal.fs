#version 330 core

in vec3 position_ws;
in vec3 normal_ws;

uniform sampler2D tb_tex;
uniform sampler2D value_tex;

uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform float scale;

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
    p *= 0.75;
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
const float phi = 1.618033988749894848204586834365638117720309179805762862135; // (sqrt(5) + 1) / 2
const float psi = 1.511522628152341460960267404050002785276889577787122118459; // (sqrt(5) + 3) / (2 * sqrt(3))

float distance_field(vec3 p)
{
    vec3 q = abs(p);
    vec3 l = q + phi * q.yzx;
    return max(l.x, max(l.y, l.z)) - scale * psi;
}

float alpha_func(vec3 p, vec3 color)
{
    float q = 0.005;// - clamp(0.013 * length(color), 0.0, 1.0);
    return q;
}

vec3 color_func(vec3 p)
{
    return tex3d(p);
}

vec3 crystal_march(vec3 front, vec3 view_ray)
{
    vec3 color = vec3(0.0);                         // accumulated color
    float alpha = 1.0;                              // accumulated alpha
    vec3 p = front;
    do
    {
        vec3 c = tex3d(0.85 * p);
        float a = alpha_func(p, c);
        float dt = 0.025f * exp(-1.2f * a);
        p = p + dt * view_ray;
        color = 0.99f * color + 0.06f * c;
    }
    while(distance_field(p) <= 0.0);

    return pow(color, vec3(0.56, 0.67, 0.92));
}

void main()
{
    vec3 n = normalize(normal_ws);
    vec3 light = light_ws - position_ws;
    vec3 view = camera_ws - position_ws;
    vec3 l = normalize(light);
    vec3 v = normalize(view);

    vec3 color = crystal_march(position_ws, -v);
    //vec3 color = raymarch(position_ws, -v);
    //FragmentColor = vec4(color, 1.0);

    vec3 diffuse = (0.6 + 0.4 * dot(n, l)) * color;

    vec3 h = normalize(l + v);  
    const float Ks = 0.85f;
    const float Ns = 70.0f;
    float specular = Ks * pow(max(dot(n, h), 0.0), Ns);

    vec3 c = diffuse + vec3(specular);
    FragmentColor = vec4(c, 1.0);

}