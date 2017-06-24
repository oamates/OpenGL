#version 330 core

in vec3 position_ws;
in vec3 normal_ws;

uniform sampler2D backface_depth_tex;
uniform sampler2D tb_tex;

uniform mat3 camera_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform vec2 focal_scale;
uniform vec2 inv_resolution;

out vec4 FragmentColor;

vec3 tex2d(vec2 uv)
{
    return texture(tb_tex, uv).rgb;
}

vec3 tex3d(in vec3 p)
{
    p *= 1.4875;
    vec3 w = vec3(0.577);
    mat3 rgb_samples = mat3(tex2d(p.yz), tex2d(p.zx), tex2d(p.xy));
    return sqrt(rgb_samples * w);
}

float csZ(float depth)
{                                
    const float znear = 0.5;
    return znear / (depth - 1.0);
}

vec3 position_cs(vec2 uv)
{
    vec2 ndc = 2.0 * uv - 1.0;
    vec3 view = vec3(focal_scale * ndc, -1.0f);
    float depth = texture(backface_depth_tex, uv).r;
    float Z = csZ(depth);
    vec3 p = -Z * view;
    return p;
}

vec2 csqr(vec2 a)
{
    return vec2(a.x * a.x - a.y * a.y, 2.0f * a.x * a.y); 
}

float map(in vec3 p)
{    
    float res = 0.0f;    
    vec3 c = p;
    for (int i = 0; i < 5; ++i)
    {
        p = 0.7f * abs(p) / dot(p, p) - 0.7f;
        p.yz = csqr(p.yz);
        p = p.zxy;
        res += exp(-19.0f * abs(dot(p, c)));
        
    }
    return 0.5f * res;
}

vec3 raymarch(in vec3 b, in vec3 e)
{
    vec3 col = vec3(0.0f);
    const int iterations = 80;
    for(int i = 0; i <= iterations; ++i)
    {
        vec3 p = mix(e, b, float(i) / float(iterations));
        float c = map(p);

        float d = abs(c) - 0.65f;
        if (d > 0.0f)
        {
            vec3 q = tex3d(0.341 * p);
            col = mix(1.05 * col, q, 0.041 * d);
        }
    }

    //return log(1.0f + 1.175 * col);
    return pow(col, vec3(0.22f));
}

void main()
{
    vec2 uv = gl_FragCoord.xy * inv_resolution;

    vec3 n = normalize(normal_ws);
    vec3 light = light_ws - position_ws;
    vec3 view = camera_ws - position_ws;
    vec3 l = normalize(light);
    vec3 v = normalize(view);

    vec3 backface_ws = camera_ws + camera_matrix * position_cs(uv);

    vec3 color = raymarch(position_ws, backface_ws);
    vec3 diffuse = (0.8 + 0.2 * dot(n, l)) * color;

    vec3 h = normalize(l + v);  
    const float Ks = 1.15f;
    const float Ns = 64.0f;
    float specular = Ks * pow(max(dot(n, h), 0.0), Ns);

    vec3 c = diffuse + vec3(specular);
    FragmentColor = vec4(c, 1.0f);
}





