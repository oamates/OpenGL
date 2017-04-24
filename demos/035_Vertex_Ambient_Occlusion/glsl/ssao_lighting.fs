#version 330 core

out vec4 FragmentColor;
in vec2 uv;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D ssao;

uniform vec3 light_cs;
uniform int draw_mode;

const vec3 light_color    = vec3(0.172f, 0.211f, 0.871f);
const vec3 specular_color = vec3(0.707f, 0.707f, 0.707f);

const float two_pi = 6.28318530717958647692528676655900576839;

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

uniform float hue_shift;

void main()
{             
    vec4  position_occlusion = texture(gPosition, uv);
    vec3  position_cs        = position_occlusion.xyz;
    float vertex_occlusion   = position_occlusion.w / two_pi;
    vertex_occlusion = mix(vertex_occlusion, 1.0f, 0.85f);

    vec4 normal_hue = texture(gNormal, uv);
    vec3 normal_cs = normal_hue.xyz;
    float hue = normal_hue.w;

    if(draw_mode == 1)
    {
        FragmentColor = vec4(vec3(vertex_occlusion), 1.0f);
        return;
    }

    if(draw_mode == 2)
    {
        FragmentColor = vec4(texture(ssao, uv).rrr, 1.0f);
        return;
    }

    if(draw_mode == 3)
    {
        FragmentColor = vec4(hue, hue, hue, 1.0f);
        return;
    }

    float ambient_occlusion  = texture(ssao, uv).r;


    vec3 ambient_color = hsv2rgb(vec3(hue_shift + 3.0 * hue, 0.5 * vertex_occlusion, 0.2 * ambient_occlusion));
    vec3 diffuse_color = hsv2rgb(vec3(hue_shift + 3.0 * hue, 0.8 * vertex_occlusion, ambient_occlusion));

    
    
    vec3 v = normalize(-position_cs);
    float distance = length(light_cs - position_cs);
    vec3 l = (light_cs - position_cs) / distance;
    vec3 h = normalize(l + v);  
    float specular_power = 0.25f * pow(max(dot(normal_cs, h), 0.0), 8.0);

    vec3 ambient = ambient_color * vertex_occlusion;
    vec3 diffuse = max(dot(normal_cs, l), 0.0) * diffuse_color * light_color * vertex_occlusion;
    vec3 specular = specular_color * light_color * specular_power;

    float attenuation = 1.0 / (1.0 + 0.03f * distance + 0.012f * distance * distance);
    diffuse  *= attenuation;
    specular *= attenuation;

    FragmentColor = vec4(pow(ambient + diffuse + specular, vec3(1.0f / 2.2f)), 1.0);
}
