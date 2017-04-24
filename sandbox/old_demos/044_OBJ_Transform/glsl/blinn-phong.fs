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

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;
uniform float d;
uniform float bm;

uniform sampler2D map_Ka;           // ambient texture
uniform sampler2D map_Kd;           // diffuse texture
uniform sampler2D map_Ks;           // specular texture
uniform sampler2D map_Ns;           // specular shininess texture
uniform sampler2D map_bump;         // bump texture
uniform sampler2D map_d;            // mask texture


uniform int texture_flags;                     // logical combination of texture presence flags below

const int AMBIENT_TEXTURE_PRESENT   = 0x01;
const int DIFFUSE_TEXTURE_PRESENT   = 0x02;
const int SPECULAR_TEXTURE_PRESENT  = 0x04;
const int SHININESS_TEXTURE_PRESENT = 0x08;

const int HEIGHTMAP_TEXTURE_PRESENT = 0x10;    // these two flags are mutually exclusive : heightmap is a grayscale texture, normal map is an rgb-one
const int NORMALMAP_TEXTURE_PRESENT = 0x20;    // the value is read from map_bump sampler, the application should set filtering mode to GL_LINEAR
                                               // if both bits are 0, unperturbed normals are used
const int MASK_TEXTURE_PRESENT      = 0x40;



vec3 ambient_color()
{
    if ((texture_flags & AMBIENT_TEXTURE_PRESENT) == 0) 
        return Ka;
    return Ka * texture(map_Ka, uv).rgb;
}

vec3 diffuse_color()
{
    if ((texture_flags & DIFFUSE_TEXTURE_PRESENT) == 0)
        return Kd;
    return Kd * texture(map_Kd, uv).rgb;
}

vec3 specular_color()
{
    if ((texture_flags & SPECULAR_TEXTURE_PRESENT) == 0)
        return Ks;
    return Ks * texture(map_Ks, uv).rgb;
}

float specular_exponent()
{
    if ((texture_flags & SHININESS_TEXTURE_PRESENT) == 0)
        return Ns;
    return Ns * texture(map_Ns, uv).r;
}


vec3 compute_normal()
{
    if ((texture_flags & (HEIGHTMAP_TEXTURE_PRESENT | NORMALMAP_TEXTURE_PRESENT)) == 0) 
        return normalize(normal);

    if ((texture_flags & NORMALMAP_TEXTURE_PRESENT) == 0)       // we got heightmap bump texture
    {
        vec3 Px = dFdx(position);
        vec3 Py = dFdy(position);
        vec3 tX = cross(Py, normal);
        vec3 tY = cross(normal, Px);
        float det = dot(Px, tX);
        vec2 UVx = dFdx(uv);
        vec2 UVy = dFdy(uv);
        float Bx = texture(map_bump, uv + 0.5f * UVx).r - texture(map_bump, uv - 0.5f * UVx).r;
        float By = texture(map_bump, uv + 0.5f * UVy).r - texture(map_bump, uv - 0.5f * UVy).r;
        vec3 surface_gradient = 0.5f * bm * sign(det) * (Bx * tX + By * tY);
        return normalize(abs(det) * normal - surface_gradient);
    }

    vec3 Px = dFdx(position);                                   // we got 3-component normalmap texture
    vec3 Py = dFdy(position);
    vec2 UVx = dFdx(uv);
    vec2 UVy = dFdy(uv);
 
    vec3 tangent_y = cross(Py, normal);                         
    vec3 tangent_x = cross(normal, Px);
    vec3 T = tangent_x * UVx.x + tangent_y * UVy.x;             // tangent 
    vec3 B = tangent_x * UVx.y + tangent_y * UVy.y;             // binormal
 
    float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
    vec3 components = texture(map_bump, uv).rgb - vec3(0.5f);
    return normalize(invmax * (T * components.x + B * components.y) + normal * components.z);
}

out vec4 FragmentColor;

void main()
{
    if (((texture_flags & MASK_TEXTURE_PRESENT) != 0) && (d * texture2D(map_d, uv).r < 0.5f)) discard;

    vec3 n = compute_normal();
    vec3 v = normalize(view_direction);                                                         

    float light_distance = length(light_direction);
    vec3 l = light_direction / light_distance;                                      


    float diffuse_factor = 0.0f;
    float specular_factor = 0.0f;

    float cos_theta = dot(n, l);

    if (cos_theta > 0.0f) 
    {
        diffuse_factor = cos_theta;

        // Phong lighting
        // vec3 r = reflect(-l, n);
        // float cos_alpha = max(dot(v, r), 0.0f);
        // float exponent = 0.25f * specular_exponent();
        
        // Blinn - Phong lighting
        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, n), 0.0f);
        float exponent = specular_exponent();

        specular_factor = pow(cos_alpha, exponent);
    }
    vec3 color = ambient_color()
                    + diffuse_factor * diffuse_color()
                    + specular_factor * specular_color();

    FragmentColor = vec4(color, 1.0f);

//    FragmentColor = vec4(texture2D(map_Ka, uv).rgb, 1.0f);
//    FragmentColor = vec4(texture2D(map_Kd, uv).rgb, 1.0f);
//    FragmentColor = vec4(texture2D(map_Ks, uv).rgb, 1.0f);

}

