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
out vec3 view_cs;
out vec3 view_ws;

void main()
{
    uv = uvs[gl_VertexID];
    view_cs = vec3(focal_scale * uv, -1.0f);
    view_ws = camera_matrix * view_cs;
    gl_Position = vec4(uv, 0.0f, 1.0f);
}