#version 330 core

in vec3 position_ws;
in vec3 normal_ws;
in vec2 uv;

uniform vec3 camera_ws;
uniform float time;

uniform sampler2D water_texture;
out vec4 FragmentColor;

const vec3 light = normalize(vec3(0.0, 0.707, 0.707));
const vec3 OCEAN_DEPTH_COLOR = vec3(0.032f, 0.113f, 0.138f);
const vec3 OCEAN_WATER_COLOR = vec3(0.247f, 0.317f, 0.296f);
const vec3 OCEAN_SPECULAR_COLOR = vec3(0.79f, 0.91f, 0.93f);
const float OCEAN_BASE_AMPLITUDE = -0.15;
const vec3 FOG_COLOR = vec3(0.91f, 0.95f, 0.97f);
const float FOG_RADIUS = 1536.0;

float noise(vec2 P) 
{
    const float FACTOR_X = 127;
    const float FACTOR_Y = 311;
    const vec4 hash = vec4(0, FACTOR_X, FACTOR_Y, FACTOR_X + FACTOR_Y);

    vec2 Pi = floor(P);
    vec2 Pf = P - Pi;

    vec4 h = dot(Pi, hash.yz) + hash;
    vec2 Ps = Pf * Pf * (3.0 - 2.0 * Pf);
    h = fract(sin(h) * 43758.5453123);

    vec2 val = mix(h.xy, h.zw, Ps.y);
    return -1.0 + 2.0 * mix(val.x, val.y, Ps.x);
}

vec3 sky_color(vec3 e)
{
    const mat2 octave_matrix = mat2(1.272, 1.696, -1.696, 1.272);
    const vec3 CLOUD_COLOR = vec3(0.81, 0.89, 0.97);
    vec2 uv = e.xy / (abs(e.z) + 0.001);
    uv *= 2.313;
    uv += vec2(0.01527, 0.00948) * time;
    float fbm = noise(uv);

    float amplutide = 1.0;
    for (int i = 0; i < 6; ++i)
    {
        uv *= octave_matrix;
        amplutide *= 0.45f;
        fbm += amplutide * noise(uv);
    }

    float cloud_factor = smoothstep(0.0, 1.3, fbm) * exp(-0.00125 * length(uv));
    float q = abs(e.z);
    vec3 qq = vec3(1.0f - q, 1.0f - q, 1.0f - 0.5 * q);
    return pow(FOG_COLOR * qq, vec3(2.0f, 1.5f, 1.0f)) + cloud_factor * CLOUD_COLOR;
}

void main (void)
{

	vec3 p = position_ws;
	vec3 n = normalize(normal_ws);
	vec3 view = position_ws - camera_ws;
	float t = length(view);
	vec3 e = view / t;

    //==========================================================================================================================================================
    // reflected sky color
    //==========================================================================================================================================================
    float dp = dot(e, n);
    vec3 l = e - 2.0 * dp * n;						// direction of the reflected light coming to the sky
    vec3 reflected_sky = sky_color(l);



    //==========================================================================================================================================================
    // refracted water color
    //==========================================================================================================================================================
	dp = min(dp, 0.0f);
    const float eta = 0.75f;									// air - water refraction coefficient
    float ss = 1.0f - dp * dp;									// sine squared of the incidence angle
    float k = 1.0f - ss * eta * eta;  							// sine squared of the refraction angle
    float g = sqrt(k) / eta;									// fresnel koefficient
    float f = 0.5 * pow((g + dp) / (g - dp), 2.0f) * (1.0 + pow((dp * (g - dp) + 1.0f) / (dp * (g + dp) - 1.0f), 2.0f));

    vec3 water_color = mix(texture(water_texture, 0.0625 * uv).rgb * vec3(1.0, 1.15, 0.75), OCEAN_WATER_COLOR, f);

    //==========================================================================================================================================================
    // the larger the angle between normal and refracted ray, the lighter the water 
    //==========================================================================================================================================================
    float lambda = pow(smoothstep(1.0f - eta * eta, 1.0, k), 0.36);
    vec3 refracted_water = mix(water_color, OCEAN_DEPTH_COLOR, lambda);
    vec3 color = mix(refracted_water, reflected_sky, f);
    
    //==========================================================================================================================================================
    // attenuate with exponential decay water above average ocean level 
    //==========================================================================================================================================================
    float attenuation = 0.0225 * exp(-abs(0.007 * t));
    color += water_color * (p.z - OCEAN_BASE_AMPLITUDE) * attenuation;

    //==========================================================================================================================================================
    // add specular component : standard Phong term
    //==========================================================================================================================================================
    float specular_power = 0.327f * pow(max(dot(l, light), 0.0), 100.0);
    color += specular_power * OCEAN_SPECULAR_COLOR;

    FragmentColor = vec4(pow(color, vec3(0.75)), 1.0);
}