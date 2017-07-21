#version 330 core

in vec2 uv;
in vec3 view;

uniform vec3 camera_ws;
uniform vec3 light_ws;

uniform sampler2D tb_tex;
uniform sampler3D sdf_tex;
uniform samplerCube environment_tex;

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
    p.y = -p.y;
    vec3 q = 0.5f * p + 0.5f;
    vec3 r = abs(p);
    float cube_sd = max(r.x, max(r.y, r.z)) - 1.0;
    float tex3d_sd = texture(sdf_tex, q).x;

    if (cube_sd < 0.0)
        return tex3d_sd;
    else
        return max(tex3d_sd, cube_sd);
}

float raymarch(vec3 position, vec3 direction)
{
    const float epsilon = 0.0005;
    const int maxSteps = 128;
    float t = 0.0f;

    for(int i = 0; i < maxSteps; ++i) 
    {
        float d = distance_field(position + direction * t);
        if(d < epsilon)
            return t;
        t += d;
    }

    return -1.0;
}

vec3 grad(vec3 p)
{
    vec3 dp = vec3(0.075f, 0.0f, -0.075f);
    vec3 q = vec3(distance_field(p + dp.xyy) - distance_field(p + dp.zyy), 
                  distance_field(p + dp.yxy) - distance_field(p + dp.yzy), 
                  distance_field(p + dp.yyx) - distance_field(p + dp.yyz));
    return normalize(q);
}

void main()
{
    vec3 direction = normalize(view);

    float t = raymarch(camera_ws, direction);

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