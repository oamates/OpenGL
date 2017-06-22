#version 330 core

in vec3 position;
in vec3 normal;
in vec3 tangent_x;
in vec3 tangent_y;
in vec3 view;
in vec3 light;
in vec2 uv;

uniform sampler2D diffuse_tex;
uniform sampler2D bump_tex;

out vec4 FragmentColor;


void main()
{
    mat3 frame = mat3(tangent_x, tangent_y, normal);

    vec3 normal_ms = vec3(texture(bump_tex, uv)) - vec3(0.5f, 0.5f, 0.0f);

	vec3 n = normalize(frame * normal_ms);
	vec3 v = normalize(view);

    float distance = length(light);
    vec3 l = light / distance;

    vec3 ambient_color = texture(diffuse_tex, uv).xyz;
    vec3 specular_color = vec3(1.0f);

    float cos_theta = max(dot(n, l), 0.0f);

    vec3 h = normalize(l + v);
    float cos_alpha = max(dot(h, n), 0.0f);

    vec3 color = ambient_color * cos_alpha + specular_color * pow(cos_alpha, 20);
    FragmentColor = vec4(color, 0.5);
}

