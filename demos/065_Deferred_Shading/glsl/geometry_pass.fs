#version 330 core

layout (location = 0) out vec3 position;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec4 albedo;

uniform sampler2D specular_tex;
uniform sampler2D diffuse_tex;

in vec3 position_ws;
in vec3 normal_ws;
in vec2 uv;


void main()
{
    position = position_ws;
    normal = normalize(normal_ws);
    vec3 diffuse = texture(diffuse_tex, uv).rgb;
    float specular = texture(specular_tex, uv).r;
    albedo = vec4(diffuse, specular);
}