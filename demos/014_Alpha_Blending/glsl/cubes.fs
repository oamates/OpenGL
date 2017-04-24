#version 330 core

in vec4 position;
in vec4 color;
in vec4 normal;
in vec4 tangent_x;
in vec4 tangent_y;
in vec4 view_direction;
in vec2 texture_coord;

uniform sampler2D texture_sampler;
out vec4 FragmentColor;

#define two_pi 6.283185307179586476925286766559

const float cube_size = 16.4;

const vec4 light_position[8] = vec4[]
(
    vec4(-cube_size, -cube_size, -cube_size, 1.0f),    
    vec4(-cube_size, -cube_size,  cube_size, 1.0f),    
    vec4(-cube_size,  cube_size, -cube_size, 1.0f),    
    vec4(-cube_size,  cube_size,  cube_size, 1.0f),    

    vec4( cube_size, -cube_size, -cube_size, 1.0f),    
    vec4( cube_size, -cube_size,  cube_size, 1.0f),    
    vec4( cube_size,  cube_size, -cube_size, 1.0f),    
    vec4( cube_size,  cube_size,  cube_size, 1.0f)
);

const vec4 light_color[8] = vec4[]
(
    vec4(1.0f, 1.0f, 0.0f, 1.0f),    
    vec4(1.0f, 1.0f, 0.0f, 1.0f),    
    vec4(1.0f, 1.0f, 0.0f, 1.0f),    
    vec4(1.0f, 1.0f, 0.0f, 1.0f),    

    vec4(1.0f, 1.0f, 0.0f, 1.0f),    
    vec4(1.0f, 1.0f, 0.0f, 1.0f),    
    vec4(1.0f, 1.0f, 0.0f, 1.0f),    
    vec4(1.0f, 1.0f, 0.0f, 1.0f)
);

vec4 procedural_texture_ambient(vec2 p)
{
    float x = p.x;
    float y = p.y;
    float grey_value = (2.0f + cos(two_pi*x) + cos(two_pi*y) ) / 4.0;
    return vec4(grey_value, grey_value, grey_value, 1.0f);    
}

vec4 procedural_texture_diffuse(vec2 p)
{
    return vec4(0.1, 0.1, 0.1, 1.0);
}

vec4 procedural_texture_specular(vec2 p)
{
    return vec4(0.1, 0.1, 0.1, 1.0);
}


void main()
{

    vec3 normal_ms = vec3(texture(texture_sampler, texture_coord)) - vec3(0.5, 0.5, 0.5);
    normal_ms.z = clamp(normal_ms.z, 0.0, 1.0);
    normal_ms.z = clamp(normal_ms.z, 0.0, 1.0);

	vec4 n = normalize(normal_ms.x * tangent_x + normal_ms.y * tangent_y + normal_ms.z * normal);   // normal vector to fragment
	vec4 v = normalize(view_direction);															


    vec4 material_ambient_color = (vec4 (normal_ms, 0.0) + procedural_texture_ambient(texture_coord))/2.0;
    vec4 material_diffuse_color = procedural_texture_diffuse(texture_coord);
    vec4 material_specular_color = procedural_texture_specular(texture_coord);

    FragmentColor = vec4(0.0, 0.0, 0.0, 0.0);

    float inv_sqr = 0.0;

    for (int i = 0; i < 8; ++i)
    {
        float distance = length(light_position[i] - position);

        vec4 l = (light_position[i] - position) / distance;
        float lambert_cosine = clamp(dot(n,l), 0.0f, 1.0f);

	    vec4 r = reflect(l, n);
    	float cos_alpha = clamp(dot(r,v), 0.0f, 1.0f);															
        FragmentColor += material_diffuse_color * light_color[i] * 1000.0 * lambert_cosine / (distance * distance) +
                         material_specular_color * light_color[i] * 100.0 * pow(cos_alpha, 10) / (distance * distance);
        inv_sqr += 1 / distance;
    }        
    FragmentColor += clamp(inv_sqr, 0.0, 1.0) * (material_ambient_color + color);
    FragmentColor.w = 0.5;
}

