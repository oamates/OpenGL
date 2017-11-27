#version 330 core

uniform mat3 camera_matrix;
uniform vec2 focal_scale;

out vec2 uv;
out vec3 view_cs;
out vec3 view_ws;

const vec2 _ndc[4] = vec2[4] 
(
    vec2(-1.0f,  1.0f),
    vec2(-1.0f, -1.0f),
    vec2( 1.0f,  1.0f),
    vec2( 1.0f, -1.0f)
);

void main(void)
{
    vec2 ndc = _ndc[gl_VertexID];
    uv = 0.5 + 0.5 * ndc;
    view_cs = vec3(focal_scale * ndc, -1.0f);
    view_ws = camera_matrix * view_cs;
    gl_Position = vec4(ndc, 0.0f, 1.0f);
}