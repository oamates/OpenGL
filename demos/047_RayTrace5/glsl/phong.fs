#version 410 core

struct Material
{
	vec3 color_diffuse;
	vec3 color_specular;
	float reflectance; // [0, 1]
	float specular_reflectance; // [0, 1], part of reflectance
	float specular_polish; // [0, 1]
};

struct LightSource
{
	float intensity;
	vec3 color;
	vec3 position;
};

in vec3 normal_cs;
in vec3 normal_ws;
in vec3 position_cs;
in vec3 position_ws;

uniform Material material;
uniform LightSource light;

out vec4 FragmentColor;


vec3 calculateLocalDiffuse()
{
	vec3 res = vec3(0,0,0);
	vec3 n = normalize(normal_ws);

	vec3 light_diff = position_ws - light.position;
	float light_dist = length(light_diff);
	vec3 l = normalize(light_diff);

	float cosTheta = dot(n, -l);
	vec3 diffuse = light.color * light.intensity * max(cosTheta, 0.0f) / (light_dist * light_dist);

	vec3 ambient = vec3(0.0f);
	res = ambient + diffuse;
	return res;
}

void main()
{
    vec3 color = calculateLocalDiffuse() * material.color_diffuse * material.reflectance * (1 - material.specular_reflectance);
    color = pow(color, vec3(1.0 / 2.6));
	FragmentColor = vec4(color, 1.0f);
}