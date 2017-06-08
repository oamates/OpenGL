#version 330 core

in vec3 position_ws;
in vec3 normal_ws;
in vec3 view;
in vec3 light;

uniform sampler2D tb_tex2d;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;
uniform float bf;

out vec4 FragmentColor;

const vec3 rgb_power = vec3(0.299f, 0.587f, 0.114f);

//==============================================================================================================================================================
// Straightforward tri-linear texturing and bump :: 18 texture samples
//==============================================================================================================================================================
vec3 tex2d(vec2 uv)
{
    return texture(tb_tex2d, uv).rgb;
}

vec3 tex3d(in vec3 p, in vec3 n)
{
    p *= 0.4875;
    vec3 w = max(abs(n) - 0.317f, 0.0f);
    w /= dot(w, vec3(1.0f));
    mat3 rgb_samples = mat3(tex2d(p.yz), tex2d(p.zx), tex2d(p.xy));
    return rgb_samples * w;
}

vec3 bump_normal(in vec3 p, in vec3 n)
{
    const vec2 e = vec2(0.03125, 0);
    mat3 mp = mat3(tex3d(p + e.xyy, n), tex3d(p + e.yxy, n), tex3d(p + e.yyx, n));
    mat3 mm = mat3(tex3d(p - e.xyy, n), tex3d(p - e.yxy, n), tex3d(p - e.yyx, n));
    vec3 g = (rgb_power * (mp - mm)) / e.x;
    return normalize(n - bf * g);                                       // Bumped normal. "bf" - bump factor.
}

//==============================================================================================================================================================
// Sophisticated tri-linear texturing and bump using tetrahedral derivative :: 12 texture samples
//==============================================================================================================================================================
vec3 deform_normal(in vec3 p, inout vec3 n)
{
    const float scale = 0.02875f; 
    const float delta = 0.01250f;
    const vec2 dp = vec2(delta, -delta); 
    const vec2 e = scale * dp; 
    const mat4x3 m = mat4x3(vec3( 1.0f, -1.0f, -1.0f), 
                            vec3(-1.0f, -1.0f,  1.0f), 
                            vec3(-1.0f,  1.0f, -1.0f), 
                            vec3( 1.0f,  1.0f,  1.0f));

    vec3 w = max(abs(n) - 0.317f, 0.0f);
    w /= dot(w, vec3(1.0f));            

    p *= scale;

    mat4x3 rgb_YZ = mat4x3(tex2d(p.yz + e.yy), tex2d(p.yz + e.yx), tex2d(p.yz + e.xy), tex2d(p.yz + e.xx));
    mat4x3 rgb_ZX = mat4x3(tex2d(p.zx + e.yx), tex2d(p.zx + e.xy), tex2d(p.zx + e.yy), tex2d(p.zx + e.xx));
    mat4x3 rgb_XY = mat4x3(tex2d(p.xy + e.xy), tex2d(p.xy + e.yy), tex2d(p.xy + e.yx), tex2d(p.xy + e.xx));

    mat4x3 rgb_samples = w.x * rgb_YZ + w.y * rgb_ZX + w.z * rgb_XY;

    vec4 luminosity = rgb_power * rgb_samples;
    vec3 gradient = m * luminosity;
    vec3 g = gradient - dot(gradient, n) * n;
    n = normalize(n - 8.0 * bf * g);

    return rgb_samples[0];
}

void main()
{
    vec3 n = normalize(normal_ws);
    vec3 b = bump_normal(position_ws, n);

    vec3 v = normalize(view);
    float light_distance = length(light);
    vec3 l = light / light_distance;                                      

    float diffuse_factor = 0.0f;
    float specular_factor = 0.0f;
    float cos_theta = dot(n, l);

    vec3 q = tex3d(n, b);
    vec3 ambient = Ka * q;
    vec3 diffuse = Kd * q;

    if (cos_theta > 0.0f) 
    {
        diffuse_factor = cos_theta;
        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, b), 0.0f);
        specular_factor = 0.125 * pow(cos_alpha, Ns);
    }
    vec3 color = ambient + diffuse_factor * diffuse + specular_factor * Ks;

    FragmentColor = vec4(color, 1.0f);
}