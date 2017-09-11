#version 330 core

in vec3 normal;
in vec3 tangent_x;
in vec3 tangent_y;
in vec3 view;
in vec3 light;
in vec2 uv;
in float hue;

uniform sampler2D diffuse_tex;
uniform sampler2D bump_tex;

out vec4 FragmentColor;


vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{

    mat3 frame = mat3(tangent_x, tangent_y, normal);
    vec3 normal_ms = vec3(texture(bump_tex, uv)) - vec3(0.5f, 0.5f, 0.0f);
	vec3 n = normalize(frame * normal_ms);
	vec3 v = normalize(view);
    float distance = length(light);
    vec3 l = light / distance;

    vec4 dtc4 = texture(diffuse_tex, uv);

    vec3 hsv = rgb2hsv(dtc4.rgb);
    hsv.x = mix(hue, hsv.x, 0.5);

    vec3 diffuse_color = hsv2rgb(hsv);
    float alpha = dtc4.a;


    vec3 ambient_color = 0.125f * diffuse_color;
    vec3 color = ambient_color;

    vec3 diffuse = sqrt(abs(dot(n, l))) * diffuse_color;
    vec3 h = normalize(l + v);

    const float Ks = 2.15;  
    const float Ns = 72.0;  
    float specular_factor = Ks * pow(abs(dot(n, h)), Ns);

    vec3 specular = specular_factor * vec3(1.0f);

    float attenuation = 1.0 / (1.0 + 0.115 * distance);

    color = color + attenuation * (diffuse + specular);

    FragmentColor = vec4(color, alpha * alpha * alpha);
}

