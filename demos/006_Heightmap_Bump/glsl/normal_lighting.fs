#version 430 core

uniform sampler2D diffuse_tex;
uniform sampler2D heightmap_tex; 

in vec3 position_ws;
in vec3 view;
in vec3 light;
in vec3 normal_ws;
in vec2 uv;
in float hue;

out vec4 FragmentColor;
 
vec3 rgb2hsv(vec3 c)
{
    const vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
 

vec3 hsv2rgb(vec3 c)
{
    const vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 bump_normal(vec3 p, vec3 n, vec2 uv)
{
    float lod = textureQueryLod(heightmap_tex, uv).x;
    //lod -= 0.5f;
    //vec2 size = textureSize(heightmap_tex, 0);

    vec3 Px = dFdx(p);
    vec3 Py = dFdy(p);

    vec3 Nx = dFdx(n);
    vec3 Ny = dFdy(n);

    float B = texture(heightmap_tex, uv).r - 0.5f;

    vec2 UVx = dFdx(uv);
    vec2 UVy = dFdy(uv);

    float q = 0.1f;

    float Bx = textureLod(heightmap_tex, uv + q * UVx, lod).r - textureLod(heightmap_tex, uv - q * UVx, lod).r;
    float By = textureLod(heightmap_tex, uv + q * UVy, lod).r - textureLod(heightmap_tex, uv - q * UVy, lod).r;

    const float bm = 0.02f;

    vec3 P_x = Px + bm * (Bx * n + B * Nx);
    vec3 P_y = Py + bm * (By * n + B * Ny);

    vec3 b = normalize(cross(P_x, P_y));
    return b;
}

void main()
{    
    vec3 l = normalize(light);
    vec3 v = normalize(view);
    vec3 n = bump_normal(position_ws, normal_ws, uv);

    vec3 rgb = texture(diffuse_tex, uv).rgb;
    vec3 hsv = rgb2hsv(rgb);
    hsv.x = 0.25f + 0.25f * hue;
    hsv.y = pow(hsv.y, 0.25);
    vec3 diffuse_color = hsv2rgb(hsv);
    vec3 ambient_color = 0.25f * diffuse_color;
    vec3 specular_color = hsv2rgb(vec3(hsv.x, pow(hsv.yz, vec2(0.25))));

    float cos_theta = dot(n, l);

    vec3 color = ambient_color;

    if (cos_theta > 0.0f) 
    {
        color += cos_theta * diffuse_color;

        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, n), 0.0f);

        const float Ns = 80.0f;
        color += 2.75 * pow(cos_alpha, Ns) * specular_color;
    }

    FragmentColor = vec4(color, 1.0f);                 
}





