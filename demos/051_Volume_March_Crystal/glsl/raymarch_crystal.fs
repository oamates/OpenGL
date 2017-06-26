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
    vec3 w = vec3(0.331);
    mat3 rgb_samples = mat3(tex2d(p.yz), tex2d(p.zx), tex2d(p.xy));
    return pow(rgb_samples * w, vec3(1.5));
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

float map(vec3 p)
{
    p *= 0.341;
    float res = 0.0f;    
    vec3 c = p;
    for (int i = 0; i < 4; ++i)
    {
        p = 0.7f * abs(p) / dot(p, p) - 0.7f;
        p.yz = csqr(p.yz);
        p = p.zxy;
        res += exp(-18.0f * abs(dot(p, c)));
        
    }
    return 0.5f * res;
}

//==============================================================================================================================================================
// Tetrahedral normal
//==============================================================================================================================================================
vec4 map_bump(in vec3 p)
{
    vec3 e = vec3(0.001, -0.001, 0.25);
    vec4 b = e.xyyz * map(p + e.xyy) + e.yyxz * map(p + e.yyx) + e.yxyz * map(p + e.yxy) + e.xxxz * map(p + e.xxx);
    b.xyz = normalize(b.xyz);
    return b;
}
vec4 raymarch(in vec3 front, in vec3 back)
{
    float alpha = 0.25;
    vec3 col = vec3(0.0f);
    vec3 dir = front - back;
    float l = length(dir);

    dir = dir / l;
    l = min(2.75, l);

    const float step = 0.05725;
    float t = step * (fract(l / step) - 1.01);

    while(t <= l)
    {
        vec3 p = back + t * dir;
        //float c = map(p);
        vec4 c = map_bump(p);
        float d = (abs(c.w) - 0.75f);
        vec3 l = normalize(light_ws - p);
        vec3 h = normalize(dir - l);
        float s = abs(dot(c.xyz, h));

        if (d > 0.0)
        {
            float q = smoothstep(0.0, 1.0, d);
            q *= q;

            vec3 b = tex3d(p);



            b += 2.13 * pow(s, 20.0);
            col = mix(col, b, q);
            alpha = max(alpha, q);

        }
        else if(d > -0.25) 
        {
            float q = smoothstep(0.0, 1.0, 2.41 * (d + 0.25));
            q *= q;

            vec3 b = vec3(0.475, 0.561, 0.783);
            b += 2.13 * pow(s, 20.0);
            col = mix(col, b, q);
            alpha = max(alpha, q);



        }
        t += step;
    }

    return vec4(pow(col, vec3(0.92f, 0.67f, 0.51f)), alpha);
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

    vec4 color = raymarch(position_ws, backface_ws);
    vec3 diffuse = (0.8 + 0.2 * dot(n, l)) * color.rgb;

    vec3 h = normalize(l + v);  
    const float Ks = 0.85f;
    const float Ns = 70.0f;
    float specular = Ks * pow(max(dot(n, h), 0.0), Ns);

    vec3 c = diffuse + vec3(specular);
    FragmentColor = vec4(c, color.a);
}





