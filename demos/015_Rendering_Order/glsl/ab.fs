#version 330 core

in vec3 normal;
in vec3 tangent_x;
in vec3 tangent_y;
in vec3 view;
in vec3 light;
in vec2 uv;

uniform sampler2D diffuse_tex;

out vec4 FragmentColor;


void main()
{
    /*
    mat3 frame = mat3(tangent_x, tangent_y, normal);
    vec3 normal_ms = vec3(texture(bump_tex, uv)) - vec3(0.5f, 0.5f, 0.0f);
	vec3 n = normalize(frame * normal_ms);
	vec3 v = normalize(view);
    float distance = length(light);
    vec3 l = light / distance;
    */

    vec4 dtc4 = texture(diffuse_tex, uv);

    dtc4.a *= 0.5;
    FragmentColor = dtc4;
}

