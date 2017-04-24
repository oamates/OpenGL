#version 430 core

in vec4 position;
in vec4 normal;
in vec4 view_direction;
in vec2 texture_coord_f;

uniform sampler2DMS texture_sampler;

uniform mat4 view_matrix;

const float two_pi = 6.283185307179586476925286766559f;
const float one_over_root2 = 0.70710678118654752440084436210485f;
const float one_over_root3 = 0.57735026918962576450914878050196f;
const float cube_size = 25.4;

const vec4 light_position[8] = 
{
    vec4(-cube_size, -cube_size, -cube_size, 1.0f),    
    vec4(-cube_size, -cube_size,  cube_size, 1.0f),    
    vec4(-cube_size,  cube_size, -cube_size, 1.0f),    
    vec4(-cube_size,  cube_size,  cube_size, 1.0f),    

    vec4( cube_size, -cube_size, -cube_size, 1.0f),    
    vec4( cube_size, -cube_size,  cube_size, 1.0f),    
    vec4( cube_size,  cube_size, -cube_size, 1.0f),    
    vec4( cube_size,  cube_size,  cube_size, 1.0f)
};


const vec4 light_color[8] = 
{
    vec4(one_over_root3, one_over_root3, one_over_root3, 1.0f),    
    vec4(one_over_root2, one_over_root2, 0.0f,           1.0f),    
    vec4(0.0f,           one_over_root2, one_over_root2, 1.0f),    
    vec4(one_over_root2, 0.0f,           one_over_root2, 1.0f),    
    vec4(1.0f,           0.0f,           0.0f,           1.0f),    
    vec4(0.0f,           1.0f,           0.0f,           1.0f),    
    vec4(0.0f,           0.0f,           1.0f,           1.0f),    
    vec4(one_over_root3, one_over_root3, one_over_root3, 1.0f)
};


out vec4 fragment_color;


void main()
{
	vec4 v = normalize(view_direction);															

	vec4 material_ambient_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    ivec2 texel_coord = ivec2(2048 * texture_coord_f.x, 2048 * texture_coord_f.y);
    float weight = 0.0f;	

	for (int k = 0; k < 8; ++k)
 	   for (int i = -3; i < 3; ++i)
          for (int j = -3; j < 3; ++j)
		  {
			 float factor = exp(- (i * i + j * j));
             material_ambient_color += factor * texelFetch(texture_sampler, texel_coord + ivec2(i, j), k);
             weight += factor;
          }
    material_ambient_color /= weight;

    vec4 material_specular_color = vec4(0.5f, 0.5f, 0.5f, 1.0f);

    fragment_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    vec4 specular_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 8; ++i)
    {
        float distance = length(light_position[i] - position);

        vec4 l = (light_position[i] - position) / distance;      // a direction to a source of light
        float lambert_cosine = dot(normal, l);
        if (lambert_cosine < 0.0f) lambert_cosine *= lambert_cosine;
	    vec4 r = reflect(l, normal);
    	float cos_alpha = dot(r, v);

        float ambient_distance_factor = 2 * clamp(1500.0f / (distance * distance), 0.0f, 1.0f);
        float specular_distance_factor = 2 * clamp(150.0f / (distance * distance), 0.0f, 1.0f);
        fragment_color += material_ambient_color * light_color[i] * lambert_cosine * lambert_cosine * ambient_distance_factor;
		specular_color += material_specular_color * light_color[i] * pow(cos_alpha, 20) * specular_distance_factor;
    };
    fragment_color += specular_color;
    fragment_color *= 2.0;
    fragment_color.w = 1.0;

};

