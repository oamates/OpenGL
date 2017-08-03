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
    return tx * tx * q.x + ty * ty * q.y + tz * tz * q.z;
}

//==============================================================================================================================================================
// volume marcher/blender function
//==============================================================================================================================================================
float alpha_func(vec3 p)
{
    vec3 q1 = cos(5.11 * p);
    vec3 q2 = cos(12.17 * p + 13.45);
    float q = 0.147 * dot(q1, q1) + 0.073 * dot(q2, q2);
    return 1.55 * q * q;
}

float distance_field(vec3 p)
{
    vec3 q = bbox_inv_size * (p - bbox_min);
    q.y = 1.0 - q.y;
    float r = texture(sdf_tex, q).x;
    return r;
}

vec3 grad(vec3 p)
{
    vec2 e = vec2(0.0014125, -0.0014125); 
    return normalize(e.xyy * distance_field(p + e.xyy) + e.yyx * distance_field(p + e.yyx) + e.yxy * distance_field(p + e.yxy) + e.xxx * distance_field(p + e.xxx));
}

vec4 multilayer_march(in vec3 position, in vec3 direction, in float min_t, in float max_t, out float t)
{
    const float min_step = 0.0009765625;
    const float min_alpha = 0.001953125;

    float alpha_c = 1.0;
    float ct = min_t;
    float dt = 0.025f;
    t = -1.0;

    vec3 color = vec3(0.0f);

    while((ct < max_t) && (alpha_c > min_alpha))
    {
        vec3 p = position + ct * direction;
        float d = distance_field(p);

        if (d > 0)
        {
            // outside the surface
            ct += max(d, min_step);
            continue;
        }
        if (t < 0) t = ct;
        
        // inside the surface
        float a = alpha_func(p);
        vec3 rgb = tex3d(p);

        alpha_c *= (1.0 - a);
        color += alpha_c * rgb;
        ct += max(dt * exp2(-2.0f * a), min_step);
    }
    return vec4(color, alpha_c);
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
    vec4 color = multilayer_march(camera_ws, direction, t0, t1, t);

    vec3 position_ws = camera_ws + t * direction;
    vec3 n = grad(position_ws);

    vec3 light = light_ws - position_ws;
    vec3 l = normalize(light);
    vec3 v = -direction;

    vec3 diffuse = (0.5f + 0.5f * dot(n, l)) * color.rgb;

    vec3 h = normalize(l + v);
    const float Ks = 0.15f;
    const float Ns = 70.0f;
    float specular = Ks * pow(max(dot(n, h), 0.0), Ns);

    vec3 c = diffuse + vec3(specular);
    FragmentColor.rgb = diffuse + color.a * FragmentColor.rgb;
}