#version 330 core

layout (location = 0) out vec3 position;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec4 albedo;

out vec3 position_ws;
out vec3 normal_ws;
out vec2 uv;

uniform sampler2D diffuse_tex;
uniform sampler2D specular_tex;

void main()
{    
    position = position_ws;
    normal = normalize(normal_ws);
    albedo.rgb = texture(diffuse_tex,  uv).rgb;
    albedo.a   = texture(specular_tex, uv).r;
}