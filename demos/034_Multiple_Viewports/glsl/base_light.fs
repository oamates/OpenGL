#version 410 core

uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex;

in GS_FS_VERTEX
{
    vec3 view;
    vec3 light;
    vec3 normal;
    vec3 tangent_x;
    vec3 tangent_y;
    vec2 uv;
} vertex_in;

out vec4 FragmentColor;

void main()
{
    
    float light_distance = length(vertex_in.light);
    vec3 l = vertex_in.light / light_distance;                                        

    vec3 b = texture(normal_tex, vertex_in.uv).xyz - vec3(0.5f, 0.5f, 0.5f);

    vec3 n = normalize(b.x * vertex_in.tangent_x
                     + b.y * vertex_in.tangent_y
                     + b.z * vertex_in.normal);

    vec3 e = normalize(vertex_in.view);                                                           
    vec3 r = reflect(l, n);

    float dp = dot(n, l);
    float cos_theta = clamp(dp, 0.0f, 1.0f);
    float cos_alpha = 0.0f;
    if (dp > 0.0f) cos_alpha = clamp(dot(e, r), 0.0f, 1.0f);                                                        

    vec3 diffuse_color = texture(diffuse_tex, vertex_in.uv).rgb;
    vec3 ambient_color = 0.25 * diffuse_color;
    vec3 specular_color = vec3(1.0f);

    vec3 color = ambient_color
              + 10.0f * diffuse_color * cos_theta / (1.0f + 0.25 * light_distance)
              + 5.0f * specular_color * pow(cos_alpha, 12) / (1.0f + 0.25 * light_distance);                 

    FragmentColor = vec4(color, 1.0f);
}





