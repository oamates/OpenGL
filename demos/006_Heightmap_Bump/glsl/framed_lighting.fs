#version 430 core

uniform sampler2D diffuse_texture;
uniform sampler2D bump_texture; 

in vec3 position_ws;
in vec3 normal_ws;
in vec3 tangent_x;
in vec3 tangent_y;
in vec2 uv;
in vec3 view;
in vec3 light;
in float hue;

out vec4 FragmentColor;

const float Ns = 20.0f;

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
    vec3 l = normalize(light);

    vec2 UVx = dFdx(uv);
    vec2 UVy = dFdy(uv);
    float Bx = texture(bump_texture, uv + 0.5f * UVx).r - texture(bump_texture, uv - 0.5f * UVx).r;
    float By = texture(bump_texture, uv + 0.5f * UVy).r - texture(bump_texture, uv - 0.5f * UVy).r;

    const float bm = 0.5f;
    vec3 surface_gradient = 0.5f * bm * (Bx * tangent_x + By * tangent_y);

    vec3 n = normalize(normal_ws - surface_gradient);
    vec3 v = normalize(view);                                                         
    
    vec3 rgb = texture(diffuse_texture, uv).rgb;
    vec3 hsv = rgb2hsv(rgb);
    hsv.x = hue;
    hsv.y = pow(hsv.y, 0.25);
    vec3 diffuse_color = hsv2rgb(hsv);
    vec3 ambient_color = 0.25f * diffuse_color;
    vec3 specular_color = hsv2rgb(vec3(0.0f, hsv.yz));

    float cos_theta = dot(n, l);

    vec3 color = ambient_color;

    if (cos_theta > 0.0f) 
    {
        color += cos_theta * diffuse_color;

        // Phong lighting
//        vec3 r = reflect(-l, n);
//        float cos_alpha = max(dot(v, r), 0.0f);
//        float exponent = 0.25f * specular_exponent();
        
        // Blinn - Phong lighting
        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, n), 0.0f);

        color += pow(cos_alpha, Ns) * specular_color;
    }

    FragmentColor = vec4(color, 1.0f);                 
}




