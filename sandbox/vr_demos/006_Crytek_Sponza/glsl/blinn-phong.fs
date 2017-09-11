#version 330 core

in vec3 view_direction;
in vec3 light_direction;
in vec3 position;
in vec3 normal;
in vec2 uv;

layout (std140) uniform matrices
{
    mat4 projection_view_matrix;
    mat4 projection_matrix;
    mat4 view_matrix;
    mat4 camera_matrix;
};

uniform sampler2D map_Kd;           // diffuse texture
uniform sampler2D map_bump;         // bump texture
uniform sampler2D map_d;         // bump texture

// uniform sampler2D map_Ka;     // ambient texture
// uniform sampler2D map_Ks;     // specular texture
// uniform sampler2D map_Ns;     // specular shininess texture
// uniform sampler2D map_d;      // mask texture

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;
uniform float d;
uniform int mask_texture;

out vec4 FragmentColor;

void main()
{
    if ((mask_texture != 0) && (d * texture2D(map_d, uv)).r < 0.5f) discard;

    vec3 Px = dFdx(position);
    vec3 Py = dFdy(position);

    vec3 Nx = dFdx(normal);
    vec3 Ny = dFdy(normal);

    vec2 UVx = dFdx(uv);
    vec2 UVy = dFdy(uv);

    float delta = 0.00125f;
    float inv = 0.5f / delta;

    float B = texture2D(map_bump, uv).x - 0.5f;
    float Bu = inv * (texture2D(map_bump, uv + vec2(delta, 0.0f)).r - texture2D(map_bump, uv - vec2(delta, 0.0f)).r);
    float Bv = inv * (texture2D(map_bump, uv + vec2(0.0f, delta)).r - texture2D(map_bump, uv - vec2(0.0f, delta)).r);
    float Bx = Bu * UVx.x + Bv * UVx.y;
    float By = Bu * UVy.x + Bv * UVy.y;

    vec3 Pxx = Px + Bx * normal + B * Nx; 
    vec3 Pyy = Py + By * normal + B * Ny; 


    vec3 n = normalize(cross(Pxx, Pyy));

    float light_distance = length(light_direction);

    vec3 l = light_direction / light_distance;                                      
    vec3 e = normalize(view_direction);                                                         
    vec3 r = reflect(l, n);

    float dp = dot(n, l);
    float cos_theta = clamp(dp, 0.0f, 1.0f);
    float cos_alpha = 0.0f;
    if (dp > 0.0f) cos_alpha = clamp(dot(e, r), 0.0f, 1.0f);                                                        

    vec3 diffuse_color = Kd * texture2D(map_Kd, uv).rgb;
    vec3 ambient_color = 0.075 * Ka * diffuse_color;
    vec3 specular_color = Ks;

    FragmentColor.rgb = ambient_color + 
                    200.0f * diffuse_color * cos_theta / light_distance
                  + (165.0f * specular_color * pow(cos_alpha, 12) / light_distance);

    FragmentColor.a = 1.0f;

//    FragmentColor = vec4(texture2D(bump_texture, uv).xxx, 1.0f);

}

