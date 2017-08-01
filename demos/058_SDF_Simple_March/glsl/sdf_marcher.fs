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
    return vec3(0.75, 0.75, 0.41);
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

    if (t0 >= t1)
    {
        FragmentColor = texture(environment_tex, view);
        return;        
    }

    float t = raymarch(camera_ws, direction, t0, t1);
    if (t < 0.0)
    {
        FragmentColor = texture(environment_tex, view);
        return;
    }

    vec3 position_ws = camera_ws + t * direction;
    vec3 normal_ws = grad(position_ws);

    vec3 n = normalize(normal_ws);
    vec3 light = light_ws - position_ws;
    vec3 l = normalize(light);
    vec3 v = -direction;

    vec3 color = tex3d(position_ws);
    vec3 diffuse = (0.6f + 0.4f * dot(n, l)) * color.rgb;

    vec3 h = normalize(l + v);
    const float Ks = 0.85f;
    const float Ns = 70.0f;
    float specular = Ks * pow(max(dot(n, h), 0.0), Ns);

    vec3 c = diffuse + vec3(specular);
    FragmentColor = vec4(c, 1.0);
}