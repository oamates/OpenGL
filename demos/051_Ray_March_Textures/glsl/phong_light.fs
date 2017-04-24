#version 330 core

uniform vec3 light_ws;
uniform vec3 camera_ws;
uniform float time;

in vec3 position_ms;
in vec3 camera_ms;
in vec3 position;
in vec3 normal;
in vec3 tangent_x;
in vec3 tangent_y;
in vec2 uv;

out vec4 FragmentColor;

const float Ns = 80.0f;

vec2 csqr(vec2 a )
{
    return vec2(a.x * a.x - a.y * a.y, 2.0f * a.x * a.y); 
}

float map(in vec3 p)
{    
    float res = 0.0f;    
    vec3 c = p;
    for (int i = 0; i < 12; ++i)
    {
        p = 0.7f * abs(p) / dot(p, p) - 0.7f;
        p.yz = csqr(p.yz);
        p = p.zxy;
        res += exp(-19.0f * abs(dot(p, c)));
        
    }
    return 0.5f * res;
}

vec3 raymarch(in vec3 ro, vec3 rd, float tmin)
{
    float t = tmin;
    float dt = 0.02f;
    // float dt = 0.2f - 0.195f * cos(time * 0.05f);                        // animated

    vec3 col = vec3(0.0f);
    float c = 0.0f;
    for( int i = 0; i < 32; i++)
    {
        t += dt * exp(-2.0f * c);
        c = map(ro + t * rd);               
//        col = 0.99f * col + 0.08f * vec3(c * c, c, c * c * c);            // green  
//        col = 0.93f * col + 0.08f * vec3(c * c, c, c * c * c);     // green + yellow
        col = 0.99f * col + 0.08f * vec3(c * c * c, c * c, c);            // blue
    }    
    return log(1.0f + 0.175 * col);
}

void main()
{
    
    vec3 n = normalize(normal);
    vec3 l = normalize(light_ws - position);
    vec3 v = normalize(camera_ws - position);

    vec3 ms_v = normalize(position_ms - camera_ms);
    
    vec3 diffuse_color = raymarch(position_ms, ms_v, 0.0f);
    vec3 ambient_color = 0.125f * diffuse_color;
    vec3 specular_color = vec3(0.5f);


    float cos_theta = dot(n, l);

    vec3 color = ambient_color;

    if (cos_theta > 0.0f) 
    {
        color += cos_theta * diffuse_color;

        // Phong lighting
        //vec3 r = reflect(-l, n);
        //float cos_alpha = max(dot(v, r), 0.0f);
        
        // Blinn - Phong lighting
        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, n), 0.0f);

        color += pow(cos_alpha, Ns) * specular_color;
    }

    FragmentColor = vec4(color, 1.0f);                 
}





