#version 430 core

in vec3 position_ws;
in float intensity;

uniform sampler2D tb_tex2d;

uniform vec3 camera_ws;
uniform vec3 light_ws;

const vec3 Ka = vec3(1.0f);
const vec3 Kd = vec3(1.0f);
const vec3 Ks = vec3(1.0f);
const float Ns = 20.0f;
const float bf = 0.125;

out vec4 FragmentColor;

//==============================================================================================================================================================
// canyon distance function
//==============================================================================================================================================================
vec3 tri(in vec3 x)
{
    vec3 q = abs(fract(x) - 0.5f);
    return q;
}

float sdf(vec3 p)
{    
    vec3 op = tri(1.1f * p + tri(1.1f * p.zxy));
    float ground = p.z + 3.5 + dot(op, vec3(0.067));
    p += (op - 0.25) * 0.3;
    p = cos(0.444f * p + sin(1.112f * p.zxy));
    float canyon = (length(p) - 1.05) * 0.95;
    return min(ground, canyon);
}

//==============================================================================================================================================================
// sdf gradient :: tetrahedral evaluation
//==============================================================================================================================================================
vec3 grad(in vec3 p)
{
    vec2 e = vec2(0.0125, -0.0125);
    return normalize(e.xyy * sdf(p + e.xyy) + e.yyx * sdf(p + e.yyx) + e.yxy * sdf(p + e.yxy) + e.xxx * sdf(p + e.xxx));
}

//==============================================================================================================================================================
// Straightforward tri-linear texturing and bump :: 18 texture samples
//==============================================================================================================================================================
const vec3 rgb_power = vec3(0.299f, 0.587f, 0.114f);

vec3 tex2d(vec2 uv)
{
    return texture(tb_tex2d, uv).rgb;
}

vec3 tex3d(in vec3 p, in vec3 n)
{
    p *= 0.04875;
    vec3 w = max(abs(n) - 0.317f, 0.0f);
    w /= dot(w, vec3(1.0f));
    mat3 rgb_samples = mat3(tex2d(p.yz), tex2d(p.zx), tex2d(p.xy));
    return rgb_samples * w;
}

vec3 bump_normal(in vec3 p, in vec3 n)
{
    const vec2 e = vec2(0.0625, 0);
    mat3 mp = mat3(tex3d(p + e.xyy, n), tex3d(p + e.yxy, n), tex3d(p + e.yyx, n));
    mat3 mm = mat3(tex3d(p - e.xyy, n), tex3d(p - e.yxy, n), tex3d(p - e.yyx, n));
    vec3 g = (rgb_power * (mp - mm)) / e.x;
    return normalize(n - bf * g);                                       // Bumped normal. "bf" - bump factor.
}



void main(void)
{
	vec3 normal_ws = grad(position_ws);
  	vec3 view = camera_ws - position_ws;
  	vec3 light = light_ws - position_ws;

    vec3 n = bump_normal(position_ws, normal_ws);
    vec3 q = tex3d(position_ws, normal_ws);

    vec3 v = normalize(view);
    float light_distance = length(light);
    vec3 l = light / light_distance;                                      

    float diffuse_factor = 0.0f;
    float specular_factor = 0.0f;
    float cos_theta = dot(n, l);

    vec3 ambient = Ka * q;
    vec3 diffuse = Kd * q;

    if (cos_theta > 0.0f) 
    {
        diffuse_factor = cos_theta;

        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, n), 0.0f);
        float exponent = Ns;

        specular_factor = 0.125 * pow(cos_alpha, exponent);
    }
    vec3 color = ambient + diffuse_factor * diffuse + specular_factor * Ks;

    FragmentColor = vec4(color, 1.0f);
 
}