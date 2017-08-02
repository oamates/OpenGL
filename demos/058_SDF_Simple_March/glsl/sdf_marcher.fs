#version 330 core

in vec2 uv;
in vec3 view;

uniform vec3 camera_ws;
uniform vec3 light_ws;

uniform sampler2D tb_tex;
uniform sampler3D sdf_tex;
uniform samplerCube environment_tex;

uniform vec3 bbox_half_size;
uniform vec3 bbox_inv_size;
uniform vec3 bbox_center;
uniform vec3 bbox_min;

out vec4 FragmentColor;

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
    return sqrt(tx * tx * q.x + ty * ty * q.y + tz * tz * q.z);
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
        res += exp(-16.0f * abs(dot(p, c)));
        
    }
    return 0.5f * res;
}

float distance_field(vec3 p)
{
    vec3 q = bbox_inv_size * (p - bbox_min);
    float r = texture(sdf_tex, q).x;
    return r;
}

vec3 grad(vec3 p)
{
    vec2 e = vec2(0.0014125, -0.0014125); 
    return normalize(e.xyy * distance_field(p + e.xyy) + e.yyx * distance_field(p + e.yyx) + e.yxy * distance_field(p + e.yxy) + e.xxx * distance_field(p + e.xxx));
}

vec4 multilayer_crystal_march(in vec3 position, in vec3 direction, in float min_t, in float max_t, out float t)
{
    float alpha = 0.0;
    const int maxSteps = 256;
    t = min_t;
    float dt = 0.0625f;

    int i = 0;
    float c = 0.0f;
    vec3 color = vec3(0.0f);

    while((i < maxSteps) && (t < max_t))
    {
        vec3 p = position + t * direction;
        float d = distance_field(p);

        if (d > 0)
        {
            // outside the surface
            c = 0.0f;
        }
        else
        {
            // inside the surface
            c = map(1.25 * p);
            c *= c;
            color = 0.97f * color + vec3(c, c * c, c * c * c);

        }
        t += max(d * exp(-2.0f * c), 0.003125);
        alpha = max(alpha, c);
    }

    return vec4(color, alpha);
}

void main()
{
    vec3 direction = normalize(view);

    vec3 s = bbox_half_size / abs(direction);
    vec3 r = (bbox_center - camera_ws) / direction;
    vec4 a = vec4(r - s, 0.0);
    a.xy = max(a.xy, a.zw);
    float t0 = max(a.x, a.y);

    vec3 b = r + s;
    float t1 = min(min(b.x, b.y), b.z);
    FragmentColor = texture(environment_tex, view);

    if (t0 >= t1) return;

    float t;
    vec4 color = multilayer_crystal_march(camera_ws, direction, t0, t1, t);

    vec3 position_ws = camera_ws + t * direction;
    vec3 normal_ws = grad(position_ws);

    vec3 n = normalize(normal_ws);
    vec3 light = light_ws - position_ws;
    vec3 l = normalize(light);
    vec3 v = -direction;

    vec3 diffuse = (0.5f + 0.5f * dot(n, l)) * color.rgb;

    vec3 h = normalize(l + v);
    const float Ks = 0.85f;
    const float Ns = 70.0f;
    float specular = Ks * pow(max(dot(n, h), 0.0), Ns);

    vec4 c = vec4(diffuse + vec3(specular), 1.0f);
    FragmentColor = mix(FragmentColor, c, color.a);
}