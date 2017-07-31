#version 330 core

const vec2 uvs[4] = vec2[4] 
(
    vec2(-1.0f,  1.0f),
    vec2(-1.0f, -1.0f),
    vec2( 1.0f,  1.0f),
    vec2( 1.0f, -1.0f)
);

uniform mat3 camera_matrix;
uniform vec2 focal_scale;

out vec2 uv;
out vec3 view;

void main()
{
    uv = uvs[gl_VertexID];
    view = camera_matrix * vec3(focal_scale * uv, -1.0f);
    gl_Position = vec4(uv, 0.0f, 1.0f);
}