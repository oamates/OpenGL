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
        res += exp(-19.0f * abs(dot(p, c)));
        
    }
    return 0.5f * res;
}

float distance_field(vec3 p)
{
    vec3 q = bbox_inv_size * (p - bbox_min);
    float r = texture(sdf_tex, q).x;
    return r;
}

float raymarch(vec3 position, vec3 direction, float min_t, float max_t)
{
    const int maxSteps = 160;
    float t = min_t;

    for(int i = 0; i < maxSteps; ++i) 
    {
        float d = distance_field(position + direction * t);

        if(d < 0.0007125)
            return t;
        t += d;

        if(t >= max_t)
            break;
    }
    return -1.0;
}

vec3 grad(vec3 p)
{
    vec2 e = vec2(0.0014125, -0.0014125); 
    return normalize(e.xyy * distance_field(p + e.xyy) + e.yyx * distance_field(p + e.yyx) + e.yxy * distance_field(p + e.yxy) + e.xxx * distance_field(p + e.xxx));
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
        c = q * map(0.25 * p);
        color = 0.97f * color + vec3(0.1f, 0.08f, 0.06f) * pow(vec3(c), vec3(0.75, 1.21, 2.11));
        alpha = max(alpha, q);
    }    
    return vec4(log(1.0 + 1.25 * color), alpha);
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
    float t = raymarch(camera_ws, direction, t0, t1);
    if (t < 0.0) return;


    vec3 position_ws = camera_ws + t * direction;
    vec3 normal_ws = grad(position_ws);

    vec4 color = crystal_march(position_ws, view);

    vec3 n = normalize(normal_ws);
    vec3 light = light_ws - position_ws;
    vec3 l = normalize(light);
    vec3 v = -direction;

    vec3 diffuse = (0.5f + 0.5f * dot(n, l)) * color.rgb;

    vec3 h = normalize(l + v);
    const float Ks = 0.85f;
    const float Ns = 70.0f;
    float specular = Ks * pow(max(dot(n, h), 0.0), Ns);

    vec3 c = diffuse + vec3(specular);
    FragmentColor = mix(FragmentColor, c, color.a);
}