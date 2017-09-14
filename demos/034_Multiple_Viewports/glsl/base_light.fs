#version 410 core

uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex;

in GS_FS_VERTEX
{
    vec3 view_direction;
    vec3 light_direction;
    vec3 normal_direction;
    vec3 tangent_x_direction;
    vec3 tangent_y_direction;
    vec2 texture_coord;
} vertex_in;

out vec4 FragmentColor;

void main()
{
	
	float light_distance = length(vertex_in.light_direction);
	vec3 l = vertex_in.light_direction / light_distance; 										

	vec3 components = texture(normal_tex, vertex_in.texture_coord).xyz - vec3(0.5f, 0.5f, 0.0f);

	vec3 n = normalize(components.x * vertex_in.tangent_x_direction
	                 + components.y * vertex_in.tangent_y_direction
	                 + components.z * vertex_in.normal_direction);

	vec3 e = normalize(vertex_in.view_direction);															
	vec3 r = reflect(l, n);

	float dp = dot(n, l);
	float cos_theta = clamp(dp, 0.0f, 1.0f);
	float cos_alpha = 0.0f;
	if (dp > 0.0f) cos_alpha = clamp(dot(e, r), 0.0f, 1.0f);														

    vec4 material_diffuse_color = texture(diffuse_tex, vertex_in.texture_coord);
	vec4 material_ambient_color = 0.1 * material_diffuse_color;
	vec4 material_specular_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	FragmentColor = material_ambient_color + 
					100.0f * material_diffuse_color * cos_theta / light_distance
                  + (55.0f * material_specular_color * pow(cos_alpha, 12) / light_distance);                 
}





